#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_EVENT_TYPE_H
#define SHOES_EVENT_TYPE_H

#define NEW_MACRO_EVENT

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern VALUE cShoesEvent;

extern shoes_app _shoes_app;

#define SHOES_MODIFY_SHIFT 0x01
#define SHOES_MODIFY_CTRL  0x02
#define SHOES_MODIFY_ALT   0x04
#define SHOES_MODIFY_META  0x08  // caution: fan key on osx, something else on linux


typedef struct {
    VALUE type; 
    VALUE object;
    VALUE accept;
    int btn;
    int x;  
    int y;
    int width;
    int height;
    VALUE key;     // UTF-8 string
    VALUE modifiers; //  modifiers if we can get them.
} shoes_event;

#ifdef NEW_MACRO_EVENT
extern const rb_data_type_t shoes_event_type;
#endif

// ruby getters and setters
VALUE shoes_event_kind(VALUE self);
VALUE shoes_event_object(VALUE self);
VALUE shoes_event_get_accept(VALUE self);
VALUE shoes_event_set_accept(VALUE self, VALUE tf);
VALUE shoes_event_button(VALUE self);
VALUE shoes_event_x(VALUE self);
VALUE shoes_event_y(VALUE self);
VALUE shoes_event_width(VALUE self);
VALUE shoes_event_height(VALUE self);
VALUE shoes_event_key(VALUE self);
VALUE shoes_event_set_key(VALUE self, VALUE key);
VALUE shoes_event_modifiers(VALUE self);
// init create
void shoes_event_mark(shoes_event *event);
void shoes_event_free(shoes_event *event);
VALUE shoes_event_new(VALUE klass, ID type, VALUE widget, int x, int y,
    int btn, VALUE modifiers, VALUE key);
VALUE shoes_event_alloc(VALUE klass);
VALUE shoes_event_create_event(shoes_app *app, ID action, int button, int x, int y, VALUE modifiers, VALUE key);
VALUE shoes_canvas_shoesevent(int argc, VALUE *argv, VALUE self);
VALUE shoes_event_new_widget(VALUE klass, ID type, VALUE widget, int btn, int x, \
        int y, int w, int h, VALUE modifiers, VALUE key);
VALUE shoes_event_find_psuedo (VALUE self, int x, int y, VALUE *pswidget);
VALUE shoes_event_new_key(VALUE klass, ID type, VALUE key);
VALUE shoes_event_find_native (VALUE self, int x, int y, VALUE *hitobj);
void shoes_shoesevent_init();
VALUE shoes_textblock_event_is_here(VALUE self, int x, int y);
VALUE shoes_image_event_is_here(VALUE self, int x, int y);
VALUE shoes_svg_event_is_here(VALUE self, int x, int y);
VALUE shoes_plot_event_is_here(VALUE self, int x, int y);
VALUE shoes_shape_event_is_here(VALUE self, int x, int y);
VALUE shoes_control_is_here(VALUE self, int x, int y);

// helper
VALUE shoes_event_contrain_TF(VALUE evt);
#endif
