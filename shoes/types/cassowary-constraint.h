#ifndef SHOES_CASSOWARY_TYPE_H
#define SHOES_CASSOWARY_TYPE_H

typedef struct {
  VALUE target_object;
  VALUE target_attr;
  VALUE relation;
  VALUE source_object;
  VALUE source_attr;
  VALUE multiplier;
  VALUE constant;
  VALUE strength;
} shoes_cassowary_constraint;

extern VALUE cLayout;
void shoes_cassowary_constraint_init();
VALUE shoes_cassowary_constraint_alloc(VALUE klass);
VALUE shoes_cassowary_constraint_new(int argc, VALUE *argv, VALUE self);

#endif
