#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/types/menubar.h"
#include "shoes/types/menu.h"
#include "shoes/types/menuitem.h"
#include "shoes/native/cocoa/menus.h"


// -------- menubar -----
void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn) {

}

/*
 * This is clumsy. Create a Menubar, File Menu, and File->Quit in
 * both gtk and Shoes. This happens when menus are requested for a app/window
 * The default set of menus is created here.
*/ 
void shoes_native_menubar_quit(void *extra) {
  //fprintf(stderr, "Dummy quit called\n");
  shoes_native_quit();
}

// TODO: calling this will segfault
VALUE shoes_native_menubar_make_block(char *code) {
  int argc = 1;
  VALUE argv[2];
  argv[0] = rb_str_new2(code);
  argv[1] = Qnil;
  VALUE block = Qnil;
  rb_scan_args(argc, argv, "0&", &block);
  return block;
}

VALUE shoes_native_menubar_setup(shoes_app *app) {
  fprintf(stderr,"menubar setup called\n");
  return Qnil;
}



// ------- menu ------
void *shoes_native_menu_new(shoes_menu *mn) {
  return NULL;
}

void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi) {
  return NULL;
}

// -------- menuitem ------


void *shoes_native_menuitem_new(shoes_menuitem *mi) {
  return NULL;
}



