/*
 *  Interface between Shoes (internal layout) and emeus code
 *  Emeus has A LOT of Gobject/Glib stuff which we keep. Some of the
 *  emeus code has been deleted (the gtk interface builder for example).
 *  
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
#include "shoes/layout/gshoes-ele.h"
#include "shoes/layout/shoes-vfl.h"

#include "shoes/layout/emeus-vfl-parser-private.h"
#include "shoes/layout/emeus-constraint.h"
#include "shoes/types/cassowary-constraint.h"
extern ID s_name, cCassowaryconstraint;

// forward declares in this file 
void shoes_vfl_add_layout_stays (EmeusConstraintLayout *self);
VALUE shoes_vfl_wrap_emeus(EmeusConstraint *emc);
VALUE shoes_vfl_hash_emeus(EmeusConstraint *c);
EmeusConstraintAttribute shoes_vfl_attr_from_str(char *attr);
EmeusConstraintRelation shoes_vfl_rel_from_str(char *rel);
// Temporary - just enough to link on non gtk platforms. WARNING.

SimplexSolver * emeus_constraint_layout_get_solver(EmeusConstraintLayout *layout)
{
  return layout->solver;
}

void  emeus_constraint_layout_activate_constraint (EmeusConstraintLayout *layout,
     EmeusConstraint *constraint) 
{
  printf("emeus_constraint_layout_activate_constraint called\n");
}

void emeus_constraint_layout_deactivate_constraint(EmeusConstraintLayout *layout,
    EmeusConstraint *constraint) 
{
  printf("emeus_constraint_layout_deactivate_constraint called\n");
}


// --------- implement shoes usr layout protocol for vfl/emeus  ----------- 


void shoes_vfl_setup(shoes_layout *lay, shoes_canvas *canvas, VALUE attr) {
  fprintf(stderr, "shoes_vfl_setup called\n");
  // create OUR layout struct - different from the shoes_layout, sort of.
  EmeusConstraintLayout *layout;
  layout = malloc(sizeof(EmeusConstraintLayout));
  // make a pointer to the solver (aka context, tableau)
  SimplexSolver *solver = malloc(sizeof (SimplexSolver));
  solver->initialized = 0;
  simplex_solver_init (solver);
  layout->solver = solver;
  layout->children = NULL;
  layout->shoes_contents = lay;  // cross link 
  lay->root = (void *)layout;
  
  // get height and width from attr
  VALUE hgtobj, widobj;
  int wid, hgt = 0;
  widobj = ATTR(attr, width);
  if (! NIL_P(widobj))
    wid = NUM2INT(widobj);
  hgtobj = ATTR(attr, height);
  if (! NIL_P(hgtobj))
    hgt = NUM2INT(hgtobj);
    
  // init hash tables for elements (views) and metrics
  layout->views = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  layout->metrics = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
    
  // Create Variables and stays for the Layout's canvas.
  layout->bound_attributes = g_hash_table_new_full (NULL, NULL,
                                                  NULL,
                                                  (GDestroyNotify) variable_unref);
  layout->constraints = g_hash_table_new_full (NULL, NULL,
                                             g_object_unref,
                                             NULL);

  shoes_vfl_add_layout_stays (layout);
}

void shoes_vfl_add_ele(shoes_canvas *canvas, VALUE ele) {
  shoes_layout *lay;
  Data_Get_Struct(canvas->layout_mgr, shoes_layout, lay);
  shoes_abstract *widget;
	Data_Get_Struct(ele, shoes_abstract, widget);
  VALUE name = ATTR(widget->attr, name);
  if (NIL_P(name)) 
    rb_raise(rb_eArgError, "please supply a name: ");
  char *str = RSTRING_PTR(name);
  rb_hash_aset(lay->views, name, ele);
  /*
   *  create Variables/constraints for 'ele'. At this point in time
   *  we don't have w or h for the elements
  */
}

void shoes_vfl_delete_at(shoes_layout *lay, shoes_canvas *canvas, VALUE ele,
      int pos)
{
  fprintf(stderr, "shoes_vfl_delete_at called\n");
}

