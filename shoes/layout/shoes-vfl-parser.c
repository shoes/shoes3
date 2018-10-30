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
#include "shoes/layout/layouts.h"

#include "shoes/layout/emeus-vfl-parser-private.h"
// Test data from emeus/examples/simple-grid.c


/* Layout:
 *
 *   +-----------------------------+
 *   | +-----------+ +-----------+ |
 *   | |  Child 1  | |  Child 2  | |
 *   | +-----------+ +-----------+ |
 *   | +-------------------------+ |
 *   | |         Child 3         | |
 *   | +-------------------------+ |
 *   +-----------------------------+
 *
 * Visual format:
 *
 *   H:|-8-[view1(==view2)]-12-[view2]-8-|
 *   H:|-8-[view3]-8-|
 *   V:|-8-[view1,view2]-12-[view3(==view1,view2)]-8-|
 */
 char *lines[3] = {
   "H:|-8-[view1(==view2)]-12-[view2]-8-|",
   "H:|-8-[view3]-8-|",
   "V:|-8-[view1,view2]-12-[view3(==view1,view2)]-8-|"
 };
 /* The third line fails to parse: A predicate must follow a view name
  * 
  * vfl calls these 'views'. In shoes this would be the list of all the
  * widget :names in a the layout. 
 */ 
 char *names[3] = { "view1", "view2", "view3" }; 
 
 /* Constraints:
 *
 *   super.start = child1.start - 8
 *   child1.width = child2.width
 *   child1.end = child2.start - 12
 *   child2.end = super.end - 8
 *   super.start = child3.start - 8
 *   child3.end = super.end - 8
 *   super.top = child1.top - 8
 *   super.top = child2.top - 8
 *   child1.bottom = child3.top - 12
 *   child2.bottom = child3.top - 12
 *   child3.height = child1.height
 *   child3.height = child2.height
 *   child3.bottom = super.bottom - 8
 *
 */


void  print_constraint(VflConstraint *c) {
  fprintf(stderr,"{\n"
           "  .view1: '%s',\n"
           "  .attr1: '%s',\n"
           "  .relation: '%d',\n"
           "  .view2: '%s',\n"
           "  .attr2: '%s',\n"
           "  .constant: %g,\n"
           "  .multiplier: %g,\n"
           "  .strength: %g\n"
           "}\n",
           c->view1,
           c->attr1,
           c->relation,
           c->view2 != NULL ? c->view2 : "none",
           c->attr2 != NULL ? c->attr2 : "none",
           c->constant,
           c->multiplier,
           c->strength);
}

extern ID s_name;

static VALUE wrap_constraint(VflConstraint *c) {
  VALUE hsh = rb_hash_new();
  VALUE val, key;
  key = ID2SYM(rb_intern("view1"));
  val = rb_str_new2(c->view1);
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("attr1"));
  val = rb_str_new2(c->attr1);
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("relation"));
  if (c->relation == OPERATOR_TYPE_LE)
    val = rb_str_new2("LE");
  else if (c->relation == OPERATOR_TYPE_GE)
    val = rb_str_new2("GE");
  else
    val = rb_str_new2("EQ");

  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("view2"));
  if (c->view2 != NULL)
    val = rb_str_new2(c->view2);
  else
    val = Qnil;
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("attr2"));
  if (c->view2 != NULL)
    val = rb_str_new2(c->attr2);
  else
    val = Qnil;
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("constant"));
  val = DBL2NUM(c->constant);
  rb_hash_aset(hsh, key, val);
  
  key = ID2SYM(rb_intern("multiplier"));
  val = DBL2NUM(c->multiplier);
  rb_hash_aset(hsh, key, val);
  
  key = ID2SYM(rb_intern("strength"));
  val = DBL2NUM(c->strength);
  rb_hash_aset(hsh, key, val);
  
  return hsh;
  
}


