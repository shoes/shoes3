/*
 *  Interface between Shoes (internal layout) and emeus cassowary solver.
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
#include "shoes/layout/layouts.h"

#include "shoes/layout/emeus-expression-private.h"
#include "shoes/layout/emeus-simplex-solver-private.h"
#include "shoes/layout/emeus-constraint-private.h"
#include "shoes/layout/emeus-utils-private.h"

typedef struct {
  SimplexSolver *solver;
  GHashTable *bound_attributes;
  GHashTable *constraints;
  struct {
    Constraint *top, *left, *width, *height;
  } stays;
} shoes_vfl_layout;

extern ID s_name;

shoes_vfl_layout *vfl_layout;   // TODO: this should be in a shoes_layout?

// forward declares in this file 
void shoes_vfl_add_layout_stays (shoes_vfl_layout *self);


// Temporary - just enough to link on non gtk platforms. 

SimplexSolver * emeus_constraint_layout_get_solver(EmeusConstraintLayout *layout)
{
  return NULL;
}

void  emeus_constraint_layout_activate_constraint (EmeusConstraintLayout *layout,
     EmeusConstraint       *constraint) {
}

void emeus_constraint_layout_deactivate_constraint(EmeusConstraintLayout *layout,
    EmeusConstraint       *constraint) {
}

// --------- implement shoes usr layout protocol for vfl/emeus  ----------- 


void shoes_vfl_setup(shoes_layout *lay, shoes_canvas *canvas, VALUE attr) {
  fprintf(stderr, "shoes_vfl_setup called\n");
  // create OUR layout struct - different from the shoes_layout, sort of.
  vfl_layout = malloc(sizeof(shoes_vfl_layout));
  // make a pointer to the solver (aka context, tableau)
  SimplexSolver *solver = malloc(sizeof (SimplexSolver));
  solver->initialized = 0;
  simplex_solver_init (solver);
  vfl_layout->solver = solver;
  
  // get height and width from attr
  VALUE hgtobj, widobj;
  int wid, hgt = 0;
  widobj = ATTR(attr, width);
  if (! NIL_P(widobj))
    wid = NUM2INT(widobj);
  hgtobj = ATTR(attr, height);
  if (! NIL_P(hgtobj))
    hgt = NUM2INT(hgtobj);
    
  // Create Variables and stays for the Layout's canvas.
  vfl_layout->bound_attributes = g_hash_table_new_full (NULL, NULL,
                                                  NULL,
                                                  (GDestroyNotify) variable_unref);
  vfl_layout->constraints = g_hash_table_new_full (NULL, NULL,
                                             g_object_unref,
                                             NULL);

  shoes_vfl_add_layout_stays (vfl_layout);
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
}

void shoes_vfl_add_contraints(shoes_layout *lay, shoes_canvas *canvas, VALUE arg)
{
  fprintf(stderr, "shoes_vfl_add_contraints called\n");
}

VALUE shoes_vfl_parse(shoes_layout *lay, shoes_canvas *canvas, VALUE arg)
{
  fprintf(stderr, "shoes_vfl_parse called\n");
}

VALUE shoes_vfl_get_constraints(shoes_layout *lay, shoes_canvas *canvas)
{
  fprintf(stderr, "shoes_vfl_get_constraints called\n");
  return Qnil;
}
// -------------- internels - no Ruby api into these functions ---------

void shoes_vfl_add_layout_stays(shoes_vfl_layout *self) {
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