void shoes_vfl_clear(shoes_layout *lay, shoes_canvas *canvas)
{
  fprintf(stderr, "shoes_vfl_clear called\n");
}

void shoes_vfl_size(shoes_layout *lay, shoes_canvas *canvas, int pass) {
  fprintf(stderr, "shoes_vfl_size pass: %d  called\n", pass);
}
 
void shoes_vfl_finish(shoes_layout *lay, shoes_canvas *canvas) {
	fprintf(stderr,"shoes_vfl_finish called\n");
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  // Now What ?
  
#if 0  // height,top,width,left - the stays.
  GHashTableIter iter;
  gpointer key, value;
  
  g_hash_table_iter_init (&iter, layout->bound_attributes);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
      /* do something with key and value */
      char *str = (char *)key;
      EmeusConstraintAttribute *attr = (EmeusConstraintAttribute *)value;
      fprintf(stderr, "attr: %s\n", str);
  }
#endif 
}

void shoes_vfl_add_contraints(shoes_layout *lay, shoes_canvas *canvas, VALUE arg)
{
  fprintf(stderr, "shoes_vfl_add_contraints called\n");
  if (! rb_obj_is_kind_of(arg, cCassowaryconstraint))
    rb_raise(rb_eArgError, "arg must be a Shoes::Constraint object");
  // convert to emeus and add to solver.
  shoes_cassowary_constraint *cs;
  Data_Get_Struct(arg, shoes_cassowary_constraint, cs);
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  gpointer source, target;
	EmeusConstraintAttribute source_attr, target_attr;
	EmeusConstraintRelation relation;
  double multiplier,constant;
  int strength;
  
  // target 
  if (NIL_P(cs->target_object))
    target = NULL;
  else {
    char *name = RSTRING_PTR(cs->target_object);
    VALUE rbv = rb_hash_aref(lay->views, cs->target_object);
    if (NIL_P(rbv))
      rb_raise(rb_eArgError,"no Shoes element named: %s", name);
    
    // get the gshoes_ele from layout->views - 
    target = g_hash_table_lookup(layout->views, name);
    if (target == NULL) {
      // make a gshoes_ele
      GString *gstr = g_string_new(name);
      target = (gpointer)gshoes_ele_new(gstr, (gpointer)rbv);
    }
  }
  VALUE rba = cs->target_attr;
  char *rbas =RSTRING_PTR(rba);
  target_attr = attribute_from_name(rbas);  // emeues-types.c
  
  rba = cs->relation;
  relation = shoes_vfl_rel_from_str(RSTRING_PTR(rba));
  
  if (! NIL_P(cs->source_attr)) {
  
    if (NIL_P(cs->source_object))
      source = NULL;
    else {
      char *name = RSTRING_PTR(cs->source_object);
      VALUE rbv = rb_hash_aref(lay->views, cs->source_object);
      if (NIL_P(rbv))
        rb_raise(rb_eArgError,"no Shoes element named: %s", name);
      
      // get the gshoes_ele from layout->views - 
      source = g_hash_table_lookup(layout->views, name);
      if (source == NULL) {
        // make a gshoes_ele
        GString *gstr = g_string_new(name);
        source = (gpointer)gshoes_ele_new(gstr, (gpointer)rbv);
      }
    }
    VALUE rba2 = cs->source_attr;
    source_attr = attribute_from_name(RSTRING_PTR(rba2));
    
    VALUE mub = cs->multiplier;
    multiplier = NUM2DBL(mub);
  }
  VALUE rbc = cs->constant;
  constant = NUM2DBL(rbc);
  
  if (TYPE(cs->strength) == T_STRING) {
    char *s = RSTRING_PTR(cs->strength);
  } else {
    strength = NUM2INT(cs->strength);
  }
  if (NIL_P(cs->source_attr)) {
    // TODO: Testing and more testing
    emeus_constraint_new_constant (target, target_attr, relation,
        constant, strength);
  } else {
    emeus_constraint_new(target, target_attr, relation, source, source_attr,
      multiplier, constant, strength);
  }
}

