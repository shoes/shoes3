//
// Windows-specific menu code for Shoes.
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
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/types/settings.h"
#include "shoes/native/windows.h"
#include <commdlg.h>
#include <shlobj.h>

// TODO: a lot

WCHAR *shoes_win32_mi_create_str(shoes_menuitem *mi);

// This is called from a shoes script via types/menubar.c
// Returns a  Shoes menubar object but the natives are created
// later (or sooner?) in shoes_win32_menubar_setup
// WARNING! it's also called 
VALUE shoes_native_menubar_setup(shoes_app *app, void *nothing) {
  HMENU hMenubar = (HMENU)nothing;
  if (nothing == NULL) {
    fprintf(stderr,"shoes_native_menubar_setup: creating mb\n");
    hMenubar = CreateMenu();
    app->os.menubar = hMenubar;
    app->have_menu = TRUE;
  } 
  if (NIL_P(app->menubar)) {
    // use the platform neutral calls to build Shoes menu
    // we can't do that for the menubar so we'll build it ourself
    VALUE mbv = shoes_menubar_alloc(cShoesMenubar);
    Get_TypedStruct2(mbv, shoes_menubar, mb);
    mb->context = app->canvas;
    mb->native = (void *)hMenubar;
    // save menubar object in app object
    app->menubar = mbv;
  }
  return app->menubar;
}
// A bit tricky in Windows. We create the native menus from inside the 
// message loop for 'hwnd'? Why? MAYBE NOT - app isn't set.
void shoes_win32_menubar_setup(shoes_app *app, HWND hwnd) {
  if (! app) {
    fprintf(stderr, "shoes_win32_menubar_setup: app is nil\n");
    return;
  }
  fprintf(stderr, "shoes_win32_menubar_setup: have an app!");
  shoes_menubar *mb;
  TypedData_Get_Struct(app->menubar, shoes_menubar, &shoes_menubar_type, mb);
  SetMenu(hwnd, (HMENU)mb->native);
}

void shoes_native_build_menus(shoes_app *app, VALUE mbv) {
      // Shoes menu
      //VALUE shoestext = rb_str_new2("Shoes");
      Get_TypedStruct2(shoes_world->settings, shoes_settings, st);
      VALUE shoestext = st->app_name;
      VALUE shoesmenu = shoes_menu_new(shoestext);
      int flags = MENUITEM_ENABLE;
      char *key = "";
      // New/open Shoes.show_selector
      VALUE otext = rb_str_new2("Open");
      VALUE oproc = rb_eval_string("proc { Shoes.show_selector }");
      VALUE oitem = shoes_menuitem_new(otext,flags|MENUITEM_CONTROL, "o", oproc, app->canvas);
      shoes_menu_append(shoesmenu, oitem);
      // -------- separator
      VALUE stext = rb_str_new2("--- a seperator");
      VALUE s1item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s1item);
      // Manual 
      VALUE mtext = rb_str_new2("Manual");
      VALUE mproc = rb_eval_string("proc { Shoes.show_manual }");
      VALUE mitem = shoes_menuitem_new(mtext, flags, key, mproc, app->canvas);
      shoes_menu_append(shoesmenu, mitem);
      // Cobbler
      VALUE ctext = rb_str_new2("Cobbler");
      VALUE cproc = rb_eval_string("proc { Shoes.cobbler }");
      VALUE citem = shoes_menuitem_new(ctext, flags, key, cproc, app->canvas);
      shoes_menu_append(shoesmenu, citem);

      // Profile - bug in profiler (#400 , @dredknight)
      VALUE ftext = rb_str_new2("Profile");
      VALUE fproc = rb_eval_string("proc { require 'shoes/profiler'; Shoes.profile(nil) }");
      VALUE fitem = shoes_menuitem_new(ftext, flags, key, fproc, app->canvas);
      shoes_menu_append(shoesmenu, fitem);
      // Package
      VALUE ptext = rb_str_new2("Package");
      VALUE pproc = rb_eval_string("proc { Shoes.app_package }");
      VALUE pitem = shoes_menuitem_new(ptext, flags, key, pproc, app->canvas);
      shoes_menu_append(shoesmenu, pitem);
      // --------
      VALUE s2item = shoes_menuitem_new(stext, flags, key, Qnil, app->canvas);
      shoes_menu_append(shoesmenu, s2item);

      // Quit
      VALUE qtext = rb_str_new2("Quit");
      VALUE qproc = rb_eval_string("proc { Shoes.quit }");
      VALUE qitem = shoes_menuitem_new(qtext, flags | MENUITEM_CONTROL, "q", qproc, app->canvas); 

      shoes_menu_append(shoesmenu, qitem);
      shoes_menubar_append(mbv, shoesmenu);
}

// ---- mapping from int(id) to *menuitem ---------

static int shoes_win32_menuctr = 1;
static void *shoes_win32_menutbl[256] = {0};

int shoes_win32_menuid(shoes_menuitem *mi) {
  int newi = shoes_win32_menuctr++;
  shoes_win32_menutbl[newi] = (void *)mi;
  return newi;
}
// Called from the message loop - we may have to search the menu tree
// to find the matching menuitem_number
void shoes_win32_menu_lookup(shoes_app *app, int menuitem_number) {
  shoes_menuitem *mi;
  if (menuitem_number >= shoes_win32_menuctr) {
    fprintf(stderr, "Out of range menu id: %d\n", menuitem_number);
    return;
  }
  mi = shoes_win32_menutbl[menuitem_number];
  if (mi) {
    fprintf(stderr, "Dispatch %d %s\n", mi->extra, mi->title);
    shoes_safe_block(mi->context, mi->block, rb_ary_new3(1, mi->context));
  }
}

