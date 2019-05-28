#ifndef  SHOES_NATIVE_WINDOWS_H
#define SHOES_NATIVE_WINDOWS_H

#include "shoes/ruby.h"
#include <commdlg.h>
#include <shlobj.h>

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define HEIGHT_PAD 6

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

extern VALUE cTimer;
extern ID s_hand_cursor, s_link, s_arrow;

// function prototypes for windows specfic helpers
extern WCHAR *shoes_wchar(char *utf8);
extern char *shoes_utf8(WCHAR *buffer);
extern void shoes_win32_center(HWND hwnd);
extern void shoes_win32_control_font(int id, HWND hwnd);
extern void shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg);

#endif
