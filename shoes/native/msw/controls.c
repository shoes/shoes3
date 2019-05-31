//
// Windows-specific controls code for Shoes.
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
#include "shoes/types/button.h"
#include "shoes/types/check.h"
#include "shoes/types/edit_box.h"
#include "shoes/types/edit_line.h"
#include "shoes/types/list_box.h"
#include "shoes/types/radio.h"
#include "shoes/types/slider.h"

/*
 * Hard to believe but we cast (HMENU)(UINT_PTR)intvar to fit into a void *
 */

static WNDPROC shoes_original_edit_line_proc = NULL;

void
shoes_native_control_hide(SHOES_CONTROL_REF ref)
{
  ShowWindow(ref, SW_HIDE);
}

void
shoes_native_control_show(SHOES_CONTROL_REF ref)
{
  ShowWindow(ref, SW_SHOW);
}

void
shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
  shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
  MoveWindow(ref, p2->ix + p2->dx, p2->iy + p2->dy, p2->iw, p2->ih, TRUE);
}

// TODO: can't find this in shoes-r1157 - types/list_box.c wants it
void shoes_native_control_position_no_pad(SHOES_CONTROL_REF ref , shoes_place *p1,
        VALUE self, shoes_canvas *canvas, shoes_place *p2) {
  PLACE_COORDS();
  MoveWindow(ref, p2->ix + p2->dx, p2->iy + p2->dy, p2->iw, p2->ih, TRUE);
}

void
shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
  shoes_canvas *canvas, shoes_place *p2)
{
  p2->iy -= canvas->slot->scrolly;
  if (CHANGED_COORDS())
    shoes_native_control_position(ref, p1, Qnil, canvas, p2);
  p2->iy += canvas->slot->scrolly;
}

// TODO: can't find this in shoes-r1157 - types/list_box.c wants it
void shoes_native_control_repaint_no_pad(SHOES_CONTROL_REF ref, shoes_place *p1,
        shoes_canvas *canvas, shoes_place *p2) {
  p2->iy -= canvas->slot->scrolly;
  if (CHANGED_COORDS())
    shoes_native_control_position(ref, p1, Qnil, canvas, p2);
  p2->iy += canvas->slot->scrolly;
}

void
shoes_native_control_state(SHOES_CONTROL_REF ref, BOOL sensitive, BOOL setting)
{
  EnableWindow(ref, sensitive);
  SendMessage(ref, EM_SETREADONLY, !setting, 0);
}

void
shoes_native_control_focus(SHOES_CONTROL_REF ref)
{
  SetFocus(ref);
}

void
shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas)
{
  if (GetFocus() == ref) SetFocus(canvas->app->slot->window);
  DestroyWindow(ref);
}

void
shoes_native_control_free(SHOES_CONTROL_REF ref)
{
}

SHOES_SURFACE_REF
shoes_native_surface_new(shoes_canvas *canvas, VALUE self, shoes_place *place)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_SURFACE_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, SHOES_VLCLASS, "Shoes VLC Window",
      WS_CHILD | WS_TABSTOP | WS_VISIBLE,
      place->ix + place->dx, place->iy + place->dy,
      place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE), NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

void
shoes_native_surface_position(SHOES_SURFACE_REF ref, shoes_place *p1, 
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  shoes_native_control_position(ref, p1, self, canvas, p2);
}

void
shoes_native_surface_hide(SHOES_SURFACE_REF ref)
{
  shoes_native_control_hide(ref);
}

void
shoes_native_surface_show(SHOES_SURFACE_REF ref)
{
  shoes_native_control_show(ref);
}

void
shoes_native_surface_remove(shoes_canvas *canvas, SHOES_SURFACE_REF ref)
{
  DestroyWindow(ref);
}

SHOES_CONTROL_REF
shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place,
     VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowExW(WS_EX_NOPARENTNOTIFY, L"BUTTON", NULL,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  shoes_native_edit_line_set_text(ref, msg);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

SHOES_CONTROL_REF
shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("EDIT"), NULL,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL |
      (RTEST(ATTR(attr, secret)) ? ES_PASSWORD : 0),
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  
  shoes_original_edit_line_proc = (WNDPROC)GetWindowLongPtr(ref, GWLP_WNDPROC);
  SetWindowLongPtr(ref, GWLP_WNDPROC, shoes_edit_line_win32proc); 
  
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  shoes_native_edit_line_set_text(ref, msg);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

LRESULT CALLBACK shoes_edit_line_win32proc(HWND win, UINT msg, WPARAM w, LPARAM l)
{
  if (msg == WM_KEYDOWN && w == VK_RETURN) rb_eval_string("Shoes.hook");
  return CallWindowProc(shoes_original_edit_line_proc, win, msg, w, l);
}

VALUE
shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref)
{
  VALUE text;
  LONG i;
  char *utf8 = NULL;
  WCHAR *buffer = NULL;
  i = (LONG)SendMessageW(ref, WM_GETTEXTLENGTH, 0, 0) + 1;
  if (!i) goto empty;
  buffer = SHOE_ALLOC_N(WCHAR, i);
  if (!buffer) goto empty;
  SendMessageW(ref, WM_GETTEXT, i, (LPARAM)buffer);

  utf8 = shoes_utf8(buffer);
  text = rb_str_new2(utf8);
  SHOE_FREE(utf8);
  SHOE_FREE(buffer);
  return text;
empty:
  if (buffer != NULL) SHOE_FREE(buffer);
  return rb_str_new2("");
}

void
shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  WCHAR *buffer = shoes_wchar(msg);
  if (buffer != NULL)
  {
    SendMessageW(ref, WM_SETTEXT, 0, (LPARAM)buffer);
    SHOE_FREE(buffer);
  }
}