VALUE shoes_vfl_parse(shoes_layout *lay, shoes_canvas *canvas, VALUE args)
{
  fprintf(stderr, "shoes_vfl_parse called\n");
  GError *error = NULL;
  //int hspacing = 10;
  //int vspacing = 10;
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  
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
  //views = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  for (int i = 0; i < RARRAY_LEN(keys); i++) { 
    VALUE ent = rb_ary_entry(keys, i); 
    GString *str = g_string_new(RSTRING_PTR(ent));
    // we want the Shoes ele to match the name (key)
    VALUE ele = rb_hash_aref(lay->views, ent);
    GshoesEle *gele = gshoes_ele_new(str, (gpointer)ele);
    g_hash_table_insert(layout->views, RSTRING_PTR(ent), gele);
    // create an EmuesChildLayout (equiv) here? Create solver variables?
  }

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
  //metrics = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
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
   g_hash_table_insert(layout->metrics, (char *)str, v);
  }
  
  // convert lines. 
  // get lines, check for array
  ID s_lines = rb_intern("lines");
  VALUE lnary = rb_hash_aref(args, ID2SYM(s_lines));
  if (TYPE(lnary) != T_ARRAY)
    rb_raise(rb_eArgError, "missing lines: array?");
  
  int n_lines = RARRAY_LEN(lnary);
  const char *lines[n_lines];  // that used to be illegal in C
  for (int i = 0; i <n_lines; i++) {
    lines[i] = strdup(RSTRING_PTR(rb_ary_entry(lnary, i)));
  }
  
  // store the glist in EmeusConstrainLayout->parsed_constraints
  layout->parsed_constraints = emeus_create_constraints_from_description(
      lines, n_lines, -1, -1, layout->views, layout->metrics);
  // finish() will move them into the Solver somehow. 
  // In Theory. May not be need - the constraints are in the solver?
  // Don't forget to free the lines array strings
  for (int i = 0; i < n_lines; i++) {
    free((void *)lines[i]);
  }
  return Qtrue;
}

VALUE shoes_vfl_get_constraints(shoes_layout *lay, shoes_canvas *canvas)
{
  fprintf(stderr, "shoes_vfl_get_constraints called\n");
  // returns array of the hash wrapping for a constraints in the 
  lay->rbconstraints = rb_ary_new();
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  for (GList *ptr = layout->parsed_constraints; ptr != NULL; ptr = ptr->next) {
		EmeusConstraint *emc = (EmeusConstraint *)ptr->data;
		rb_ary_push(lay->rbconstraints, shoes_vfl_wrap_emeus(emc));
	}
  return lay->rbconstraints;
}

VALUE shoes_vfl_get_constraints_hash(shoes_layout *lay, shoes_canvas *canvas)
{
  fprintf(stderr, "shoes_vfl_get_constraints called\n");
  // returns array of the hash wrapping for a constraints in the 
  lay->rbconstraints = rb_ary_new();
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  for (GList *ptr = layout->parsed_constraints; ptr != NULL; ptr = ptr->next) {
		EmeusConstraint *emc = (EmeusConstraint *)ptr->data;
		rb_ary_push(lay->rbconstraints, shoes_vfl_hash_emeus(emc));
	}
  return lay->rbconstraints;
}


// -------------- internels, helpers - no Ruby api into these functions ---------

gboolean shoes_vfl_is_element(GshoesEle *p) {
  if (! GSHOES_IS_ELE(p))
    return false;
  shoes_abstract *ab;
  gpointer gp = gshoes_ele_get_element(p);
  Data_Get_Struct((VALUE)gp, shoes_abstract, ab);
  shoes_canvas *canvas;
  Data_Get_Struct(ab->parent, shoes_canvas, canvas);
  if (NIL_P(canvas->layout_mgr))
    return false;
  return true;
}

