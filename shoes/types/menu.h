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
extern VALUE cShoes, cApp, cTypes, cCanvas, cShoesMenu;
extern shoes_app _shoes_app;

// The shoes_menubar object has references to this object
typedef struct {
    void *native; //Gtk uses two GtkWidgets - menuitem
    void *extra;  //  and menu
    void *context; // a canvas - gtk needs this
    void *parent; // not used - for submenus? 
    char *title;  // C string utf8
    VALUE items;  // ruby array of menuitem Objects
} shoes_menu;

extern void shoes_menu_init();
VALUE shoes_menu_alloc(VALUE klass);
VALUE shoes_menu_new(VALUE text);
VALUE shoes_menu_list(VALUE self);
VALUE shoes_menu_title(VALUE self);
VALUE shoes_menu_append(VALUE self, VALUE menuitem);
VALUE shoes_menu_insert(VALUE self, VALUE menuitem, VALUE pos);
VALUE shoes_menu_at(VALUE self, VALUE arg);
VALUE shoes_menu_index(VALUE self, VALUE arg);
VALUE shoes_menu_remove(VALUE self, VALUE arg);
// canvas (creation of menu)
VALUE shoes_canvas_menu(int, VALUE *, VALUE);

// natives
void *shoes_native_menu_new(shoes_menu *mn);
void *shoes_native_menu_append(shoes_menu *mn, shoes_menuitem *mi);
void shoes_native_menu_insert(shoes_menu *mn, shoes_menuitem *mi, int pos);
void shoes_native_menu_remove(shoes_menu *mn, int pos);
#endif