// args is a verified Ruby hash.
VALUE shoes_vfl_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE args) {
  GError *error = NULL;
  int hspacing = 10;
  int vspacing = 10;
  GHashTable *views, *metrics;
  VflConstraint *constraints;
  int n_constraints;
  
  //fprintf(stderr, "in vfl parse test\n");
  // create a parser
  VflParser *parser = vfl_parser_new (-1, -1, NULL, NULL);
  
  // get view names (shoes ele's) 
  VALUE keys;
  ID s_views = rb_intern("views");
  VALUE hashv = rb_hash_aref(args, ID2SYM(s_views));
  if (! NIL_P(hashv)) {
    if (TYPE(hashv) == T_HASH) {
      keys = rb_funcall(hashv, rb_intern("keys"), 0);
      lay->views = hashv;
    } else
      rb_raise(rb_eArgError, "views: is not a hash");
  } else {
    // unspecifed, try the one in the Layout obj - should be nil
    keys = rb_funcall(lay->views, rb_intern("keys"), 0);
  }
  views = g_hash_table_new (g_str_hash, g_str_equal);
  for (int i = 0; i < RARRAY_LEN(keys); i++) { 
    VALUE ent = rb_ary_entry(keys, i); 
    char *str = strdup(RSTRING_PTR(ent));
    //printf("view |%s|\n", str);
    g_hash_table_add (views, str);
  }
  vfl_parser_set_views (parser, views);

  // Move any metrics from Ruby to glib hash 'key' -> double
  ID s_metrics = rb_intern("metrics");
  hashv = rb_hash_aref(args, ID2SYM(s_metrics));
  if (! NIL_P(hashv)) {
    if (TYPE(hashv) == T_HASH) {
      keys = rb_funcall(hashv, rb_intern("keys"), 0);
      lay->metrics = hashv;
    }
    else
      rb_raise(rb_eArgError, "views: is not a hash");
  } else {
    // unspecifed, try the one in the Layout obj - should be nil
    keys = rb_funcall(lay->metrics, rb_intern("keys"), 0);
  }
  metrics = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  for (int i = 0; i < RARRAY_LEN(keys); i++) {
    const char *str;
    ID metk;
    VALUE metv;
    double *v = g_new(double, 1);
    
    VALUE ent = rb_ary_entry(keys, i);
    if (TYPE(ent) == T_SYMBOL) 
      str = rb_id2name(SYM2ID(ent));
    else if (TYPE(ent) == T_STRING) 
      str = RSTRING_PTR(ent);
    else 
      rb_raise(rb_eArgError, "vfl metric key not string or symbol");
    metv = rb_hash_aref(lay->metrics, ent);
    if (NIL_P(metv)) {
      rb_raise(rb_eArgError, "vfl metric should not be nil for %s", str);
    } else {
      *v  = NUM2DBL(metv);
      //fprintf(stderr,"metrics key: %s => %g\n", str, *v);
    }
   g_hash_table_insert(metrics, (char *)str, v);
  }
  vfl_parser_set_metrics (parser, metrics);
  
  // get lines, check for array
  ID s_lines = rb_intern("lines");
  VALUE lnary = rb_hash_aref(args, ID2SYM(s_lines));
  if (TYPE(lnary) != T_ARRAY)
    rb_raise(rb_eArgError, "missing lines: array?");
  
  // feed lines to parser 
  lay->rbconstraints = rb_ary_new();
  for (int i = 0; i < RARRAY_LEN(lnary); i++) {
    VALUE ln = rb_ary_entry(lnary, i);
    char *line = RSTRING_PTR(ln);
    //fprintf(stderr, "parse: %s\n", line);
    vfl_parser_parse_line (parser, line, -1, &error);
    if (error != NULL) {
      char err[100];
      rb_raise(rb_eArgError, "vfl err: %s", error->message);
    } else {
      // get constraints out
      constraints = vfl_parser_get_constraints (parser, &n_constraints);
      //fprintf(stderr, "n_constraints: %d\n", n_constraints);
      for (int i = 0; i < n_constraints; i++) {
         VflConstraint *c = &constraints[i];
         rb_ary_push(lay->rbconstraints, wrap_constraint(c));
      }
    }
  }
  return Qtrue;
}