void shoes_vfl_add_layout_stays(EmeusConstraintLayout *self) {
   Variable *var;

  /* Add two required stay constraints for the top left corner */
  var = simplex_solver_create_variable (self->solver, "top", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_TOP),
                       var);
  self->stays.top =
    simplex_solver_add_stay_variable (self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (self->solver, "left", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_LEFT),
                       var);
  self->stays.left =
    simplex_solver_add_stay_variable (self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (self->solver, "width", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH),
                       var);
  self->stays.width =
    simplex_solver_add_stay_variable (self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (self->solver, "height", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT),
                       var);
  self->stays.height =
    simplex_solver_add_stay_variable (self->solver, var, STRENGTH_WEAK);
}


VALUE shoes_vfl_hash_emeus(EmeusConstraint *c) {
  gpointer source, target;
	EmeusConstraintAttribute source_attr, target_attr;
	EmeusConstraintRelation relation;

  VALUE hsh = rb_hash_new();
  VALUE val, key;
  key = ID2SYM(rb_intern("target_object"));
  gpointer p = emeus_constraint_get_target_object(c);
  shoes_abstract *ab;
  if (! GSHOES_IS_ELE(p))
    val = Qnil;
  else {
    gpointer gp = gshoes_ele_get_element(p);
    Data_Get_Struct((VALUE)gp, shoes_abstract, ab);
    VALUE obj = ATTR(ab->attr, name);
    if (NIL_P(obj))
      val = Qnil;
    else
      val = obj;
  }
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("target_attr"));
  target_attr = emeus_constraint_get_target_attribute(c);
  val = rb_str_new2(get_attribute_name(target_attr));
  rb_hash_aset(hsh, key, val);

  key = ID2SYM(rb_intern("relation"));
  relation = emeus_constraint_get_relation(c);
  val = rb_str_new2(get_relation_symbol(relation));
  rb_hash_aset(hsh, key, val);
  
  if (target_attr != EMEUS_CONSTRAINT_ATTRIBUTE_INVALID) {
    key = ID2SYM(rb_intern("source_object"));
     p = emeus_constraint_get_source_object(c);
      shoes_abstract *ab;
      if (! GSHOES_IS_ELE(p))
        val = rb_str_new2("parent");
      else {
        gpointer gp = gshoes_ele_get_element(p);
        Data_Get_Struct((VALUE)gp, shoes_abstract, ab);
        VALUE obj = ATTR(ab->attr, name);
        if (NIL_P(obj))
          val = Qnil;
        else
          val = obj;
      }
    rb_hash_aset(hsh, key, val);
    key = ID2SYM(rb_intern("source_attr"));
    source_attr = emeus_constraint_get_source_attribute(c);
    val = rb_str_new2(get_attribute_name(source_attr));
    rb_hash_aset(hsh, key, val);
    
    double d = emeus_constraint_get_multiplier(c);
    if (fabs (d - 1.0) > DBL_EPSILON) {
      key = ID2SYM(rb_intern("multiplier"));
      val = DBL2NUM(d);
      rb_hash_aset(hsh, key, val);
    }
  }
  double d = emeus_constraint_get_constant(c);
  if (fabs (d - 1.0) > DBL_EPSILON) {
    key = ID2SYM(rb_intern("constant"));
    val = DBL2NUM(d);
    rb_hash_aset(hsh, key, val);
  }
  int strength = emeus_constraint_get_strength(c);
  key = ID2SYM(rb_intern("strength"));
  val = INT2NUM(strength);
  //val = rb_str_new2(strength_to_string (strength));
  rb_hash_aset(hsh, key, val);  

  return hsh;
  
}

