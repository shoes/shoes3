//
// Windows-specific dialog code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"
#include "shoes/appwin32.h"
#include <commdlg.h>
#include <shlobj.h>
#include "shoes/native/windows.h"

const char *dialog_title = USTR("Shoes asks:");
const char *dialog_title_says = USTR("Shoes says:");


VALUE
shoes_native_dialog_color(shoes_app *app)
{
  DWORD winc = GetSysColor(COLOR_3DFACE);
  return shoes_color_new(GetRValue(winc), GetGValue(winc), GetBValue(winc), SHOES_COLOR_OPAQUE);
}

LPWSTR win32_dialog_label = NULL;
LPWSTR win32_dialog_answer = NULL;

BOOL CALLBACK
shoes_ask_win32proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) {
    case WM_INITDIALOG:
      SetDlgItemTextW(hwnd, IDQUIZ, win32_dialog_label);
      if (win32_dialog_label != NULL) {
        SHOE_FREE(win32_dialog_label);
        win32_dialog_label = NULL;
      }
      if (win32_dialog_answer != NULL) {
        GlobalFree((HANDLE)win32_dialog_answer);
        win32_dialog_answer = NULL;
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:
        {
          int len = GetWindowTextLength(GetDlgItem(hwnd, IDQUED));
          if(len > 0) {
            int i;
            win32_dialog_answer = (LPWSTR)GlobalAlloc(GPTR, (len + 1) * sizeof(WCHAR));
            GetDlgItemTextW(hwnd, IDQUED, win32_dialog_answer, len + 1);
          }
        }
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return TRUE;
      }
      break;

    case WM_CLOSE:
      EndDialog(hwnd, 0);
      return FALSE;
  }
  return FALSE;
}

// TODO: was shoes_dialog_alert(VALUE self, VALUE msg)
VALUE
shoes_dialog_alert(int argc, VALUE *argv, VALUE self) {
  WCHAR *buffer;
  GLOBAL_APP(app);
  //msg = shoes_native_to_s(msg);
  VALUE msg = shoes_native_to_s(argv[0]);
  buffer = shoes_wchar(RSTRING_PTR(msg));
  MessageBoxW(APP_WINDOW(app), buffer, (LPCWSTR)dialog_title_says, MB_OK);
  if (buffer != NULL)
    SHOE_FREE(buffer);
  return Qnil;
}

VALUE
shoes_dialog_ask(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE answer = Qnil;
  rb_parse_args(argc, argv, "s|h", &args);
  GLOBAL_APP(app);
  win32_dialog_label = shoes_wchar(RSTRING_PTR(args.a[0]));
  int confirm = DialogBox(shoes_world->os.instance,
	MAKEINTRESOURCE(RTEST(ATTR(args.a[1],secret)) ? ASKSECRETDLG : ASKDLG),
	APP_WINDOW(app),
	shoes_ask_win32proc);
  if (confirm == IDOK) {
    if (win32_dialog_answer != NULL) {
      char *ans8 = shoes_utf8(win32_dialog_answer);
      answer = rb_str_new2(ans8);
      SHOE_FREE(ans8);
      GlobalFree((HANDLE)win32_dialog_answer);
      win32_dialog_answer = NULL;
    }
  }
  return answer;
}

// TODO: was shoes_dialog_confirm(VALUE self, VALUE quiz)
VALUE
shoes_dialog_confirm(int argc, VALUE *argv, VALUE self)
{
  WCHAR *buffer;
  VALUE answer = Qfalse;
  GLOBAL_APP(app);
  VALUE quiz = shoes_native_to_s(argv[0]);
  buffer = shoes_wchar(RSTRING_PTR(quiz));
  int confirm = MessageBoxW(APP_WINDOW(app), buffer, (LPCWSTR)dialog_title, MB_OKCANCEL);
  if (confirm == IDOK)
    answer = Qtrue;
  if (buffer != NULL)
    SHOE_FREE(buffer);
  return answer;
}

VALUE
shoes_dialog_color(VALUE self, VALUE title)
{
  VALUE color = Qnil;
  GLOBAL_APP(app);
  CHOOSECOLOR cc;
  static COLORREF acrCustClr[16];
  static DWORD rgbCurrent;

  // Initialize CHOOSECOLOR 
  ZeroMemory(&cc, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = APP_WINDOW(app);
  cc.lpCustColors = (LPDWORD) acrCustClr;
  cc.rgbResult = rgbCurrent;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT;
   
  if (ChooseColor(&cc)) {
    color = shoes_color_new(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult),
      SHOES_COLOR_OPAQUE);
  }
  return color;
}

static char *
shoes_fix_slashes(char *path)
{
  char *p;
  for (p = path; *p != '\0'; p++)
    if (*p == '\\')
      *p = '/';
  return path;
}

static VALUE
shoes_dialog_chooser(VALUE self, char *title, DWORD flags)
{
  BOOL ok;
  VALUE path = Qnil;
  GLOBAL_APP(app);
  char dir[MAX_PATH+1], _path[MAX_PATH+1];
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  GetCurrentDirectory(MAX_PATH, (LPSTR)dir);
  ofn.lStructSize     = sizeof(ofn);
  ofn.hwndOwner       = APP_WINDOW(app);
  ofn.hInstance       = shoes_world->os.instance;
  ofn.lpstrFile       = _path;
  ofn.lpstrTitle      = title;
  ofn.nMaxFile        = sizeof(_path);
  ofn.lpstrFile[0] = '\0';
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL; // (LPSTR)dir;
  ofn.Flags           = OFN_EXPLORER | flags;
  if (flags & OFN_OVERWRITEPROMPT)
    ok = GetSaveFileName(&ofn);
  else
    ok = GetOpenFileName(&ofn);
  if (ok)
    path = rb_str_new2(shoes_fix_slashes(ofn.lpstrFile));
  SetCurrentDirectory((LPSTR)dir);
  return path;
}

static VALUE
shoes_dialog_chooser2(VALUE self, char *title, UINT flags)
{
  VALUE path = Qnil;
  BROWSEINFO bi = {0};
  bi.lpszTitle = title;
  bi.ulFlags = BIF_USENEWUI | flags;
  LPITEMIDLIST pidl = SHBrowseForFolder (&bi);
  if (pidl != 0) {
    char *p;
    char _path[MAX_PATH+1];
    if (SHGetPathFromIDList(pidl, _path))
      path = rb_str_new2(shoes_fix_slashes(_path));

    IMalloc *imalloc = 0;
    if (SUCCEEDED(SHGetMalloc(&imalloc))) {
      IMalloc_Free(imalloc, pidl);
      IMalloc_Release(imalloc);
    }
  }
  return path;
}

VALUE
shoes_dialog_open(int argc, VALUE *argv, VALUE self) {
  return shoes_dialog_chooser(self, "Open file...", OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST);
}

VALUE
shoes_dialog_save(int argc, VALUE *argv, VALUE self)
{
  return shoes_dialog_chooser(self, "Save file...", OFN_OVERWRITEPROMPT);
}

VALUE
shoes_dialog_open_folder(int argc, VALUE *argv, VALUE self)
{
  return shoes_dialog_chooser2(self, "Open folder...", BIF_NONEWFOLDERBUTTON | BIF_RETURNONLYFSDIRS);
}

VALUE
shoes_dialog_save_folder(int argc, VALUE *argv, VALUE self)
{
  return shoes_dialog_chooser2(self, "Save folder...", BIF_RETURNONLYFSDIRS);
}
