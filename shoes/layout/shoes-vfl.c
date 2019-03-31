/*
 *  Interface between Shoes (internal layout) and emeus code
 *  Emeus has A LOT of Gobject/Glib stuff which we keep. Some of the
 *  emeus code has been deleted (the gtk interface builder for example).
 *  
 * The shoes protocol for layouts is 'setup', add_ele..., finish and then
 * responding to size events.  The emeus-contraint-layout.c uses the much
 * more complicated Gtk process. We use the emeus names, macros and code
 * even when it's been modified for the Shoes/ruby way.  
 * 
 * We  use a GObject implementation of EmeusConstraintLayout without all the 
 * Gtk stuff. Same for EmeusContraintLayoutChild.  
 * 
 * We also have a a Ruby class CassowaryConstraint for mapping between
 * Ruby and EmeusConstraint (and VflConstraint)
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
#if 0
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
#endif

// --------- implement shoes usr layout protocol for vfl/emeus  ----------- 


void shoes_vfl_setup(shoes_layout *lay, shoes_canvas *canvas, VALUE attr) {
  fprintf(stderr, "shoes_vfl_setup called\n");
  // create OUR layout struct - different from the shoes_layout, sort of.
  EmeusConstraintLayout *layout;
  layout = emeus_constraint_layout_new(lay);
  lay->root = (void *)layout;
  
  // get height and width from attr
  VALUE hgtobj, widobj;
  int wid, hgt = 100;
  widobj = ATTR(attr, width);
  if (! NIL_P(widobj))
    wid = NUM2INT(widobj);
  hgtobj = ATTR(attr, height);
  if (! NIL_P(hgtobj))
    hgt = NUM2INT(hgtobj);
  
#if 0   
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
#endif
}

void shoes_vfl_add_ele(shoes_canvas *canvas, VALUE ele) {
#ifdef NEW_MACRO_LAYOUT
  Get_TypedStruct2(canvas->layout_mgr, shoes_layout, lay);
#else
  shoes_layout *lay;
  TypedData_Get_Struct(canvas->layout_mgr, shoes_layout, &shoes_layout_type,  lay);
#endif
  shoes_abstract *widget;
  widget = (shoes_abstract*)RTYPEDDATA_DATA(ele);
  VALUE name = ATTR(widget->attr, name);
  if (NIL_P(name)) 
    rb_raise(rb_eArgError, "please supply a name: ");
  char *str = RSTRING_PTR(name);
  rb_hash_aset(lay->views, name, ele);
#if 0
  /*
   *  create Variables/constraints for 'ele'. At this point in time
   *  we don't have w or h for the element 
  */
  GString *gstr = g_string_new(str);
  GshoesEle *gele = gshoes_ele_new(gstr, (gpointer)ele);
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  emeus_constraint_layout_pack (layout, gele, str, NULL, NULL);
#endif
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
  if (pass == 0) 
    return;
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  if (layout->setup) {
    fprintf(stderr, "shoes_vfl_size: recomputing\n");
    shoes_vfl_outer_size(layout, canvas->width, canvas->height);
  }
  return;
}
 

void shoes_vfl_finish(shoes_layout *lay, shoes_canvas *canvas) {
	fprintf(stderr,"shoes_vfl_finish called\n");
  /*
   *  create Variables/constraints for elements 
  */
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  VALUE keys;
  keys = rb_funcall(lay->views, rb_intern("keys"), 0);
  for (int i = 0; i < RARRAY_LEN(keys); i++) {
    VALUE ent = rb_ary_entry(keys, i); 
    GString *str = g_string_new(RSTRING_PTR(ent));
    // we want the Shoes ele to match the name (key)
    VALUE ele = rb_hash_aref(lay->views, ent);
    GshoesEle *gele = gshoes_ele_new(str, (gpointer)ele);    
    emeus_constraint_layout_pack (layout, gele, RSTRING_PTR(ent), NULL, NULL);
  }
  // Update the outer w/h constraints and stays, and the inners.
  shoes_vfl_outer_size(layout, canvas->width, canvas->height);
  // compute (may not be needed ?
  simplex_solver_resolve (&layout->solver); 
  // debug printout 
  layout->setup = true;
   
  
#if 0  // height,top,width,left - the stays.
  GHashTableIter iter;
  gpointer key, value;
  
  g_hash_table_iter_init (&iter, layout->constraints);
  while (g_hash_table_iter_next (&iter, &key, &value)) {
      /* do something with key and value */
      char *str = (char *)key;
      EmeusConstraint *cs = (EmeusConstraint *)value;
      char *as =  emeus_constraint_to_string(cs);
      fprintf(stderr, "stay cs: %s: %s \n", str, as);
  }
  
#endif 
}

