#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_MENUITEM_TYPE_H
#define SHOES_MENUITEM_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cMenuitem, cMenu;
extern shoes_app _shoes_app;

typedef struct {
    void *native;  
    char *title;   // C string
    char *key;     // accelerator key like 'control_q', C string
    VALUE block;   // block to call
    VALUE context; // app->canvas (ie the app/window default canvas)
} shoes_menuitem;


extern void shoes_menuitem_init();
VALUE shoes_menuitem_alloc(VALUE klass);
VALUE shoes_menuitem_gettitle(VALUE self);
VALUE shoes_menuitem_settitle(VALUE self, VALUE tstr);
VALUE shoes_menuitem_getkey(VALUE self);
VALUE shoes_menuitem_setkey(VALUE self, VALUE kstr);
VALUE shoes_menuitem_setblk(VALUE self, VALUE blk);
VALUE shoes_canvas_menuitem(int argc, VALUE *argv, VALUE self);

// Natives
void *shoes_native_menuitem_new(shoes_menuitem *mi);

#endif
