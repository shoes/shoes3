#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
//#include "shoes/types/menu.h"

#ifndef SHOES_MENUITEM_TYPE_H
#define SHOES_MENUITEM_TYPE_H

//#define NEW_MACRO_MENUITEM

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cShoesMenuitem, cShoesMenu;
extern shoes_app _shoes_app;

typedef struct {
    void *native;
    void *parent; // assigned when placed in a menu
    char *title; 
    int  state;    // enable/disable T/F, plus key modifier flags. 
    char *key;     // accelerator key . flags in state var above
    VALUE block;   // ruby block to call
    VALUE context; // app->canvas 
    void *id;      // gtk accel path, optionally a cocoa tag
} shoes_menuitem;


#define MENUITEM_ENABLE 1
#define MENUITEM_SHIFT 2
#define MENUITEM_CONTROL 4
#define MENUITEM_ALT 8
#define NO_NATIVE 16
 
#ifdef NEW_MACRO_MENUITEM
extern const rb_data_type_t shoes_menuitem_type;
#endif

extern void shoes_menuitem_init();
VALUE shoes_menuitem_alloc(VALUE klass);
VALUE shoes_menuitem_new(VALUE text, int flags, char *key, VALUE blk, VALUE canvas);
VALUE shoes_menuitem_gettitle(VALUE self);
VALUE shoes_menuitem_settitle(VALUE self, VALUE tstr);
VALUE shoes_menuitem_getkey(VALUE self);
VALUE shoes_menuitem_setkey(VALUE self, VALUE kstr);
VALUE shoes_menuitem_setblk(VALUE self, VALUE blk);
VALUE shoes_menuitem_setenable(VALUE self, VALUE state);
// helpers
int shoes_menuitem_parse_key(VALUE keystr, char *outkey);
// canvas
VALUE shoes_canvas_menuitem(int argc, VALUE *argv, VALUE self);
// Natives
void *shoes_native_menuitem_new(shoes_menuitem *mi);
void *shoes_native_menusep_new(shoes_menuitem *mi);
void shoes_native_menuitem_enable(shoes_menuitem *mi, int ns);
void shoes_native_menuitem_set_title(shoes_menuitem *mi);
void shoes_native_menuitem_set_key(shoes_menuitem *mi, int newflags, char *newkey);
int shoes_native_menuitem_get_key(shoes_menuitem *mi);
#endif