GshoesEle * 
shoes_vfl_find_child(EmeusConstraintLayout *layout, char *name) {
  GSequenceIter *iter;
  GshoesEle *target = NULL;
  iter = g_sequence_get_begin_iter (layout->children);
  while (!g_sequence_iter_is_end (iter)) {
    EmeusConstraintLayoutChild *child;
    child = (EmeusConstraintLayoutChild *)g_sequence_get (iter);
    if (strcmp(child->name, name) == 0) {
      target = child->widget;
      break;
    }
  }
  return target;
}

void shoes_vfl_add_contraints(shoes_layout *lay, shoes_canvas *canvas, VALUE arg)
{
  fprintf(stderr, "shoes_vfl_add_contraints called\n");
  if (! rb_obj_is_kind_of(arg, cCassowaryconstraint))
    rb_raise(rb_eArgError, "arg must be a Shoes::Constraint object");
  // convert to emeus and add to solver.
  shoes_cassowary_constraint *cs;
  TypedData_Get_Struct(arg, shoes_cassowary_constraint, &shoes_cassowary_constraint_type, cs);
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
    
    // get the gshoes_ele from layout->children
    //target = g_hash_table_lookup(layout->views, name);
    target = shoes_vfl_find_child(layout, name);
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
      //source = g_hash_table_lookup(layout->views, name);
      source = shoes_vfl_find_child(layout, name);
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
  GHashTable *views, *metrics;
  //int hspacing = 10;
  //int vspacing = 10;
  EmeusConstraintLayout *layout = (EmeusConstraintLayout *)lay->root;
  views = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  metrics = g_hash_table_new_full (g_str_hash, g_str_equal, NULL, g_free);
  
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
    g_hash_table_insert(views, RSTRING_PTR(ent), gele);
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
   g_hash_table_insert(metrics, (char *)str, v);
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
      lines, n_lines, -1, -1, views, metrics);
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
  ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
  shoes_canvas *canvas;
  TypedData_Get_Struct(ab->parent, shoes_canvas, &shoes_canvas_type, canvas);
  if (NIL_P(canvas->layout_mgr))
    return false;
  return true;
}

