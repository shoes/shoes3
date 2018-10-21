#ifndef SHOES_LAYOUT_TYPE_H
#define SHOES_LAYOUT_TYPE_H

// C level layouts. Not all are implemented. 
typedef enum {
  Layout_None,
  Layout_Constraints,
  Layout_VFL,
  Layout_Grid,
} Layout_Types;

typedef struct {
  VALUE delegate;   // user Written Ruby layout class
  VALUE canvas;  
  int cache_w;      // used to synthesize 'size' event
  int cache_h;
  // fields below belong to the C crafted layout manager(s), what ever that
  // turns out to be. 
  Layout_Types mgr;
  VALUE fields;     // whatever the manager needs it to be.
} shoes_layout;

// all drawables do/should implement this at the top - slightly safer
// than deconstructing shoes_canvas for everything. Treat as read-only
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
} shoes_abstract;

extern VALUE cLayout;
void shoes_layout_init();
VALUE shoes_layout_new(VALUE attr, VALUE parent);
// slot like methods:
VALUE shoes_layout_insert(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_delete_at(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_clear(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_add_rules(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_finish(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_get_height(VALUE self);
VALUE shoes_layout_get_width(VALUE self);
VALUE shoes_layout_start(int argc, VALUE *argv, VALUE self);
VALUE shoes_layout_style(int argc, VALUE *argv, VALUE self);

// canvas calls these, delegate to usr or the secret layout
void shoes_layout_size(shoes_canvas *canvas, int pass);
void shoes_layout_cleared(shoes_canvas *canvas);
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele);
VALUE shoes_layout_delete_ele(shoes_canvas *canvas, VALUE ele);

//  TODO: delegate methods for the future internal manager(s). 
void shoes_layout_internal_setup(shoes_layout *lay, shoes_canvas *canvas,
  VALUE attr);
void shoes_layout_internal_add(shoes_canvas *canvas, VALUE ele);
VALUE shoes_layout_internal_delete_at(shoes_layout *lay, shoes_canvas *canvas,
      VALUE ele, int pos);
void shoes_layout_internal_clear(shoes_canvas *canvas);
void shoes_layout_internal_size(shoes_layout *lay, shoes_canvas *canvas, int pass);
void shoes_layout_internal_finish(shoes_canvas *canvas);
void shoes_layout_internal_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE arg);
#endif
