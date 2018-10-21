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
//
// Shoes::Layout needs to be a slot-like class - mostly same api
//
extern VALUE cButton, cBackground;

// user written managers must implment these methods:
static ID s_manager, s_setup, s_addw, s_clear;
extern ID s_remove, s_finish, s_size;

/*  FUNC_M generate two functions here
 *  shoes_canvas_c_layout(int argc, VALUE *argv, VALUE self) { ..}
 *      + means call shoes_canvas_repaint_all() at end
 *  shoes_app_c_layout(int argc, VALUE *argv, VALUE self) {...}
 */ 
FUNC_M("+layout", layout, -1);

void shoes_layout_init() {
  cLayout = rb_define_class_under(cTypes, "Layout", cShoes);
  rb_define_method(cLayout, "clear", CASTHOOK(shoes_layout_clear), -1);
  rb_define_method(cLayout, "insert", CASTHOOK(shoes_layout_insert), -1);
  rb_define_method(cLayout, "delete_at", CASTHOOK(shoes_layout_delete_at), -1);
  rb_define_method(cLayout, "rules", CASTHOOK(shoes_layout_add_rules), -1);  
  rb_define_method(cLayout, "finish", CASTHOOK(shoes_layout_finish), -1);
  
  rb_define_method(cLayout, "height", CASTHOOK(shoes_layout_get_height), 0);
  rb_define_method(cLayout, "width", CASTHOOK(shoes_layout_get_width), 0);
  rb_define_method(cLayout, "start", CASTHOOK(shoes_layout_start), -1);
  rb_define_method(cLayout, "style", CASTHOOK(shoes_layout_style), -1);
  
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
  ly->delegate = Qnil;
  ly->canvas = Qnil;
  return obj;
}

VALUE shoes_layout_new(VALUE attr, VALUE parent) {
    //fprintf(stderr, "shoes_layout_new called\n");
    VALUE obj = shoes_layout_alloc(cLayout);
    shoes_layout *lay;
    Data_Get_Struct(obj, shoes_layout, lay);
    // Most of shoes thinks its a Flow (cFlow)
    VALUE hgt = ATTR(attr, height);
    if (NIL_P(hgt)) {
      ATTRSET(attr, height, INT2NUM(100));
    }
    // user layout starts out as hidden. - Don't - won't have h/w set
    //ATTRSET(attr, hidden, Qtrue);
    VALUE canvas = shoes_slot_new(cFlow, attr, parent);
    lay->canvas = canvas;
    shoes_canvas *cvs;
    Data_Get_Struct(canvas, shoes_canvas, cvs);
    VALUE mgr;
    s_manager = rb_intern ("use");
    s_setup = rb_intern("setup");
    s_addw = rb_intern("add");
    s_clear = rb_intern("clear");
    // get manager from attr, put in delegate.
    mgr = ATTR(attr, manager);
    if (SYMBOL_P(mgr) || NIL_P(mgr)) {
      lay->mgr = 0; // internal
      if (NIL_P(mgr))
        lay->mgr = Layout_None;
      else {
        ID  mgrid = SYM2ID(mgr);
        if (mgrid == rb_intern("Vfl"))
          lay->mgr = Layout_VFL;
        else if (mgrid == rb_intern("constraints"))
          lay->mgr = Layout_Constraints;
        else if (mgrid == rb_intern("grid"))
          lay->mgr = Layout_Grid;
        else {
          lay->mgr = Layout_None;
        }
      }
      shoes_layout_internal_setup(lay, cvs, attr);
    } else {
      // verify class implements required methods
      if (!rb_respond_to(mgr, s_setup))
        rb_raise(rb_eArgError, "'setup' not implment in Layout");
      if (!rb_respond_to(mgr, s_addw))
        rb_raise(rb_eArgError, "'add' not implment in Layout");
      if (!rb_respond_to(mgr, s_remove))
        rb_raise(rb_eArgError, "'remove' not implment in Layout");
      if (!rb_respond_to(mgr, s_finish))
        rb_raise(rb_eArgError, "'finish' not implment in Layout");
      if (!rb_respond_to(mgr, s_clear))
        rb_raise(rb_eArgError, "'clear' not implment in Layout");
      if (!rb_respond_to(mgr, s_size))
        rb_raise(rb_eArgError, "'size' not implment in Layout");
       
      lay->delegate = mgr;
      rb_funcall(mgr, s_setup, 2, canvas, attr);
    }
    cvs->layout_mgr = obj; // me
    return obj;
}

