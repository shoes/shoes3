/*
 * cassowary-constraint.c 
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
#include "shoes/layout/shoes-vfl.h"

/*
 * There isn't much a user can do with this class - just create an object
 * and pass it to a layout. (or recieve them from a layout method). Perhaps
 * load and save a batch of them. 
 * since it's not intended for common use we won't polute canvas and
 * app namespace with methods.
 * 
 * usage: Shoes::Constraint.new(args...)
 * You CAN NOT modify it later. By Design. 
 */
ID cCassowaryconstraint;

void shoes_cassowary_constraint_init() {
  fprintf(stderr, "init constraint class\n");
  cCassowaryconstraint = rb_define_class_under(cShoes, "Constraint", cShoes);
  rb_define_method(cCassowaryconstraint, "initialize", CASTHOOK(shoes_cassowary_constraint_new), -1);
}

void shoes_cassowary_constraint_mark(shoes_cassowary_constraint *ct) {
  rb_gc_mark_maybe(ct->target_object);
}

static void shoes_cassowary_constraint_free(shoes_cassowary_constraint *ct) {
  RUBY_CRITICAL(SHOE_FREE(ct));
}
#ifdef NEW_MACRO_CASSOWARY
TypedData_Type_New(shoes_cassowary_constraint);
#endif

VALUE shoes_cassowary_constraint_alloc(VALUE klass) {
  //fprintf(stderr,"shoes_cassowary_constraint_alloc called\n");
  VALUE obj;
  shoes_cassowary_constraint *ct = SHOE_ALLOC(shoes_cassowary_constraint);
  SHOE_MEMZERO(ct, shoes_cassowary_constraint, 1);
#ifdef NEW_MACRO_CASSOWARY
  obj = TypedData_Wrap_Struct(klass, &shoes_cassowary_constraint_type, ct);
#else
  obj = Data_Wrap_Struct(klass, shoes_cassowary_constraint_mark, shoes_cassowary_constraint_free, ct);
#endif
  // init fields 
  ct->target_object = Qnil;
  ct->target_attr = Qnil;
  ct->relation = Qnil;
  ct->source_object = Qnil;
  ct->source_attr = Qnil;
  ct->multiplier = Qnil;
  ct->constant = Qnil;
  ct->strength = Qnil;
  return obj;
}

/*
 * usage: Shoes::Constraint.new(args...)
 * convert from Ruby (args) to Em
 */
VALUE shoes_cassowary_constraint_new(int argc, VALUE *argv, VALUE self) {
  fprintf(stderr, "shoes_cassowary_constraint_new called\n");
  VALUE obj = shoes_cassowary_constraint_alloc(cCassowaryconstraint);
#ifdef NEW_MACRO_CASSOWARY
  Get_TypedStruct2(obj, shoes_cassowary_constraint, ct);
#else
  shoes_cassowary_constraint *ct;
  Data_Get_Struct(obj, shoes_cassowary_constraint, ct);
#endif
  if (argc == 8) {
    ct->target_object = argv[0];
    ct->target_attr = argv[1];
    ct->relation = argv[2];
    ct->source_object = argv[3];
    ct->source_attr = argv[4];
    ct->multiplier = argv[5];
    ct->constant = argv[6];
    ct->strength = argv[7];
  } else if (TYPE(argv[0])==T_HASH) {
    // Todo? 
  } else
      rb_raise(rb_eArgError, "incorrect args to create constraint");
  
  return obj;
}

