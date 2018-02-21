#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/menu.h"

#ifndef SHOES_MENUBAR_TYPE_H
#define SHOES_MENUBAR_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget, cShoesMenubar;
extern shoes_app _shoes_app;

typedef struct {
    void *native;  // cast as needed
    VALUE menus;      // ruby array of shoes_menu Objects 
} shoes_menubar;

void shoes_menubar_init();
VALUE shoes_menubar_alloc(VALUE klass);
VALUE shoes_menubar_list(VALUE self);
VALUE shoes_menubar_append(VALUE self, VALUE menu);
VALUE shoes_menubar_at(VALUE self, VALUE arg);
// canvas
VALUE shoes_canvas_menubar(int argc, VALUE *argv, VALUE self);

// native
VALUE shoes_native_menubar_setup(shoes_app *app);
void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn);
#endif