VALUE shoes_layout_insert(int argc, VALUE *argv, VALUE self) {
    shoes_layout *lay;
    Data_Get_Struct(self, shoes_layout, lay);
    VALUE canvas = lay->canvas;
    rb_arg_list args;
    rb_parse_args(argc, argv, "i&", &args);  
    long pos = NUM2LONG(args.a[0]);
    shoes_canvas_insert(canvas, pos, Qnil, args.a[1]);
    return self;
}

VALUE shoes_layout_delete_at(int argc, VALUE *argv, VALUE self) {
    shoes_layout *lay;
    Data_Get_Struct(self, shoes_layout, lay);
    VALUE canvas_obj = lay->canvas;
    shoes_canvas *canvas;
    Data_Get_Struct(canvas_obj, shoes_canvas, canvas);
    rb_arg_list args;
    rb_parse_args(argc, argv, "i", &args); 
    long pos = NUM2LONG(args.a[0]);
    VALUE ele = rb_ary_entry(canvas->contents, pos);
    VALUE ok;
    if (lay->delegate)
      ok = rb_funcall(lay->delegate, s_remove, 3, canvas_obj, ele, args.a[0]);
    else
      ok = shoes_layout_internal_delete_at(lay, canvas, ele, pos);
    if (! NIL_P(ok)) {
      // normal elements respond to 'remove' themselves
      if (rb_respond_to(ele, s_remove))
        rb_funcall(ele, s_remove, 0, Qnil);
    }
    return self;
}

VALUE shoes_layout_clear(int argc, VALUE *argv, VALUE self) {
    shoes_layout *lay;
    Data_Get_Struct(self, shoes_layout, lay);
    VALUE canvas = lay->canvas;
    shoes_canvas_clear_contents(argc, argv, canvas);
    return Qnil;
}

VALUE shoes_layout_style(int argc, VALUE *argv, VALUE self) {
    shoes_layout *lay;
    Data_Get_Struct(self, shoes_layout, lay);
    VALUE canvas = lay->canvas;
    shoes_canvas_style(argc, argv, canvas);
  return Qtrue;
}


VALUE shoes_layout_finish(int argc, VALUE *argv, VALUE self) {
	//fprintf(stderr, "shoes_layout_finish called\n");
	shoes_layout *lay;
	Data_Get_Struct(self, shoes_layout, lay);
	shoes_canvas *canvas;
	Data_Get_Struct(lay->canvas, shoes_canvas, canvas);
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    if (! NIL_P(ly->delegate)) {
      VALUE del = ly->delegate;
      rb_funcall(del, s_finish, 0);
      return Qtrue;
    }
  } 
  // here if no delgate or no manager object
  shoes_layout_internal_finish(canvas);
  return Qtrue; 
}

VALUE shoes_layout_get_height(VALUE self) {
  //fprintf(stderr,"shoes_layout_get_height called\n");
	shoes_layout *lay;
	Data_Get_Struct(self, shoes_layout, lay);
	shoes_canvas *canvas;
	Data_Get_Struct(lay->canvas, shoes_canvas, canvas);
  return INT2NUM(canvas->height);
  
}


VALUE shoes_layout_get_width(VALUE self) {
  //fprintf(stderr,"shoes_layout_get_width called\n");
	shoes_layout *lay;
	Data_Get_Struct(self, shoes_layout, lay);
	shoes_canvas *canvas;
	Data_Get_Struct(lay->canvas, shoes_canvas, canvas);
  return INT2NUM(canvas->width);
}


VALUE shoes_layout_start(int argc, VALUE *argv, VALUE self) {
	shoes_layout *lay;
	Data_Get_Struct(self, shoes_layout, lay);
  shoes_canvas_start(argc, argv, lay->canvas);
  return Qtrue;
}

VALUE shoes_layout_add_rules(int argc, VALUE *argv, VALUE self) {
  // TODO: call parse_args once we figure out what it is.
  VALUE arg;
  if (argc < 1)
    arg = Qnil;
  else
    arg = argv[1];
	shoes_layout *lay;
	Data_Get_Struct(self, shoes_layout, lay);
	shoes_canvas *canvas;
	Data_Get_Struct(lay->canvas, shoes_canvas, canvas);
  if (! NIL_P(lay->delegate)) {
      ID s_rules = rb_intern("rules");
      if (rb_respond_to(lay->delegate, s_rules))
        rb_funcall(lay->delegate, s_rules, 1, arg);
      else
        rb_raise(rb_eArgError, "'rules' not implmented in Layout");
  } else {
    shoes_layout_internal_rules(lay, canvas, arg);
  }
  return Qtrue; 
}