// Create Shoes::Constraint from EmeusConstraint
VALUE shoes_vfl_wrap_emeus(EmeusConstraint *c) {
  gpointer source, target;
	EmeusConstraintAttribute source_attr, target_attr;
	EmeusConstraintRelation relation;

  VALUE csobj = shoes_cassowary_constraint_alloc(cCassowaryconstraint);
  shoes_cassowary_constraint *ct;
  Data_Get_Struct(csobj, shoes_cassowary_constraint, ct);
  VALUE val;
  gpointer p = emeus_constraint_get_target_object(c);
  shoes_abstract *ab;
  if (! GSHOES_IS_ELE(p))
    val = Qnil;
  else {
    gpointer gp = gshoes_ele_get_element(p);
    Data_Get_Struct((VALUE)gp, shoes_abstract, ab);
    VALUE obj = ATTR(ab->attr, name);
    if (NIL_P(obj))
      val = Qnil;
    else
      val = obj;
  }
  ct->target_object = val;

  target_attr = emeus_constraint_get_target_attribute(c);
  val = rb_str_new2(get_attribute_name(target_attr));
  ct->target_attr = val;

  relation = emeus_constraint_get_relation(c);
  val = rb_str_new2(get_relation_symbol(relation));
  ct->relation = val;
  
  if (target_attr != EMEUS_CONSTRAINT_ATTRIBUTE_INVALID) {
     p = emeus_constraint_get_source_object(c);
      shoes_abstract *ab;
      if (! GSHOES_IS_ELE(p))
        val = rb_str_new2("parent");
      else {
        gpointer gp = gshoes_ele_get_element(p);
        Data_Get_Struct((VALUE)gp, shoes_abstract, ab);
        VALUE obj = ATTR(ab->attr, name);
        if (NIL_P(obj))
          val = Qnil;
        else
          val = obj;
      }
    ct->source_object = val;

    source_attr = emeus_constraint_get_source_attribute(c);
    val = rb_str_new2(get_attribute_name(source_attr));
    ct->source_attr = val;
    
    double d = emeus_constraint_get_multiplier(c);
    ct->multiplier = DBL2NUM(d);
  }
  double d = emeus_constraint_get_constant(c);
  if (fabs (d - 1.0) > DBL_EPSILON) {
    val = DBL2NUM(d);
    ct->constant = val;
  }
  int strength = emeus_constraint_get_strength(c);
  val = INT2NUM(strength);
  //val = rb_str_new2(strength_to_string (strength));
  ct->strength = val;  

  return csobj;
  
}
#if 0
typedef struct {
  char *str;
  EmeusConstraintAttribute attr;
} attr_str_entry;

attr_str_entry attr_str_table[] = {
  {"", EMEUS_CONSTRAINT_ATTRIBUTE_INVALID}, 
  {"left", EMEUS_CONSTRAINT_ATTRIBUTE_LEFT},
  {"right", EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT},
  {"top", EMEUS_CONSTRAINT_ATTRIBUTE_TOP},
  {"bottom", EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM},
  {"start", EMEUS_CONSTRAINT_ATTRIBUTE_START},
  {"end", EMEUS_CONSTRAINT_ATTRIBUTE_END},
  {"width", EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH},
  {"height", EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT},
  {"centerX", EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X},
  {"cemterY", EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y},
  {"baseline", EMEUS_CONSTRAINT_ATTRIBUTE_BASELINE}
};

EmeusConstraintAttribute shoes_vfl_attr_from_str(char *attr) {
  for (int i = 0; i < 12; i++) {
    if (strcmp(attr, attr_str_table[i].str) == 0)
      return attr_str_table[i].attr;
  }
  return EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;
}
#endif
typedef struct {
  char *str;
  EmeusConstraintRelation rel;
} rel_str_entry;
rel_str_entry rel_str_table[] = {
  {"<=", EMEUS_CONSTRAINT_RELATION_LE},
  {"==", EMEUS_CONSTRAINT_RELATION_EQ},
  {">=", EMEUS_CONSTRAINT_RELATION_GE}
};
EmeusConstraintRelation shoes_vfl_rel_from_str(char *rel) {
  for (int i = 0; i < 3; i++) {
    if (strcmp(rel, rel_str_table[i].str) == 0)
      return rel_str_table[i].rel;
  }
  return EMEUS_CONSTRAINT_RELATION_EQ;
}