// --------- remember, we just want the natives to match what's already
//           been done to the Shoes objects.

void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
  HMENU menubar = (HMENU)mb->native;
  HMENU menu = (HMENU)mn->native;
  //fprintf(stderr, "mbar: append %s to mbar\n", mn->title);
  // add the app accel group to the menu
  shoes_canvas *canvas;
  TypedData_Get_Struct(mb->context, shoes_canvas, &shoes_canvas_type, canvas);
  shoes_app *app = canvas->app;
  //gtk_menu_set_accel_group((GtkMenu *)mn->extra, app->os.accel_group);
  //gtk_menu_shell_append(GTK_MENU_SHELL(menubar), menu);
  //shoes_native_menubar_update(mb->context);
  WCHAR *wstr = shoes_wchar(mn->title);
  AppendMenuW(menubar, MF_POPUP, (UINT_PTR) menu, wstr);
  BOOL rtn = DrawMenuBar(app->slot->window);
  char errstr[20];
  if (rtn == 0) 
    sprintf(errstr, "%d", GetLastError());
  fprintf(stderr,"mb append %s : %s\n", mn->title, rtn? "OK" : errstr);
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
}

void *shoes_native_menu_new(shoes_menu *mn) {
  HMENU menu = CreateMenu();
  mn->native = (void *)menu; 
  mn->extra = (void *)menu;
  //char path[100];
  //sprintf(path, "<AppID%d>/%s", shoes_app_serial_num, mn->title);
  //gtk_menu_set_accel_path((GtkMenu *)menu, path);
  fprintf(stderr, "menu new: %s\n", mn->title);
  return mn->native;
}

void shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  HMENU menu = (HMENU)mn->native;
  HMENU menuitem = (HMENU)mi->native;
  AppendMenuW(menu, MF_STRING, mi->extra, shoes_win32_mi_create_str(mi));
  fprintf(stderr, "menu_append: %s << %s\n", mn->title, mi->title);
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
}

WCHAR* shoes_win32_mi_create_str(shoes_menuitem *mi) {
  // TODO use glib functions. 
  // "\t[Shift+][ALT+][CTRL+]<char>"
  char tmp[200];
  strcpy(tmp, mi->title);
  int pos = strlen(tmp);   // bless the wonders of newer C compilers!
  if (mi->state & MENUITEM_CONTROL) {
    strcpy(tmp+pos,"\tCtrl+");
    pos = strlen(tmp);
    char up[2] = {mi->key[0], 0};
    if (up[0] > 'Z')
      up[0] = up[0] - 32;
    strcpy(tmp+pos, up);
  }
  return shoes_wchar(tmp);
}

void shoes_win32_setaccel(ACCEL *nacc, VALUE canvas) {
  // Find the app that has the canvas (shoes_menuitem.context)
  VALUE apps = shoes_world->apps;
  shoes_app *app = NULL;
  VALUE appv = Qnil;
  HACCEL hacc; 
  for (int i = 0; i < RARRAY_LEN(apps); i++) {
    appv = rb_ary_entry(apps, i);
    TypedData_Get_Struct(appv, shoes_app, &shoes_app_type, app);
    if (app->canvas == canvas)
      break;
  }
  if (app == NULL) {
    fprintf(stderr, "No app for accelerator?\n");
    return;
  }
  //fprintf(stderr, "Found an app for accel table\n");
  int cnt = app->os.acc_cnt;
  ACCEL tbl[(cnt + 1) * sizeof(ACCEL)]; 
  if (cnt == 0) {
    tbl[0] = *nacc;   // Should be a struct copy
  } else {
    CopyAcceleratorTableW(app->os.accel, tbl, cnt);
    tbl[cnt+1] = *nacc;    
    DestroyAcceleratorTable(app->os.accel);
  }
  app->os.acc_cnt++;
  app->os.accel = CreateAcceleratorTableW(tbl, app->os.acc_cnt);
}

// Returns a Windows string (utf-16) formatted with accelators
void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  HMENU gmi;
  gmi = CreateMenu();
  mi->native = (void *)gmi;
  mi->extra = shoes_win32_menuid(mi); // Get a new id
  if (mi->state & (MENUITEM_SHIFT | MENUITEM_CONTROL | MENUITEM_ALT)) {
    // need to (re)create an ACCEL table to include this item
    ACCEL nacc;
    nacc.fVirt = 0;
    if (mi->state & MENUITEM_CONTROL)
      nacc.fVirt |= FCONTROL;
    if (mi->state & MENUITEM_ALT)
      nacc.fVirt |= FALT;
    if (mi->state & MENUITEM_SHIFT)
      nacc.fVirt |= FSHIFT;
    nacc.key = (WORD)mi->key[0];
    shoes_win32_setaccel(&nacc, mi->context);
  }
  fprintf(stderr, "mi new: %d => %s\n", mi->extra, mi->title);
  return (void *)mi->native;
}

void *shoes_native_menusep_new(shoes_menuitem *mi) {
}

void shoes_native_menuitem_enable(shoes_menuitem *mi, int ns) {
}

void shoes_native_menuitem_set_title(shoes_menuitem *mi) {
}

void shoes_native_menuitem_set_key(shoes_menuitem *mi, int newflags, char *newkey) {
}

int shoes_native_menuitem_get_key(shoes_menuitem *mi) {
  return 0;
}
