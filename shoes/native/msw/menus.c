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

// TODO: a lot to todo

VALUE shoes_native_menubar_setup(shoes_app *app, void *widget) {
  return Qnil;
}

void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {
}

void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos) {
}

void shoes_native_menubar_remove(shoes_menubar *mb, int pos) {
}

void *shoes_native_menu_new(shoes_menu *mn) {
}

void shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
}

void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos) {
}

void shoes_native_menu_remove(shoes_menu *mn, int pos) {
}

void *shoes_native_menuitem_new(shoes_menuitem *mi) {
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
