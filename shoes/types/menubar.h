#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/menu.h"

#ifndef SHOES_MENUBAR_TYPE_H
#define SHOES_MENUBAR_TYPE_H

//#define NEW_MACRO_MENUBAR

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget, cShoesMenubar;
extern shoes_app _shoes_app;

typedef struct {
    void *native;     // cast as needed
    //void *root;       // in gtk, the root window GtkWidget
    //void *accgroup;   // gtk accel_group 
    VALUE context;    // a canvas 
    VALUE menus;      // ruby array of shoes_menu Objects 
} shoes_menubar;

#ifdef NEW_MACRO_MENUBAR
extern const rb_data_type_t shoes_menubar_type;
#endif


void shoes_menubar_init();
VALUE shoes_menubar_alloc(VALUE klass);
VALUE shoes_menubar_list(VALUE self);
VALUE shoes_menubar_append(VALUE self, VALUE menu);
VALUE shoes_menubar_at(VALUE self, VALUE arg);
VALUE shoes_menubar_index(VALUE self, VALUE arg);
VALUE shoes_menubar_insert(VALUE self, VALUE mn, VALUE arg);
VALUE shoes_menubar_remove(VALUE self, VALUE arg);
// canvas
VALUE shoes_canvas_menubar(int argc, VALUE *argv, VALUE self);

// native
VALUE shoes_native_menubar_setup(shoes_app *app, void *widget);
void shoes_native_menubar_append(shoes_menubar *mb, shoes_menu *mn);
void shoes_native_menubar_insert(shoes_menubar *mb, shoes_menu *mn, int pos);
void shoes_native_menubar_remove(shoes_menubar *mb, int pos); 
#endif
