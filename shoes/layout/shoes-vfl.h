#ifndef SHOES_LAYOUTS_H
#define SHOES_LAYOUTS_H
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

// TODO: just to compile and link - temporary
#if 0
#define GtkWidget void 
#ifdef GTK_TYPE_WIDGET
#undef GTK_TYPE_WIDGET
#endif
#define GTK_TYPE_WIDGET NULL

#ifdef GTK_IS_WIDGET
#undef GTK_IS_WIDGET
#endif
#define GTK_IS_WIDGET(ptr) shoes_vfl_is_element(ptr)

#endif

#include "shoes/layout/emeus-expression-private.h"
#include "shoes/layout/emeus-simplex-solver-private.h"
#include "shoes/layout/emeus-utils-private.h"
#include "shoes/layout/emeus.h"
// These structs are mostly glib - not ruby
typedef struct {
  SimplexSolver *solver; 
  GHashTable *bound_attributes;
  GHashTable *constraints;  
} EmeusConstraintLayoutChild;

typedef struct {
	shoes_layout *shoes_contents;
  GHashTable *views, *metrics;
  GList *parsed_constraints;
  SimplexSolver *solver; 
  GSequence *children;
  GHashTable *bound_attributes;
  GHashTable *constraints;
  struct {
    Constraint *top, *left, *width, *height;
  } stays;
} EmeusConstraintLayout;

// in shoes-vfl-parser.c:
VALUE shoes_vfl_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE args);
// in shoes-vfl.c:
void shoes_vfl_setup(shoes_layout *lay, shoes_canvas *canvas, VALUE attr);
void shoes_vfl_add_ele(shoes_canvas *canvas, VALUE ele);
void shoes_vfl_delete_at(shoes_layout *lay, shoes_canvas *canvas, VALUE ele,
      int pos);
void shoes_vfl_clear(shoes_layout *lay, shoes_canvas *canvas);
void shoes_vfl_size(shoes_layout *lay, shoes_canvas *canvas, int pass); 
void shoes_vfl_finish(shoes_layout *lay, shoes_canvas *canvas);
void shoes_vfl_add_contraints(shoes_layout *lay, shoes_canvas *canvas, VALUE arg);
VALUE shoes_vfl_parse(shoes_layout *lay, shoes_canvas *canvas, VALUE arg);
VALUE shoes_vfl_get_constraints(shoes_layout *lay, shoes_canvas *canvas);
VALUE shoes_vfl_get_constraints_hash(shoes_layout *lay, shoes_canvas *canvas);
gboolean shoes_vfl_is_element(GshoesEle *ele);
// some emeus-contraint-layout functions we sort of emulate.
SimplexSolver * emeus_constraint_layout_get_solver(EmeusConstraintLayout *layout);
void  emeus_constraint_layout_activate_constraint (EmeusConstraintLayout *layout,
     EmeusConstraint *constraint);
void emeus_constraint_layout_deactivate_constraint(EmeusConstraintLayout *layout,
    EmeusConstraint *constraint);


#endif
