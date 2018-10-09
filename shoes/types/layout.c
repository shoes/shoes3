/*
 * layout.c 
*/
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/version.h"
#include "shoes/types/types.h"
#include "shoes/types/settings.h"
#include "shoes/types/layout.h"
#include <math.h>
//
// Shoes::Layout needs to be a slot-like class - same api?
//
extern VALUE cButton, cBackground;

/*  FUNC_M generate two functions here
 *  shoes_canvas_c_layout(int argc, VALUE *argv, VALUE self) { ..}
 *      + means call shoes_canvas_repaint_all() at end
 *  shoes_app_c_layout(int argc, VALUE *argv, VALUE self) {...}
 */ 
FUNC_M("+layout", layout, -1);

void shoes_layout_init() {
  cLayout = rb_define_class_under(cTypes, "Layout", cShoes);
  //cLayout = rb_define_class_under(cFlow, "Layout", cShoes);
  rb_define_method(cLayout, "append", CASTHOOK(shoes_layout_append), -1);  // crash in shoes_canvas_memdraw
  //rb_define_method(cLayout, "append", CASTHOOK(shoes_canvas_append), -1); // slot is being modified
  rb_define_method(cLayout, "clear", CASTHOOK(shoes_canvas_clear_contents), -1);
  rb_define_method(cLayout, "prepend", CASTHOOK(shoes_canvas_prepend), -1);
  rb_define_method(cLayout, "before", CASTHOOK(shoes_canvas_before), -1);
  rb_define_method(cLayout, "after", CASTHOOK(shoes_canvas_after), -1);
  rb_define_method(cLayout, "rule", CASTHOOK(shoes_layout_add_rule), -1);  
  rb_define_method(cLayout, "finish", CASTHOOK(shoes_layout_compute), -1);
  
  /*  RUBY_M generates defines (allow Ruby to call the FUNC_M funtions
  rb_define_method(cCanvas, "layout", CASTHOOK(shoes_canvas_c_layout), -1); 
  rb_define_method(cApp,    "layout", CASTHOOK(shoes_app_c_layout), -1)
  */
  RUBY_M("+layout", layout, -1);
}

void shoes_layout_mark(shoes_layout *ly) {
  rb_gc_mark_maybe(ly->canvas);
  rb_gc_mark_maybe(ly->delegate);
}

static void shoes_layout_free(shoes_layout *ly) {
  RUBY_CRITICAL(SHOE_FREE(ly));
}

VALUE shoes_layout_alloc(VALUE klass) {
  VALUE obj;
  shoes_layout *ly = SHOE_ALLOC(shoes_layout);
  SHOE_MEMZERO(ly, shoes_layout, 1);
  obj = Data_Wrap_Struct(klass, shoes_layout_mark, shoes_layout_free, ly);
  // set fields ? 
  ly->x = 10;
  ly->y = 10;
  ly->delegate = Qnil;
  ly->canvas = Qnil;
  return obj;
}

VALUE shoes_layout_new(VALUE attr, VALUE parent) {
    fprintf(stderr, "shoes_layout_new called\n");
    VALUE obj = shoes_layout_alloc(cLayout);
    shoes_layout *ly;
    Data_Get_Struct(obj, shoes_layout, ly);
    // Most of shoes thinks its a Flow (cFlow or cLayout ?)
    VALUE canvas = shoes_slot_new(cFlow, attr, parent);
    ly->canvas = canvas;
    shoes_canvas *cvs;
    Data_Get_Struct(canvas, shoes_canvas, cvs);
    // get manager from attr, put in delegate.
    VALUE mgr;
    ID s_manager = rb_intern ("manager");
    mgr = ATTR(attr, manager);
    if (! NIL_P(mgr)) {
      ly->delegate = mgr;
      ID s_setup = rb_intern("setup");
      if (rb_respond_to(mgr, s_setup))
        rb_funcall(mgr, s_setup, 2, canvas, attr);
    }
    cvs->layout_mgr = obj; // me
    //return cvs;
    return obj;
}

VALUE shoes_layout_append(int argc, VALUE *argv, VALUE self) {
    shoes_layout *lay;
    Data_Get_Struct(self, shoes_layout, lay);
    VALUE canvas = lay->canvas;
    rb_arg_list args;
    rb_parse_args(argc, argv, "o,&", &args);
    shoes_canvas_insert(canvas, -1, Qnil, args.a[0]);
    return self;
}

VALUE shoes_layout_prepend(VALUE self, VALUE ele) {
  shoes_layout *ly;
  Data_Get_Struct(self, shoes_layout, ly);
  shoes_canvas *canvas;
  Data_Get_Struct(ly->canvas, shoes_canvas, canvas);
  shoes_layout_add_ele(canvas, ele);
}

// called from shoes_add_ele (def in canvas.c) by widget creators
// The ele has already been added to canvas->contents
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele) {
  if (rb_obj_is_kind_of(ele, cBackground)) {
    //fprintf(stderr, "skipping background widget\n");
    return;
  }
  // Find a delegate or use the internal default?
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    if (! NIL_P(ly->delegate)) {
      //printf(stderr,"Delegating\n");
      VALUE del = ly->delegate;
      ID s_addw = rb_intern("add");
      if (rb_respond_to(del, s_addw))
        rb_funcall(del, s_addw, 2, ly->canvas, ele);
      else {
        rb_raise( rb_eArgError, "'add' not implment in Layout");
      }
      return;
    }
  } 
  // here if no delgate or no manager object
  shoes_layout_default_add(canvas, ele);
  return; 
}

// called from inside shoes (shoes_canvas_clear)
void shoes_layout_clear(shoes_canvas *canvas) {
  fprintf(stderr,"shoes_layout_clear called\n");
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    if (! NIL_P(ly->delegate)) {
      //printf(stderr,"Delegating\n");
      VALUE del = ly->delegate;
      ID s_clear = rb_intern("clear");
      if (rb_respond_to(del, s_clear))
        rb_funcall(del, s_clear, 0);
      else {
        rb_raise( rb_eArgError, "'clear' not implment in Layout");
      }
      return;
    }
  } 
  // here if no delgate or no manager object
  shoes_layout_default_clear(canvas);
  return; 
}

/* 
 * Methods to call the default layout manager - 
 * Might be a cassawory variant? grid_bag? Something Gtk or Cocoa? 
*/
void shoes_layout_default_add(shoes_canvas *canvas, VALUE ele) {
  fprintf(stderr, "default layout add\n");
}

void shoes_layout_default_clear(shoes_canvas *canvas) {
  fprintf(stderr, "default layout clear\n");
}

VALUE shoes_layout_add_rule(VALUE self, VALUE rule) {
  fprintf(stderr,"shoes_layout_add_rule called\n");
}

VALUE shoes_layout_compute(VALUE self) {
  fprintf(stderr, "shoes_layout_compute called\n");
}
