#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SYSTRAY_TYPE_H
#define SHOES_SYSTRAY_TYPE_H

//#define NEW_MACRO_SYSTRAY

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// not a graphical widget
typedef struct _systray {
    char *icon_path;
    char *title;
    char *message;
} shoes_systray;

#ifdef NEW_MACRO_SYSTRAY
extern const rb_data_type_t shoes_systray_type;
#endif

// ruby (systray)
VALUE shoes_systray_new(int argc, VALUE *argv, VALUE self);
VALUE shoes_systray_alloc(VALUE);
void shoes_systray_init();

// canvas
VALUE shoes_canvas_systray(int, VALUE *, VALUE);

#endif