#if 0
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
#endif

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
    ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
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
      ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
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
  TypedData_Get_Struct(csobj, shoes_cassowary_constraint, &shoes_cassowary_constraint_type, ct);
  VALUE val;
  gpointer p = emeus_constraint_get_target_object(c);
  shoes_abstract *ab;
  if (! GSHOES_IS_ELE(p))
    val = Qnil;
  else {
    gpointer gp = gshoes_ele_get_element(p);
    ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
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
        ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
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

void shoes_vfl_change_pos(GshoesEle *gs, int x, int y, int width, int height)
{
  shoes_abstract *ab;
  gpointer gp = gshoes_ele_get_element(gs);
  ab = (shoes_abstract*)RTYPEDDATA_DATA((VALUE)gp);
  //if (x != ab->place.x || y != ab->place.y) {
    fprintf(stderr, "move from %d,%d to %d,%d\n", ab->place.x, ab->place.y, x, y);
    fprintf(stderr, "size from %d, %d to %d %d\n", ab->place.w, ab->place.h, width, height);
  //}
}

// Move the shoes element's width,height to constraints 
// TODO: finish the 
void shoes_vfl_child_size(EmeusConstraintLayoutChild *self) {
  Variable *attr = NULL;
  Variable *top, *left, *width, *height;
  if (self->solver == NULL)
    return;
  shoes_abstract *ab;
  VALUE abv = (VALUE)gshoes_ele_get_element(self->widget);
  ab = (shoes_abstract*)RTYPEDDATA_DATA(abv);
  top = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
  left = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
  width = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
  height = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
  
  //variable_set_value(top, ab->place.y);
  //variable_set_value(left, ab->place.x);
  variable_set_value(width, ab->place.w);
  variable_set_value(height, ab->place.h);
  // width_constraint 
  attr = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
  // if (element is not hidden in Shoes:
  { 
    /* replace the constraint because the min width can change */
    if (self->width_constraint != NULL)
      simplex_solver_remove_constraint (self->solver, self->width_constraint);

    Expression *e = expression_new_from_constant (ab->place.w);

    self->width_constraint =
      simplex_solver_add_constraint (self->solver,
                                     attr, OPERATOR_TYPE_GE, e,
                                     STRENGTH_MEDIUM);
    expression_unref (e);
  }
  /*  else hidden
    if (self->width_constraint != NULL)
      {
        simplex_solver_remove_constraint (self->solver, self->width_constraint);
        self->width_constraint = NULL;
      }
  */
  // height_constraint
  attr = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
  // if (element is not hidden):
  {
    /* reeplace the constraint because the min height can change */
    if (self->height_constraint != NULL)
      simplex_solver_remove_constraint (self->solver, self->height_constraint);

    Expression *e = expression_new_from_constant (ab->place.h);

    self->height_constraint =
      simplex_solver_add_constraint (self->solver,
                                     attr, OPERATOR_TYPE_GE, e,
                                     STRENGTH_MEDIUM);
    expression_unref (e);
  }
  /* else hidden
    if (self->height_constraint != NULL)
      {
        simplex_solver_remove_constraint (self->solver, self->height_constraint);
        self->height_constraint = NULL;
      }
  */ 
}

void shoes_vfl_outer_size(EmeusConstraintLayout *self, int canvas_width, int canvas_height)
{
  //EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (widget);
  Constraint *stay_x, *stay_y;
  Constraint *stay_w, *stay_h;

  //gtk_widget_set_allocation (widget, allocation);

  if (g_sequence_is_empty (self->children))
    return;

  Variable *layout_top = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
  Variable *layout_left = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
  Variable *layout_width = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
  Variable *layout_height = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);

  //variable_set_value (layout_left, allocation->x);
  variable_set_value (layout_left, 0);
  stay_x = simplex_solver_add_stay_variable (&self->solver, layout_left, STRENGTH_REQUIRED);

  //variable_set_value (layout_top, allocation->y);
  variable_set_value (layout_top, 0);
  stay_y = simplex_solver_add_stay_variable (&self->solver, layout_top, STRENGTH_REQUIRED);

  //variable_set_value (layout_width, allocation->width);
  variable_set_value (layout_width, canvas_width);
  stay_w = simplex_solver_add_stay_variable (&self->solver, layout_width, STRENGTH_REQUIRED);

  //variable_set_value (layout_height, allocation->height);
  variable_set_value (layout_height, canvas_height);
  stay_h = simplex_solver_add_stay_variable (&self->solver, layout_height, STRENGTH_REQUIRED);

#ifdef EMEUS_ENABLE_DEBUG
  printf("layout [%p] = { .top:%g, .left:%g, .width:%g, .height:%g }\n",
                  self,
                  variable_get_value (layout_top),
                  variable_get_value (layout_left),
                  variable_get_value (layout_width),
                  variable_get_value (layout_height));
#endif
  // iterate thru the children
  // Set the /h/w and constraints for the elements 
  EmeusConstraintLayoutChild *child = NULL;
  GSequenceIter *iter = g_sequence_get_begin_iter (self->children);
  while (!g_sequence_iter_is_end (iter)) {      
      child = g_sequence_get (iter);
      shoes_vfl_child_size(child);
      iter = g_sequence_iter_next (iter);
  }
  // loop again to copy computed positions to Shoes
  iter = g_sequence_get_begin_iter (self->children);
  while (!g_sequence_iter_is_end (iter)) {      
      Variable *top, *left, *width, *height;
      Variable *center_x, *center_y;
      Variable *baseline;
      int x,y,wid,hgt;
      
      child = g_sequence_get (iter);
      iter = g_sequence_iter_next (iter);
      
      top = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
      left = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
      width = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
      height = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
      center_x = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X);
      center_y = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y);
      baseline = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_BASELINE);
#ifdef EMEUS_ENABLE_DEBUG
     printf ("child '%s' [%p] = { "
                      ".top:%g, .left:%g, .width:%g, .height:%g, "
                      ".center:(%g, %g), .baseline:%g "
                      "}\n",
                      child->name != NULL ? child->name : "<unnamed>",
                      child,
                      variable_get_value (top),
                      variable_get_value (left),
                      variable_get_value (width),
                      variable_get_value (height),
                      variable_get_value (center_x),
                      variable_get_value (center_y),
                      variable_get_value (baseline));
#endif

      x = floor (variable_get_value (left));
      y = floor (variable_get_value (top));
      wid = ceil (variable_get_value (width));
      hgt = ceil (variable_get_value (height));
      shoes_vfl_change_pos(child->widget, x, y, wid, hgt);
  }
  // remove the Required Stays (leaving the weak stays from setup)
  // -- until we resize again
  simplex_solver_remove_constraint (&self->solver, stay_x);
  simplex_solver_remove_constraint (&self->solver, stay_y);
  simplex_solver_remove_constraint (&self->solver, stay_w);
  simplex_solver_remove_constraint (&self->solver, stay_h);
}