// --------------- called from canvas internals ------------

void shoes_layout_size(shoes_canvas *canvas, int pass) {
  //fprintf(stderr,"shoes_layout_size called\n");
  shoes_layout *lay;
  Data_Get_Struct(canvas->layout_mgr, shoes_layout, lay);
  if ((canvas->height != lay->cache_h) || (lay->cache_w != canvas->width)) {
    lay->cache_h = canvas->height;
    lay->cache_w = canvas->width;
  } else {
    return;
  }
  // Find a delegate or use the internal internal?
  if (lay->delegate != Qnil) {
    shoes_canvas *cvs;
    Data_Get_Struct(lay->canvas, shoes_canvas, cvs);
    VALUE del = lay->delegate;
    rb_funcall(lay->delegate, s_size, 2, lay->canvas, INT2NUM(pass));
  } else {
    // here if no manager object
    shoes_layout_internal_size(lay, canvas, pass);
  }
  return;   
}


// called from shoes_add_ele (def in canvas.c) by widget creators
// The ele has already been added to canvas->contents
void shoes_layout_add_ele(shoes_canvas *canvas, VALUE ele) {
  if (rb_obj_is_kind_of(ele, cBackground)) {
    //fprintf(stderr, "skipping background widget\n");
    return;
  }
  // Find a delegate or use the internal?
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    shoes_canvas *cvs;
    Data_Get_Struct(ly->canvas, shoes_canvas, cvs);
    if (! NIL_P(ly->delegate)) {
      VALUE del = ly->delegate;
			shoes_abstract *widget;
			Data_Get_Struct(ele, shoes_abstract, widget);
			rb_funcall(del, s_addw, 3, ly->canvas, ele, widget->attr);
      return;
    }
  } 
  // here if no delgate or no manager object
  shoes_layout_internal_add(canvas, ele);
  return; 
}

// called from inside shoes (shoes_canvas_clear)
void shoes_layout_cleared(shoes_canvas *canvas) {
  //fprintf(stderr,"shoes_layout_clear called\n");
  if (canvas->layout_mgr != Qnil) {
    shoes_layout *ly;
    Data_Get_Struct(canvas->layout_mgr, shoes_layout, ly);
    if (! NIL_P(ly->delegate)) {
      VALUE del = ly->delegate;
      rb_funcall(del, s_clear, 0);
      return;
    }
  } 
  // here if no delgate or no manager object
  shoes_layout_internal_clear(canvas);
  return; 
}


/* 
 * Methods to call the internal layout manager - 
 * Might be a cassawory variant? grid_bag? Something Gtk or Cocoa? 
*/

void shoes_layout_internal_setup(shoes_layout *lay, shoes_canvas *canvas,
      VALUE attr) {
  if (lay->mgr == Layout_VFL) 
    fprintf(stderr, "Vfl internal layout setup called\n");
  else
    fprintf(stderr, "shoes_layout_internal_setup called\n");
}

void shoes_layout_internal_add(shoes_canvas *canvas, VALUE ele) {
  fprintf(stderr, "shoes_layout_internal_add called\n");
}

void shoes_layout_internal_clear(shoes_canvas *canvas) {
  fprintf(stderr, "shoes_layout_internal_clear called\n");
}

void shoes_layout_internal_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE arg) {
  fprintf(stderr, "shoes_layout_internal_rules called\n");
}

void shoes_layout_internal_finish(shoes_canvas *canvas) {
	fprintf(stderr,"shoes_layout_internal_finish called\n");
}

void shoes_layout_internal_size(shoes_layout *lay, shoes_canvas *canvas, int pass) {
  fprintf(stderr,"shoes_layout_internal_size called pass: %d\n", pass);
}

VALUE shoes_layout_internal_delete_at(shoes_layout *lay, shoes_canvas *canvas,
      VALUE ele, int pos) {
  fprintf(stderr, "shoes_layout_internal_delete_at called\n");
  return Qtrue;
}