SHOES_CONTROL_REF
shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("EDIT"), NULL,
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT |
    ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | ES_NOHIDESEL,
    place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
    canvas->slot->window, (HMENU)(UINT_PTR)cid, 
    (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
    NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  shoes_native_edit_line_set_text(ref, msg);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

VALUE
shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  return shoes_native_edit_line_get_text(ref);
}

void
shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  shoes_native_edit_line_set_text(ref, msg);
}

SHOES_CONTROL_REF
shoes_native_list_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("COMBOBOX"), NULL,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST | WS_VSCROLL,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

void
shoes_native_list_box_update(SHOES_CONTROL_REF box, VALUE ary)
{
  long i;
  SendMessage(box, CB_RESETCONTENT, 0, 0);
  for (i = 0; i < RARRAY_LEN(ary); i++)
  {
    VALUE msg = shoes_native_to_s(rb_ary_entry(ary, i));
    WCHAR *buffer = shoes_wchar(RSTRING_PTR(msg));
    if (buffer != NULL)
    {
      SendMessageW(box, CB_ADDSTRING, 0, (LPARAM)buffer);
      SHOE_FREE(buffer);
    }
  }
}

VALUE
shoes_native_list_box_get_active(SHOES_CONTROL_REF ref, VALUE items)
{
  int sel = SendMessage(ref, CB_GETCURSEL, 0, 0);
  if (sel >= 0)
    return rb_ary_entry(items, sel);
  return Qnil;
}

void
shoes_native_list_box_set_active(SHOES_CONTROL_REF box, VALUE ary, VALUE item)
{
  int idx = rb_ary_index_of(ary, item);
  if (idx < 0)
    return;
  SendMessage(box, CB_SETCURSEL, idx, 0);
}

SHOES_CONTROL_REF
shoes_native_progress(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  HWND ref = CreateWindowEx(WS_EX_TRANSPARENT, PROGRESS_CLASS, msg,
      WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, NULL, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  return ref;
}

double
shoes_native_progress_get_fraction(SHOES_CONTROL_REF ref)
{
  return SendMessage(ref, PBM_GETPOS, 0, 0) * 0.01;
}

void
shoes_native_progress_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
  SendMessage(ref, PBM_SETPOS, (int)(perc * 100), 0L);
}

SHOES_CONTROL_REF
shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  HWND ref = CreateWindowEx(WS_EX_TRANSPARENT, TRACKBAR_CLASS, msg,
      WS_VISIBLE | WS_CHILD,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid,
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

double
shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref)
{
  return SendMessage(ref, TBM_GETPOS, 0, 0) * 0.01;
}

void
shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
  SendMessage(ref, TBM_SETPOS, (int)(perc * 100), 0L);
}


SHOES_CONTROL_REF
shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("BUTTON"), NULL,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

VALUE
shoes_native_check_get(SHOES_CONTROL_REF ref)
{
  return SendMessage(ref, BM_GETCHECK, 0, 0) == BST_CHECKED ? Qtrue : Qfalse;
}

void
shoes_native_check_set(SHOES_CONTROL_REF ref, int on)
{
  SendMessage(ref, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0L);
}

SHOES_CONTROL_REF
shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group)
{
  int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot->controls);
  SHOES_CONTROL_REF ref = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("BUTTON"), NULL,
      WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,
      place->ix + place->dx, place->iy + place->dy, place->iw, place->ih,
      canvas->slot->window, (HMENU)(UINT_PTR)cid, 
      (HINSTANCE)GetWindowLongPtr(canvas->slot->window, GWLP_HINSTANCE),
      NULL);
  SetWindowPos(ref, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW);
  shoes_win32_control_font(cid, canvas->slot->window);
  rb_ary_push(canvas->slot->controls, self);
  return ref;
}

void
shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref)
{
  KillTimer(canvas->slot->window, ref);
}

SHOES_TIMER_REF
shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval)
{
  long nid = rb_ary_index_of(canvas->app->extras, self);
  SetTimer(canvas->slot->window, SHOES_CONTROL1 + nid, interval, NULL);
  return SHOES_CONTROL1 + nid;
}

/* 
 *  TODO: the new control functions since r1157 ;-)
 */
void shoes_native_canvas_oneshot(int ms, VALUE canvas) {
}

void shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg) {
}

void shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref) {
} 

VALUE shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF ref) {
  return Qnil;
}

VALUE shoes_native_control_get_tooltip(SHOES_CONTROL_REF ref) {
  return Qnil;
}

void shoes_native_control_set_tooltip(SHOES_CONTROL_REF ref, VALUE tooltip) {
}

SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, 
    shoes_place *place, VALUE attr, char *msg) {
  return NULL; 
}

void shoes_native_spinner_start(SHOES_CONTROL_REF ref) {
} 

void shoes_native_spinner_stop(SHOES_CONTROL_REF ref) {
}

gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref) {
  return TRUE;
}

SHOES_CONTROL_REF shoes_native_switch(VALUE self, shoes_canvas *canvas,
    shoes_place *place, VALUE attr, char *msg) {
  return NULL;
}

void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, int activate) {
}

VALUE shoes_native_switch_get_active(SHOES_CONTROL_REF ref) {
  return Qnil;
}

void shoes_native_systray(char *title, char *message, char *path) {
}

SHOES_CONTROL_REF shoes_native_text_view(VALUE self, shoes_canvas *canvas, 
    shoes_place *place, VALUE attr, char *msg) {
  return NULL;
}

VALUE shoes_native_text_view_get_text(SHOES_CONTROL_REF ref) {
  return Qnil;
}

void shoes_native_text_view_set_text(SHOES_CONTROL_REF ref, char *msg) {
}

VALUE shoes_native_text_view_append(SHOES_CONTROL_REF ref, char *msg) {
  return Qnil;
}



