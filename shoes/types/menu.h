#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/menuitem.h"
#ifndef SHOES_MENU_TYPE_H
#define SHOES_MENU_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cMenu;
extern shoes_app _shoes_app;

// Typically, a menubar object has references to this object
typedef struct {
    void *native;
    void *extra;  // Gtk needs two GtkWidgets;
    char *title;  // C string utf8
    VALUE items;  // ruby array of menuitem Objects
} shoes_menu;

extern void shoes_menu_init();
VALUE shoes_menu_alloc(VALUE klass);
VALUE shoes_menu_get(VALUE self, VALUE idx);
VALUE shoes_menu_set(VALUE self, VALUE idx, VALUE menuitem);
VALUE shoes_menu_append(VALUE self, VALUE menuitem);
VALUE shoes_canvas_menu(int, VALUE *, VALUE);

// natives
void *shoes_native_menu_new(shoes_menu *mn);
void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi);

#endif
