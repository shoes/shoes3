/* emeus-constraint-layout.c: The constraint layout manager
 *
 * Copyright 2016  Endless
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */
 
 /* 
  * Modified for Shoes usage, November 2018 by Cecil Coupe. 
  * Removed builder interface.
  * Use Shoes instead of Gtk.
  */

/**
 * SECTION:emeus-constraint-layout
 * @Title: EmeusConstraintLayout
 * @Short_Description: A widget container using layout constraints
 *
 * #EmeusConstraintLayout is a #GtkContainter that uses constraints
 * associated to each of its children to decide their layout.
 *
 * # How do constraints work
 *
 * Constraints are objects defining the relationship between attributes
 * of a widget; you can read the description of the #EmeusConstraint
 * class to have a more in depth definition.
 *
 * By taking multiple constraints and applying them to the children of
 * an #EmeusConstraintLayout, it's possible to describe complex layout
 * policies; each constraint applied to a child or to the layout itself
 * contributes to the full description of the layout, in terms of
 * parameters for resolving the value of each attribute.
 *
 * It is important to note that a layout is defined by the totality of
 * constraints; removing a child, or a constraint, from an existing layout
 * without changing the remaining constraints may result in a layout that
 * cannot be solved.
 *
 * Constraints have an implicit "reading order"; you should start describing
 * each edge of each child, as well as their relationship with the parent
 * container, from the top left (or top right, in RTL languages), horizontally
 * first, and then vertically.
 *
 * A constraint-based layout with too few constraints can become "unstable",
 * that is: have more than one solution. The behavior of an unstable layout
 * is undefined.
 *
 * A constraint-based layout with conflicting constraints may be unsolvable,
 * and lead to an unstable layout.
 *
 * # Adding and removing widgets
 *
 * #EmeusConstraintLayout accepts only one type of widget as its direct
 * child, #EmeusConstraintLayoutChild. As a convenience for the developer,
 * #EmeusConstraintLayout will automatically add an #EmeusConstraintLayoutChild
 * for any other widget type when calling emeus_constraint_layout_pack() or
 * gtk_container_add().
 *
 * Similarly, when removing a #GtkWidget from an #EmeusConstraintLayout,
 * the container will remove the intermediate #EmeusConstraintLayoutChild
 * that was added automatically.
 *
 * When iterating over the children of #EmeusConstraintLayout, you will
 * get only #EmeusConstraintLayoutChild instances; use gtk_bin_get_child()
 * to retrieve the grandchild of the layout.
 *
 * # Describing constraints using GtkBuilder
 *
 * When creating an #EmeusConstraintLayout layout with GtkBuilder, it
 * can be simpler to also load constraints directly from the UI definition
 * file.
 *
 * #EmeusConstraintLayout implements the #GtkBuildable interface, and
 * adds the `constraints` tag, which contains `constraint` elements.
 *
 * The `constraint` element has various attributes:
 *
 *  - `target-object`, used to point to a widget using its builder id; this
 *    attribute is mandatory, and specifies the value of the
 *    #EmeusConstraint:target-object property
 *  - `target-attr`, which contains an %EmeusConstraintAttribute value; this
 *    attribute is mandatory, and specifies the value of the
 *    #EmeusConstraint:target-attribute property
 *  - `source-object`, used to point to a widget using its builder id; this
 *    attribute is optional, for constant constraints, and specifies the value
 *    of the #EmeusConstraint:source-object property
 *  - `source-attr`, which contains an %EmeusConstraintAttribute value; this
 *    attribute is optional, for constant constraints, and specifies the value
 *    of the #EmeusConstraint:source-attribute property
 *  - `relation`, which contains an %EmeusConstraintRelation value; this
 *    attribute is optional, and if not found the relation will be
 *    %EMEUS_CONSTRAINT_RELATION_EQ
 *  - `constant`, which contains the constant factor value as a floating point
 *    numer; this attribute is optional, and if not found the constant will
 *    be set to 0
 *  - `multiplier`, which contains the multiplication factor value as a
 *    floating point number; this attribute is option, and if not found the
 *    multiplier will be set to 1
 *  - `strength`, which contains an %EmeusConstraintStrength value, or a positive
 *   integer; this attribute is optional, and if not found the strength will
 *   be %EMEUS_CONSTRAINT_STRENGTH_REQUIRED
 *
 * A simple layout definitions is:
 *
 * |[<!-- language="plain" -->
 * <object class="EmeusConstraintLayout" id="layout">
 *   <property name="visible">True</property>
 *   <child>
 *     <object class="GtkButton" id="button1">
 *     ...
 *     </object>
 *   </child>
 *   <child>
 *     <object class="GtkButton" id="button2">
 *     ...
 *     </object>
 *   </child>
 *   <constraints>
 *     <constraint target-object="button1"
 *                 target-attr="width"
 *                 relation="EMEUS_CONSTRAINT_RELATION_GE"
 *                 constant="250"
 *                 strength="EMEUS_CONSTRAINT_STRENGTH_STRONG"/>
 *     <constraint target-object="button2"
 *                 target-attr="width"
 *                 relation="EMEUS_CONSTRAINT_RELATION_EQ"
 *                 source-object="button1"
 *                 source-attr="width"/>
 *     ...
 *   </constraints>
 * </object>
 * ]|
 *
 * # Describing constraints using the Visual Format Language
 *
 * While it's entirely possible to describe layouts by writing constraints
 * by hand, it can be tedious to do so. Additionally, writing constraints
 * by hand may lead to forgetting one or more of them, thus causing the
 * layout to be unstable or incomplete; or to add conflicting constraints,
 * thus causing the layout to be unsolvable.
 *
 * In order to quickly visualize, and generate, constraints for a layout,
 * Emeus implements a Visual Format language. Each row and column of a
 * layout can be described by the VFL, and translated into constraints to
 * be applied on top of a #EmeusConstraintLayout and its children. Once
 * the layout has been established, it's also possible to impose new
 * constraints for additional layout rules.
 *
 * A simple VFL description of a layout is:
 *
 * |[<!-- language="plain" -->
 *   |-[view0]-[view1(view0)]-|
 *   [view2(view1)]-|
 *   V:|-[view0]-|
 *   V:|-[view1]-[view2(view1)]-|
 * ]|
 *
 * The description will generate a layout composed by three widgets arrange
 * in two columns: one with 'view0', and the other with 'view1' and 'view2'.
 * The 'view1' and 'view2' widgets will have the same width and height, and
 * all three widgets will grow to fill the available space if their parent
 * grows.
 *
 * See emeus_create_constraints_from_description() for the VFL grammar and
 * additional examples of layouts.
 */

#include "config.h"
#include "shoes/layout/shoes-vfl.h"
#include "emeus-constraint-layout-private.h"

#include "emeus-constraint-private.h"
#include "emeus-types-private.h"
#include "emeus-expression-private.h"
#include "emeus-simplex-solver-private.h"
#include "emeus-utils-private.h"

#include <errno.h>
#include <math.h>
#include <string.h>

enum {
  CHILD_PROP_NAME = 1,
  CHILD_PROP_ELEMENT,
  CHILD_N_PROPS
};

static GParamSpec *emeus_constraint_layout_child_properties[CHILD_N_PROPS];

#if 0
static GtkBuildableIface *parent_buildable_iface;

static GQuark quark_buildable_constraints;

static void emeus_constraint_layout_buildable_iface_init (GtkBuildableIface *iface);

G_DEFINE_TYPE_WITH_CODE (EmeusConstraintLayout, emeus_constraint_layout, GTK_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, emeus_constraint_layout_buildable_iface_init))


G_DEFINE_TYPE (EmeusConstraintLayoutChild, emeus_constraint_layout_child, GTK_TYPE_BIN)
#else

G_DEFINE_TYPE (EmeusConstraintLayout, emeus_constraint_layout, G_TYPE_OBJECT);

G_DEFINE_TYPE (EmeusConstraintLayoutChild, emeus_constraint_layout_child, G_TYPE_OBJECT)
#endif 

#ifdef EMEUS_ENABLE_DEBUG
# define DEBUG(x)       x
#else
# define DEBUG(x)
#endif

static void
emeus_constraint_layout_finalize (GObject *gobject)
{
  EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (gobject);

  g_clear_pointer (&self->children, g_sequence_free);
  g_clear_pointer (&self->bound_attributes, g_hash_table_unref);
  g_clear_pointer (&self->constraints, g_hash_table_unref);

  simplex_solver_remove_constraint (&self->solver, self->stays.top);
  simplex_solver_remove_constraint (&self->solver, self->stays.left);
  simplex_solver_remove_constraint (&self->solver, self->stays.width);
  simplex_solver_remove_constraint (&self->solver, self->stays.height);

  simplex_solver_clear (&self->solver);

  G_OBJECT_CLASS (emeus_constraint_layout_parent_class)->finalize (gobject);
}

// TODO: cjc
static void
emeus_constraint_layout_destroy (GshoesEle *widget)
{
  EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (widget);

  simplex_solver_freeze (&self->solver);

  //GTK_WIDGET_CLASS (emeus_constraint_layout_parent_class)->destroy (widget);
}

Variable *
get_layout_attribute (EmeusConstraintLayout   *layout,
                      EmeusConstraintAttribute attr)
{
#if 0
  GtkTextDirection text_dir;

  /* Resolve the start/end attributes depending on the layout's text direction */
  if (attr == EMEUS_CONSTRAINT_ATTRIBUTE_START)
    {
      text_dir = gtk_widget_get_direction (GTK_WIDGET (layout));
      if (text_dir == GTK_TEXT_DIR_RTL)
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT;
      else
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_LEFT;
    }
  else if (attr == EMEUS_CONSTRAINT_ATTRIBUTE_END)
    {
      text_dir = gtk_widget_get_direction (GTK_WIDGET (layout));
      if (text_dir == GTK_TEXT_DIR_RTL)
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_LEFT;
      else
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT;
    }
#endif

  const char *attr_name = get_attribute_name (attr);
  Variable *res = g_hash_table_lookup (layout->bound_attributes, attr_name);
  if (res != NULL)
    return res;

  res = simplex_solver_create_variable (&layout->solver, attr_name, 0.0);
  variable_set_prefix (res, "super");

  g_hash_table_insert (layout->bound_attributes, (gpointer) attr_name, res);

  /* Some attributes are really constraints computed from other
   * attributes, to avoid creating additional constraints from
   * the user's perspective
   */
  switch (attr)
    {
    /* right = left + width */
    case EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT:
      {
        Variable *left = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
        Variable *width = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
        Expression *expr =
          expression_plus_variable (expression_new_from_variable (left), width);

        simplex_solver_add_constraint (&layout->solver,
                                       res, OPERATOR_TYPE_EQ, expr,
                                       STRENGTH_MEDIUM);

        expression_unref (expr);
      }
      break;

    /* bottom = top + height */
    case EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM:
      {
        Variable *top = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
        Variable *height = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
        Expression *expr =
          expression_plus_variable (expression_new_from_variable (top), height);

        simplex_solver_add_constraint (&layout->solver,
                                       res, OPERATOR_TYPE_EQ, expr,
                                       STRENGTH_MEDIUM);

        expression_unref (expr);
      }
      break;

    /* centerX = left + (width / 2.0) */
    case EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X:
      {
        Variable *left = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
        Variable *width = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
        Expression *expr =
          expression_plus_variable (expression_divide (expression_new_from_variable (width), 2.0), left);

        simplex_solver_add_constraint (&layout->solver,
                                       res, OPERATOR_TYPE_EQ, expr,
                                       STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    /* centerY = top + (height / 2.0) */
    case EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y:
      {
        Variable *top = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
        Variable *height = get_layout_attribute (layout, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
        Expression *expr =
          expression_plus_variable (expression_divide (expression_new_from_variable (height), 2.0), top);

        simplex_solver_add_constraint (&layout->solver,
                                       res, OPERATOR_TYPE_EQ, expr,
                                       STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    case EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH:
    case EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT:
      {
        Expression *expr = expression_new_from_constant (0.0);
        simplex_solver_add_constraint (&layout->solver,
                                       res, OPERATOR_TYPE_GE, expr,
                                       STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    default:
      break;
    }

  return res;
}

extern Variable *
get_child_attribute (EmeusConstraintLayoutChild *child,
                     EmeusConstraintAttribute    attr)
{
#if 0
  GtkTextDirection text_dir;

  /* Resolve the start/end attributes depending on the child's text direction */
  if (attr == EMEUS_CONSTRAINT_ATTRIBUTE_START)
    {
      text_dir = gtk_widget_get_direction (GTK_WIDGET (child));
      if (text_dir == GTK_TEXT_DIR_RTL)
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT;
      else
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_LEFT;
    }
  else if (attr == EMEUS_CONSTRAINT_ATTRIBUTE_END)
    {
      text_dir = gtk_widget_get_direction (GTK_WIDGET (child));
      if (text_dir == GTK_TEXT_DIR_RTL)
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_LEFT;
      else
        attr = EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT;
    }
#endif
  const char *attr_name = get_attribute_name (attr);
  Variable *res = g_hash_table_lookup (child->bound_attributes, attr_name);
  if (res != NULL)
    return res;

  res = simplex_solver_create_variable (child->solver, attr_name, 0.0);
  variable_set_prefix (res, child->name);

  g_hash_table_insert (child->bound_attributes, (gpointer) attr_name, res);

  /* Some attributes are really constraints computed from other
   * attributes, to avoid creating additional constraints from
   * the user's perspective
   */
  switch (attr)
    {
    /* right = left + width */
    case EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT:
      {
        Variable *left = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
        Variable *width = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
        Expression *expr =
          expression_plus_variable (expression_new_from_variable (left), width);

        child->right_constraint =
          simplex_solver_add_constraint (child->solver,
                                         res, OPERATOR_TYPE_EQ, expr,
                                         STRENGTH_MEDIUM);

        expression_unref (expr);
      }
      break;

    /* bottom = top + height */
    case EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM:
      {
        Variable *top = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
        Variable *height = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
        Expression *expr =
          expression_plus_variable (expression_new_from_variable (top), height);

        child->bottom_constraint =
          simplex_solver_add_constraint (child->solver,
                                         res, OPERATOR_TYPE_EQ, expr,
                                         STRENGTH_MEDIUM);

        expression_unref (expr);
      }
      break;

    /* centerX = left + (width / 2.0) */
    case EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X:
      {
        Variable *left = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);
        Variable *width = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
        Expression *expr =
          expression_plus_variable (expression_divide (expression_new_from_variable (width), 2.0), left);

        child->center_x_constraint =
          simplex_solver_add_constraint (child->solver,
                                         res, OPERATOR_TYPE_EQ, expr,
                                         STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    /* centerY = top + (height / 2.0) */
    case EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y:
      {
        Variable *top = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);
        Variable *height = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
        Expression *expr =
          expression_plus_variable (expression_divide (expression_new_from_variable (height), 2.0), top);

        child->center_y_constraint =
          simplex_solver_add_constraint (child->solver,
                                         res, OPERATOR_TYPE_EQ, expr,
                                         STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    case EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH:
    case EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT:
      {
        Expression *expr = expression_new_from_constant (0.0);
        simplex_solver_add_constraint (child->solver,
                                       res, OPERATOR_TYPE_GE, expr,
                                       STRENGTH_REQUIRED);

        expression_unref (expr);
      }
      break;

    default:
      break;
    }

  return res;
}

static void
add_layout_stays (EmeusConstraintLayout *self)
{
  Variable *var;

  /* Add two required stay constraints for the top left corner */
  var = simplex_solver_create_variable (&self->solver, "top", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_TOP),
                       var);
  self->stays.top =
    simplex_solver_add_stay_variable (&self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (&self->solver, "left", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_LEFT),
                       var);
  self->stays.left =
    simplex_solver_add_stay_variable (&self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (&self->solver, "width", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH),
                       var);
  self->stays.width =
    simplex_solver_add_stay_variable (&self->solver, var, STRENGTH_WEAK);

  var = simplex_solver_create_variable (&self->solver, "height", 0.0);
  variable_set_prefix (var, "super");
  g_hash_table_insert (self->bound_attributes,
                       (gpointer) get_attribute_name (EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT),
                       var);
  self->stays.height =
    simplex_solver_add_stay_variable (&self->solver, var, STRENGTH_WEAK);
}

/*
 *  TODO CJC: This may not need to be called by Shoes. But, if it was, 
 *  setup() phase would be the time to do it. 
 *  Gtk discussion: By adding the temp stay(s) the solver (re)computes
 *  layout(s) which is returned. Then the stays are removed. (which recomputes)
*/
#if 0
static void
emeus_constraint_layout_get_preferred_size (EmeusConstraintLayout *self,
                                            GtkOrientation         orientation,
                                            int                    for_size,
                                            int                   *minimum_p,
                                            int                   *natural_p)
{
  Variable *size = NULL;
  Variable *opposite_size = NULL;

  if (g_sequence_is_empty (self->children))
    {
      if (minimum_p != NULL)
        *minimum_p = 0;

      if (natural_p != NULL)
        *natural_p = 0;

      return;
    }

  switch (orientation)
    {
    case GTK_ORIENTATION_HORIZONTAL:
      size = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
      opposite_size = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
      break;

    case GTK_ORIENTATION_VERTICAL:
      size = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
      opposite_size = get_layout_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
      break;
    }

  g_assert (size != NULL);
  g_assert (opposite_size != NULL);

  /* We impose new temporary stay constraints on the size and its opposite,
   * with a low priority so that the solver will revert to the preferred
   * size of the layout for the duration of this function.
   *
   * The strength is (WEAK + 1) because it has to override the WEAK strength
   * stay constraints we add to the layout inside the instance initialization
   * function, whose job is to keep the layout origin and size greather than
   * or equal to zero. If we used the same priority, the solver would be in
   * an unstable state, and randomly fall back to a preferred size of 0.
   */
  variable_set_value (size, 0.0);

  Constraint *stay_s =
    simplex_solver_add_stay_variable (&self->solver, size, STRENGTH_WEAK + 1);

  variable_set_value (opposite_size, for_size > 0 ? for_size : 0.0);

  Constraint *stay_o =
    simplex_solver_add_stay_variable (&self->solver, opposite_size, STRENGTH_WEAK + 1);

  DEBUG (g_debug ("layout %p preferred %s size: %.3f (for opposite size: %d)",
                  self,
                  orientation == GTK_ORIENTATION_HORIZONTAL ? "horizontal" : "vertical",
                  variable_get_value (size),
                  for_size));

  double value = variable_get_value (size);

  simplex_solver_remove_constraint (&self->solver, stay_s);
  simplex_solver_remove_constraint (&self->solver, stay_o);

  if (minimum_p != NULL)
    *minimum_p = value;
  if (natural_p != NULL)
    *natural_p = value;
}
#endif
#if 0
static void
emeus_constraint_layout_get_preferred_width (GtkWidget *widget,
                                             int       *minimum_p,
                                             int       *natural_p)
{
  emeus_constraint_layout_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT (widget),
                                              GTK_ORIENTATION_HORIZONTAL,
                                              -1.0,
                                              minimum_p, natural_p);
}

static void
emeus_constraint_layout_get_preferred_height (GtkWidget *widget,
                                              int       *minimum_p,
                                              int       *natural_p)
{
  emeus_constraint_layout_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT (widget),
                                              GTK_ORIENTATION_VERTICAL,
                                              -1.0,
                                              minimum_p, natural_p);
}

static void
emeus_constraint_layout_get_preferred_width_for_height (GtkWidget *widget,
                                                        int        height,
                                                        int       *minimum_p,
                                                        int       *natural_p)
{
  emeus_constraint_layout_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT (widget),
                                              GTK_ORIENTATION_HORIZONTAL,
                                              height,
                                              minimum_p, natural_p);
}

static void
emeus_constraint_layout_get_preferred_height_for_width (GtkWidget *widget,
                                                        int        width,
                                                        int       *minimum_p,
                                                        int       *natural_p)
{
  emeus_constraint_layout_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT (widget),
                                              GTK_ORIENTATION_VERTICAL,
                                              width,
                                              minimum_p, natural_p);
}
#endif
/*
 * Shoes calls this at size() time with real values for height and width
 */
//static void
//emeus_constraint_layout_size_allocate (GtkWidget     *widget,
//                                       GtkAllocation *allocation)
void
emeus_constraint_layout_size_allocate (EmeusConstraintLayout *self, 
    int canvas_width,
    int canvas_height)
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
  DEBUG (g_debug ("layout [%p] = { .top:%g, .left:%g, .width:%g, .height:%g }",
                  self,
                  variable_get_value (layout_top),
                  variable_get_value (layout_left),
                  variable_get_value (layout_width),
                  variable_get_value (layout_height)));
#endif

  EmeusConstraintLayoutChild *child = NULL;
  GSequenceIter *iter = g_sequence_get_begin_iter (self->children);

  while (!g_sequence_iter_is_end (iter))
    {
      Variable *top, *left, *width, *height;
      Variable *center_x, *center_y;
      Variable *baseline;
      //GtkAllocation child_alloc;
      //GtkRequisition minimum;

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
      DEBUG (g_debug ("child '%s' [%p] = { "
                      ".top:%g, .left:%g, .width:%g, .height:%g, "
                      ".center:(%g, %g), .baseline:%g "
                      "}",
                      child->name != NULL ? child->name : "<unnamed>",
                      child,
                      variable_get_value (top),
                      variable_get_value (left),
                      variable_get_value (width),
                      variable_get_value (height),
                      variable_get_value (center_x), variable_get_value (center_y),
                      variable_get_value (baseline)));
#endif
#if 0
      gtk_widget_get_preferred_size (GTK_WIDGET (child), &minimum, NULL);

      child_alloc.x = floor (variable_get_value (left));
      child_alloc.y = floor (variable_get_value (top));
      child_alloc.width = variable_get_value (width) > minimum.width
                        ? ceil (variable_get_value (width))
                        : minimum.width;
      child_alloc.height = variable_get_value (height) > minimum.height
                         ? ceil (variable_get_value (height))
                         : minimum.height;

      gtk_widget_size_allocate (GTK_WIDGET (child), &child_alloc);
#else
      // shoes: 
      int x,y,wid,hgt;
      shoes_abstract *ab;
      VALUE abv = (VALUE)gshoes_ele_get_element(child->widget);
      ab = (shoes_abstract *)RTYPEDDATA_DATA(abv); 
      //variable_set_value(top, ab->place.y);
      //variable_set_value(left, ab->place.x);
      //variable_set_value(width, ab->place.w);
      //variable_set_value(height, ab->place.h);
      x = floor (variable_get_value (left));
      y = floor (variable_get_value (top));
      wid = variable_get_value (width) > ab->place.w
                        ? ceil (variable_get_value (width))
                        : ab->place.w;
      hgt = variable_get_value (height) > ab->place.h
                         ? ceil (variable_get_value (height))
                         : ab->place.h;
      // call into shoes-vfl.c to do the Shoes work
      shoes_vfl_change_pos(child->widget, x, y, wid, hgt);
#endif
    }

  simplex_solver_remove_constraint (&self->solver, stay_x);
  simplex_solver_remove_constraint (&self->solver, stay_y);
  simplex_solver_remove_constraint (&self->solver, stay_w);
  simplex_solver_remove_constraint (&self->solver, stay_h);
}

#if 0
static gboolean
emeus_constraint_layout_draw (GtkWidget *widget,
                              cairo_t   *cr)
{
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  int width = gtk_widget_get_allocated_width (widget);
  int height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_render_frame (context, cr, 0, 0, width, height);

  return GTK_WIDGET_CLASS (emeus_constraint_layout_parent_class)->draw (widget, cr);
}
#endif 

// TODO cjc
static void
emeus_constraint_layout_add (EmeusConstraintLayout *container,
                             GshoesEle    *widget)
{
  EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (container);

  emeus_constraint_layout_pack (self, widget, NULL, NULL);
}

// TODO cjc
static void
remove_constraints_from_widget (GHashTable *constraints,
                                GshoesEle  *widget)
{
  GHashTableIter iter;
  gpointer key;

  if (constraints == NULL || widget == NULL)
    return;

  g_hash_table_iter_init (&iter, constraints);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      EmeusConstraint *constraint = key;
      GshoesEle *real_target = constraint->target_object;

      if (real_target == NULL)
        continue;
#if 0 // cjc
      if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (real_target))
        real_target = gtk_bin_get_child (GTK_BIN (real_target));
#endif
      if (constraint->source_object == widget || real_target == widget)
        {
          emeus_constraint_detach (constraint);
          g_hash_table_iter_remove (&iter);
        }
    }
}

// TODO cjc
static void
emeus_constraint_layout_remove (EmeusConstraintLayout *container,
                                GshoesEle    *widget)
{
  EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (container);
  EmeusConstraintLayoutChild *layout_child;
  GshoesEle *child;
  GSequenceIter *iter;
#if 0
  if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (widget))
    {
      layout_child = EMEUS_CONSTRAINT_LAYOUT_CHILD (widget);
      child = gtk_bin_get_child (GTK_BIN (widget));
    }
  else
    {
      GtkWidget *parent = gtk_widget_get_parent (widget);

      if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (parent))
        {
          layout_child = EMEUS_CONSTRAINT_LAYOUT_CHILD (parent);
          child = widget;
        }
      else
        {
          g_critical ("Attempting to remove widget '%s' but "
                      "it's not a child of the layout",
                      G_OBJECT_TYPE_NAME (widget));
          return;
        }
    }
#else
  child = widget;
#endif
  // TODO: layout_child->iter
  layout_child = NULL;
  if (g_sequence_iter_get_sequence (layout_child->iter) != self->children)
    {
      g_critical ("Tried to remove non child %p", layout_child);
      return;
    }

  /* Remove layout constraints */
  remove_constraints_from_widget (self->constraints, child);

  /* Remove other children constraints */
  iter = g_sequence_get_begin_iter (self->children);
  while (!g_sequence_iter_is_end (iter))
    {
      EmeusConstraintLayoutChild *other = g_sequence_get (iter);
      remove_constraints_from_widget (other->constraints, child);
      iter = g_sequence_iter_next (iter);
    }
#if 0
  gboolean was_visible = gtk_widget_get_visible (GTK_WIDGET (layout_child));

  gtk_widget_unparent (GTK_WIDGET (layout_child));
#endif
  g_sequence_remove (layout_child->iter);
#if 0
  if (was_visible && gtk_widget_get_visible (GTK_WIDGET (container)))
    gtk_widget_queue_resize (GTK_WIDGET (container));
#endif
}

#if 0
// TODO cjc
static void
emeus_constraint_layout_forall (GtkContainer *container,
                                gboolean      internals,
                                GtkCallback   callback,
                                gpointer      data)
{
  EmeusConstraintLayout *self = EMEUS_CONSTRAINT_LAYOUT (container);

  if (g_sequence_is_empty (self->children))
    return;

  GtkWidget *child;
  GSequenceIter *iter = g_sequence_get_begin_iter (self->children);

  while (!g_sequence_iter_is_end (iter))
    {
      child = g_sequence_get (iter);
      iter = g_sequence_iter_next (iter);

      callback (child, data);
    }
}
#endif
static GType
emeus_constraint_layout_child_type (EmeusConstraintLayout *container)
{
  return EMEUS_TYPE_CONSTRAINT_LAYOUT_CHILD;
}

#define TAG_CONSTRAINTS "constraints"
#define TAG_CONSTRAINT  "constraint"

#define ATTR_SOURCE_OBJECT      "source-object"
#define ATTR_SOURCE_ATTR        "source-attr"
#define ATTR_TARGET_OBJECT      "target-object"
#define ATTR_TARGET_ATTR        "target-attr"
#define ATTR_RELATION           "relation"
#define ATTR_CONSTANT           "constant"
#define ATTR_MULTIPLIER         "multiplier"
#define ATTR_STRENGTH           "strength"

typedef struct {
  char *source_name;
  char *source_attr;
  char *target_name;
  char *target_attr;
  char *relation;
  char *strength;
  double constant;
  double multiplier;
} ConstraintData;

// CJC Gtk 3.22 dependency:
//extern void gtk_widget_class_set_css_name(GtkWidgetClass *widget_class,
//                               const char *name);
static void
emeus_constraint_layout_class_init (EmeusConstraintLayoutClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  //GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  //quark_buildable_constraints = g_quark_from_static_string ("-EmeusConstraintLayout-constraints");

  gobject_class->finalize = emeus_constraint_layout_finalize;

  //widget_class->get_preferred_width = emeus_constraint_layout_get_preferred_width;
  //widget_class->get_preferred_height = emeus_constraint_layout_get_preferred_height;
  //widget_class->get_preferred_width_for_height = emeus_constraint_layout_get_preferred_width_for_height;
  //widget_class->get_preferred_height_for_width = emeus_constraint_layout_get_preferred_height_for_width;
  //widget_class->size_allocate = emeus_constraint_layout_size_allocate;
  //widget_class->draw = emeus_constraint_layout_draw;
  //widget_class->destroy = emeus_constraint_layout_destroy;

  //container_class->add = emeus_constraint_layout_add;
  //container_class->remove = emeus_constraint_layout_remove;
  //container_class->forall = emeus_constraint_layout_forall;
  //container_class->child_type = emeus_constraint_layout_child_type;
  //gtk_container_class_handle_border_width (container_class);

  //gtk_widget_class_set_css_name (widget_class, "constraintlayout");
}

static void
emeus_constraint_layout_init (EmeusConstraintLayout *self)
{
  //gtk_widget_set_has_window (GTK_WIDGET (self), FALSE);

  simplex_solver_init (&self->solver);
  
  self->setup = false;

  self->children = g_sequence_new (NULL);

  self->bound_attributes = g_hash_table_new_full (NULL, NULL,
                                                  NULL,
                                                  (GDestroyNotify) variable_unref);

  self->constraints = g_hash_table_new_full (NULL, NULL,
                                             g_object_unref,
                                             NULL);

  add_layout_stays (self);
}

/**
 * emeus_constraint_layout_new:
 *
 * Creates a new constraint-based layout manager.
 *
 * Returns: (transfer full): the newly created layout widget
 *
 * Since: 1.0
 */
// TODO cjc was GtkWidget *
// Should use fancy Gobject parameters for init
EmeusConstraintLayout *
emeus_constraint_layout_new (shoes_layout *lay)
{
  EmeusConstraintLayout *layout;
  layout = g_object_new (EMEUS_TYPE_CONSTRAINT_LAYOUT, NULL);
  layout->sh_layout = lay;
  return layout;
  //return g_object_new (EMEUS_TYPE_CONSTRAINT_LAYOUT, NULL);
}

static void
create_layout_constraint (EmeusConstraintLayout *layout,
                          EmeusConstraint       *constraint)
{
  Variable *attr1, *attr2;
  Expression *expr;

  attr1 = get_layout_attribute (layout, constraint->target_attribute);
  if (constraint->source_attribute == EMEUS_CONSTRAINT_ATTRIBUTE_INVALID)
    {
      attr2 = simplex_solver_create_variable (constraint->solver, "const",
                                              emeus_constraint_get_constant (constraint));

      simplex_solver_add_stay_variable (constraint->solver, attr2, STRENGTH_REQUIRED);

      expr = expression_new_from_variable (attr2);

      constraint->constraint =
        simplex_solver_add_constraint (constraint->solver,
                                       attr1,
                                       relation_to_operator (constraint->relation),
                                       expr,
                                       strength_to_value (constraint->strength));
      expression_unref (expr);
      variable_unref (attr2);

      return;
    }

  if (constraint->source_object != NULL)
    {
      EmeusConstraintLayoutChild *source_child;

      if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (constraint->source_object))
        source_child = constraint->source_object;
#if 0
      else
        source_child = (EmeusConstraintLayoutChild *) gtk_widget_get_parent (constraint->source_object);
#endif
      attr2 = get_child_attribute (source_child, constraint->source_attribute);
    }
  else
    {
      attr2 = get_layout_attribute (layout, constraint->source_attribute);
    }

  expr =
    expression_plus (expression_times (expression_new_from_variable (attr2),
                                       constraint->multiplier),
                     constraint->constant);

  constraint->constraint =
    simplex_solver_add_constraint (constraint->solver,
                                   attr1,
                                   relation_to_operator (constraint->relation),
                                   expr,
                                   strength_to_value (constraint->strength));

  expression_unref (expr);
}

static void
add_layout_constraint (EmeusConstraintLayout *layout,
                       EmeusConstraint       *constraint)
{
  if (!emeus_constraint_attach (constraint, layout))
    return;

  g_hash_table_add (layout->constraints, g_object_ref_sink (constraint));

  if (constraint->is_active)
    create_layout_constraint (layout, constraint);
}

static void
create_child_constraint (EmeusConstraintLayout      *layout,
                         EmeusConstraintLayoutChild *child,
                         EmeusConstraint            *constraint)
{
  Variable *attr1, *attr2;
  Expression *expr;

  /* attr1 is the LHS of the linear equation */
  attr1 = get_child_attribute (child, constraint->target_attribute);

  /* attr2 is the RHS of the linear equation; if it's a constant value
   * we create a stay constraint for it. Stay constraints ensure that a
   * variable won't be modified by the solver.
   */
  if (constraint->source_attribute == EMEUS_CONSTRAINT_ATTRIBUTE_INVALID)
    {
      attr2 = simplex_solver_create_variable (constraint->solver, "const",
                                              emeus_constraint_get_constant (constraint));

      simplex_solver_add_stay_variable (child->solver, attr2, STRENGTH_REQUIRED);

      expr = expression_new_from_variable (attr2);

      constraint->constraint =
        simplex_solver_add_constraint (constraint->solver,
                                       attr1,
                                       relation_to_operator (constraint->relation),
                                       expr,
                                       strength_to_value (constraint->strength));

      expression_unref (expr);
      variable_unref (attr2);

      return;
    }

  /* Alternatively, if it's not a constant value, we find the variable
   * associated with it
   */
  if (constraint->source_object == NULL || constraint->source_object == layout)
    {
      attr2 = get_layout_attribute (layout, constraint->source_attribute);
    }
  else
    {
      EmeusConstraintLayoutChild *source_child;

      if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (constraint->source_object))
        source_child = constraint->source_object;
#if 0 //cjc
      else
        source_child = (EmeusConstraintLayoutChild *) gtk_widget_get_parent (constraint->source_object);

      if (gtk_widget_get_parent ((GtkWidget *) source_child) != (GtkWidget *) layout)
        {
          g_critical ("Source object %p of type %s does not belong to EmeusConstraintLayout %p",
                      constraint->source_object,
                      G_OBJECT_TYPE_NAME (constraint->source_object),
                      layout);
          return;
        }
#endif

      attr2 = get_child_attribute (source_child, constraint->source_attribute);
    }

  /* Turn attr2 into an expression in the form:
   *
   *   expr = attr2 * multiplier + constant
   */
  expr =
    expression_plus (expression_times (expression_new_from_variable (attr2),
                                       constraint->multiplier),
                     constraint->constant);

  constraint->constraint =
    simplex_solver_add_constraint (constraint->solver,
                                   attr1,
                                   relation_to_operator (constraint->relation),
                                   expr,
                                   strength_to_value (constraint->strength));

  expression_unref (expr);
}

static void
add_child_constraint (EmeusConstraintLayout      *layout,
                      EmeusConstraintLayoutChild *child,
                      EmeusConstraint            *constraint)
{
  if (emeus_constraint_is_attached (constraint))
    {
      const char *constraint_description = emeus_constraint_to_string (constraint);

      g_critical ("Constraint '%s' cannot be attached to more than "
                  "one child.",
                  constraint_description);

      return;
    }

  if (!emeus_constraint_attach (constraint, layout))
    return;

  g_hash_table_add (child->constraints, g_object_ref_sink (constraint));

  if (constraint->is_active)
    create_child_constraint (layout, child, constraint);
}

static gboolean
remove_child_constraint (EmeusConstraintLayout      *layout,
                         EmeusConstraintLayoutChild *child,
                         EmeusConstraint            *constraint)
{
#if 0 // cjc
  if (constraint->target_object != child)
    {
      GtkWidget *bin_child = gtk_bin_get_child (GTK_BIN (child));

      if (constraint->target_object != bin_child)
        {
          g_critical ("Attempting to remove unknown constraint %p", constraint);
          return FALSE;
        }
    }
#endif
  emeus_constraint_detach (constraint);

  g_hash_table_remove (child->constraints, constraint);

  return TRUE;
}

void
emeus_constraint_layout_activate_constraint (EmeusConstraintLayout *layout,
                                             EmeusConstraint       *constraint)
{
  g_assert (constraint->solver == &layout->solver);

  if (constraint->constraint != NULL)
    return;

  if (constraint->target_object == NULL)
    create_layout_constraint (layout, constraint);
  else
    {
      EmeusConstraintLayoutChild *child = NULL;

      if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (constraint->target_object))
        child = constraint->target_object;
#if 0 //cjc
      else
        {
          GtkWidget *parent = gtk_widget_get_parent (constraint->target_object);

          if (parent == NULL || !EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (parent))
            {
              g_critical ("Invalid target object for constraint");
              return;
            }

          child = EMEUS_CONSTRAINT_LAYOUT_CHILD (parent);
        }
#endif
      g_assert (child != NULL);
      create_child_constraint (layout, child, constraint);
    }
}

void
emeus_constraint_layout_deactivate_constraint (EmeusConstraintLayout *layout,
                                               EmeusConstraint       *constraint)
{
  g_assert (constraint->solver == &layout->solver);

  if (constraint->constraint == NULL)
    return;

  simplex_solver_remove_constraint (constraint->solver, constraint->constraint);
  constraint->constraint = NULL;
}

SimplexSolver *
emeus_constraint_layout_get_solver (EmeusConstraintLayout *layout)
{
  return &layout->solver;
}

gboolean
emeus_constraint_layout_has_child_data (EmeusConstraintLayout *layout,
                                        GshoesEle             *widget)
{
#if 0 // TODO cjc 
  GSequenceIter *iter;

  if (gtk_widget_get_parent (widget) == GTK_WIDGET (layout))
    return TRUE;

  iter = g_sequence_get_begin_iter (layout->children);
  while (!g_sequence_iter_is_end (iter))
    {
      GtkWidget *child = g_sequence_get (iter);

      iter = g_sequence_iter_next (iter);

      if (gtk_widget_get_parent (widget) == child)
        return TRUE;
    }
#endif
  return FALSE;
}

static void
layout_add_constraint (EmeusConstraintLayout *layout,
                       EmeusConstraint       *constraint)
{
  EmeusConstraintLayoutChild *child;
  GshoesEle *target, *parent;

  if (emeus_constraint_is_attached (constraint))
    {
      g_critical ("Constraint '%s' is already attached.",
                  emeus_constraint_to_string (constraint));
      return;
    }

  target = emeus_constraint_get_target_object (constraint);
  if (target == NULL)
    {
      add_layout_constraint (layout, constraint);
      return;
    }
#if 0
  parent = gtk_widget_get_parent (target);
  if (parent == NULL)
    {
      g_critical ("The target widget '%s' does not have any parent set",
                  G_OBJECT_TYPE_NAME (target));
      return;
    }

  if (!EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (target) &&
      !EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (parent))
    {
      g_critical ("The target widget '%s' is not a direct descendant of "
                  "the constraint layout",
                  G_OBJECT_TYPE_NAME (target));
      return;
    }
#endif
  // TODO: parent is not set
  parent = NULL;
  if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (target))
    child = EMEUS_CONSTRAINT_LAYOUT_CHILD (target);
  else
    child = EMEUS_CONSTRAINT_LAYOUT_CHILD (parent);

  add_child_constraint (layout, child, constraint);
}

/**
 * emeus_constraint_layout_add_constraint:
 * @layout: a #EmeusConstraintLayout
 * @constraint: a #EmeusConstraint
 *
 * Adds @constraint to the @layout.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_add_constraint (EmeusConstraintLayout *layout,
                                        EmeusConstraint       *constraint)
{
  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT (layout));
  g_return_if_fail (EMEUS_IS_CONSTRAINT (constraint));

  layout_add_constraint (layout, constraint);
}

/**
 * emeus_constraint_layout_add_constraints:
 * @layout: a #EmeusConstraintLayout
 * @first_constraint: the first #EmeusConstraint
 * @...: a %NULL-terminated list of #EmeusConstraint instances
 *
 * Adds multiple #EmeusConstraints at once to the @layout.
 *
 * See also: emeus_constraint_layout_add_constraints()
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_add_constraints (EmeusConstraintLayout *layout,
                                         EmeusConstraint       *first_constraint,
                                         ...)
{
  EmeusConstraint *constraint;
  va_list args;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT (layout));

  va_start (args, first_constraint);

  constraint = first_constraint;
  while (constraint != NULL)
    {
      layout_add_constraint (layout, constraint);

      constraint = va_arg (args, EmeusConstraint *);
    }

  va_end (args);
}

/**
 * emeus_constraint_layout_pack:
 * @layout: a #EmeusConstraintLayout
 * @child: a #GtkWidget
 * @name: (nullable): an optional name for the @child
 * @first_constraint: (nullable): a #EmeusConstraint
 * @...: a %NULL-terminated list of #EmeusConstraint instances
 *
 * Adds @child to the @layout, and applies a list of constraints to it.
 *
 * This convenience function is the equivalent of calling
 * gtk_container_add() and emeus_constraint_layout_child_add_constraint()
 * for each constraint instance.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_pack (EmeusConstraintLayout *layout,
                              GshoesEle             *child,
                              const char            *name,
                              EmeusConstraint       *first_constraint,
                              ...)
{
  EmeusConstraintLayoutChild *layout_child;
  EmeusConstraint *constraint;
  va_list args;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT (layout));
  g_return_if_fail (GSHOES_IS_ELE (child));
  g_return_if_fail (EMEUS_IS_CONSTRAINT (first_constraint) || first_constraint == NULL);

  //g_return_if_fail (gtk_widget_get_parent (child) == NULL);
  
#if 0
  if (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child))
    layout_child = EMEUS_CONSTRAINT_LAYOUT_CHILD (child);
  else
    {
      layout_child = (EmeusConstraintLayoutChild *) emeus_constraint_layout_child_new (name);

      gtk_widget_show (GTK_WIDGET (layout_child));
      gtk_container_add (GTK_CONTAINER (layout_child), child);
    }
#else
  // TODO cjc: arg should be GshoeEle (get name from there)
  layout_child = (EmeusConstraintLayoutChild *) emeus_constraint_layout_child_new (name, child);

  
#endif
  layout_child->iter = g_sequence_append (layout->children, layout_child);
  layout_child->solver = &layout->solver;
  g_object_add_weak_pointer (G_OBJECT (layout), (gpointer*) &layout_child->solver);

  //gtk_widget_set_parent (GTK_WIDGET (layout_child), GTK_WIDGET (layout));

  if (first_constraint == NULL)
    return;

  va_start (args, first_constraint);

  constraint = first_constraint;
  while (constraint != NULL)
    {
      add_child_constraint (layout, layout_child, constraint);

      constraint = va_arg (args, EmeusConstraint *);
    }

  va_end (args);
}

/**
 * emeus_constraint_layout_get_constraints:
 * @layout: a #EmeusConstraintLayout
 *
 * Retrieves all the constraints used by the @layout.
 *
 * Returns: (transfer container) (element-type EmeusConstraint): a list of
 *   #EmeusConstraint instances, owned by the #EmeusConstraintLayout
 *
 * Since: 1.0
 */
GList *
emeus_constraint_layout_get_constraints (EmeusConstraintLayout *layout)
{
  EmeusConstraintLayoutChild *child;
  GSequenceIter *iter;
  GList *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT (layout), NULL);

  /* First, add all the constraints attached to the layout */
  res = g_hash_table_get_keys (layout->constraints);

  /* Then, iterate over each child, and add the constraints attach to it */
  iter = g_sequence_get_begin_iter (layout->children);
  while (!g_sequence_iter_is_end (iter))
    {
      child = g_sequence_get (iter);
      iter = g_sequence_iter_next (iter);

      res = g_list_concat (res, g_hash_table_get_keys (child->constraints));
    }

  return res;
}

/**
 * emeus_constraint_layout_clear_constraints:
 * @layout: a #EmeusConstraintLayout
 *
 * Removes all constraints from a #EmeusConstraintLayout.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_clear_constraints (EmeusConstraintLayout *layout)
{
  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT (layout));

  GHashTableIter iter;
  gpointer key;

  g_hash_table_iter_init (&iter, layout->constraints);
  while (g_hash_table_iter_next (&iter, &key, NULL))
    {
      EmeusConstraint *constraint = key;

      emeus_constraint_detach (constraint);

      g_hash_table_iter_remove (&iter);
    }

  g_hash_table_remove_all (layout->bound_attributes);

  GSequenceIter *child_iter = g_sequence_get_begin_iter (layout->children);
  while (!g_sequence_iter_is_end (child_iter))
    {
      EmeusConstraintLayoutChild *child = g_sequence_get (child_iter);

      child_iter = g_sequence_iter_next (child_iter);

      emeus_constraint_layout_child_clear_constraints (child);
    }
}

static void
emeus_constraint_layout_child_finalize (GObject *gobject)
{
  EmeusConstraintLayoutChild *self = EMEUS_CONSTRAINT_LAYOUT_CHILD (gobject);

  if (self->solver)
    {
      simplex_solver_freeze (self->solver);

      if (self->width_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->width_constraint);

      if (self->height_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->height_constraint);

      if (self->right_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->right_constraint);

      if (self->bottom_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->bottom_constraint);

      if (self->center_x_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->center_x_constraint);

      if (self->center_y_constraint != NULL)
        simplex_solver_remove_constraint (self->solver, self->center_y_constraint);

      simplex_solver_thaw (self->solver);
#if 0
      GtkWidget *layout;
      if ((layout = gtk_widget_get_parent (GTK_WIDGET (gobject))))
        g_object_remove_weak_pointer (G_OBJECT (layout), (gpointer*) &self->solver);
#endif
      self->solver = NULL;
    }

  g_free (self->name);

  G_OBJECT_CLASS (emeus_constraint_layout_child_parent_class)->finalize (gobject);
}

static void
emeus_constraint_layout_child_dispose (GObject *gobject)
{
  EmeusConstraintLayoutChild *self = EMEUS_CONSTRAINT_LAYOUT_CHILD (gobject);

  g_clear_pointer (&self->constraints, g_hash_table_unref);
  g_clear_pointer (&self->bound_attributes, g_hash_table_unref);

  G_OBJECT_CLASS (emeus_constraint_layout_child_parent_class)->dispose (gobject);
}

static void
emeus_constraint_layout_child_set_property (GObject      *gobject,
                                            guint         prop_id,
                                            const GValue *value,
                                            GParamSpec   *pspec)
{
  EmeusConstraintLayoutChild *self = EMEUS_CONSTRAINT_LAYOUT_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_NAME:
      self->name = g_value_dup_string (value);
      break;
    case CHILD_PROP_ELEMENT:
      self->widget = g_value_get_pointer(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
emeus_constraint_layout_child_get_property (GObject    *gobject,
                                            guint       prop_id,
                                            GValue     *value,
                                            GParamSpec *pspec)
{
  EmeusConstraintLayoutChild *self = EMEUS_CONSTRAINT_LAYOUT_CHILD (gobject);

  switch (prop_id)
    {
    case CHILD_PROP_NAME:
      g_value_set_string (value, self->name);
      break;
    case CHILD_PROP_ELEMENT:
      g_value_set_pointer(value, (gpointer)self->widget);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}
#if 0 // TODO cjc - much to do?
static void
emeus_constraint_layout_child_get_preferred_size (EmeusConstraintLayoutChild *self,
                                                  GtkOrientation              orientation,
                                                  int                         for_size,
                                                  int                        *minimum_p,
                                                  int                        *natural_p)
{
  GtkWidget *child = gtk_bin_get_child (GTK_BIN (self));
  int child_min = 0;
  int child_nat = 0;
  Variable *attr = NULL;

  if (self->solver == NULL)
    return;

  switch (orientation)
    {
    case GTK_ORIENTATION_HORIZONTAL:
      attr = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);
      if (child != NULL && gtk_widget_get_visible (child))
        {

          if (for_size == -1.0)
            gtk_widget_get_preferred_width (child, &child_min, &child_nat);
          else
            gtk_widget_get_preferred_width_for_height (child, for_size, &child_min, &child_nat);

          /* Update the constraint because the min width can change */
          if (self->width_constraint != NULL)
            simplex_solver_remove_constraint (self->solver, self->width_constraint);

          Expression *e = expression_new_from_constant (child_min);

          self->width_constraint =
            simplex_solver_add_constraint (self->solver,
                                           attr, OPERATOR_TYPE_GE, e,
                                           STRENGTH_MEDIUM);

          expression_unref (e);
        }
      else
        {
          if (self->width_constraint != NULL)
            {
              simplex_solver_remove_constraint (self->solver, self->width_constraint);
              self->width_constraint = NULL;
            }
        }
      break;

    case GTK_ORIENTATION_VERTICAL:
      attr = get_child_attribute (self, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);
      if (child != NULL && gtk_widget_get_visible (child))
        {
          if (for_size == -1.0)
            gtk_widget_get_preferred_height (child, &child_min, &child_nat);
          else
            gtk_widget_get_preferred_height_for_width (child, for_size, &child_min, &child_nat);

          /* Update the constraint because the min height can change */
          if (self->height_constraint != NULL)
            simplex_solver_remove_constraint (self->solver, self->height_constraint);

          Expression *e = expression_new_from_constant (child_min);

          self->height_constraint =
            simplex_solver_add_constraint (self->solver,
                                           attr, OPERATOR_TYPE_GE, e,
                                           STRENGTH_MEDIUM);

          expression_unref (e);
        }
      else
        {
          if (self->height_constraint != NULL)
            {
              simplex_solver_remove_constraint (self->solver, self->height_constraint);
              self->height_constraint = NULL;
            }
        }
      break;
    }

  if (minimum_p != NULL)
    *minimum_p = child_min;

  if (natural_p != NULL)
    *natural_p = child_nat;
}

static void
emeus_constraint_layout_child_get_preferred_width (GtkWidget *widget,
                                                   int       *minimum_p,
                                                   int       *natural_p)
{
  emeus_constraint_layout_child_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT_CHILD (widget),
                                                    GTK_ORIENTATION_HORIZONTAL,
                                                    -1.0,
                                                    minimum_p,
                                                    natural_p);
}

static void
emeus_constraint_layout_child_get_preferred_height (GtkWidget *widget,
                                                    int       *minimum_p,
                                                    int       *natural_p)
{
  emeus_constraint_layout_child_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT_CHILD (widget),
                                                    GTK_ORIENTATION_VERTICAL,
                                                    -1.0,
                                                    minimum_p,
                                                    natural_p);
}

static void
emeus_constraint_layout_child_get_preferred_width_for_height (GtkWidget *widget,
                                                              int        height,
                                                              int       *minimum_p,
                                                              int       *natural_p)
{
  emeus_constraint_layout_child_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT_CHILD (widget),
                                                    GTK_ORIENTATION_HORIZONTAL,
                                                    height,
                                                    minimum_p,
                                                    natural_p);
}

static void
emeus_constraint_layout_child_get_preferred_height_for_width (GtkWidget *widget,
                                                              int        width,
                                                              int       *minimum_p,
                                                              int       *natural_p)
{
  emeus_constraint_layout_child_get_preferred_size (EMEUS_CONSTRAINT_LAYOUT_CHILD (widget),
                                                    GTK_ORIENTATION_VERTICAL,
                                                    width,
                                                    minimum_p,
                                                    natural_p);
}

static gboolean
emeus_constraint_layout_child_draw (GtkWidget *widget,
                                    cairo_t   *cr)
{
  GtkStyleContext *context = gtk_widget_get_style_context (widget);
  int width = gtk_widget_get_allocated_width (widget);
  int height = gtk_widget_get_allocated_height (widget);

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_render_frame (context, cr, 0, 0, width, height);

  return GTK_WIDGET_CLASS (emeus_constraint_layout_child_parent_class)->draw (widget, cr);
}
#endif 

static void
emeus_constraint_layout_child_class_init (EmeusConstraintLayoutChildClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  //GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  //GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  gobject_class->set_property = emeus_constraint_layout_child_set_property;
  gobject_class->get_property = emeus_constraint_layout_child_get_property;
  gobject_class->dispose = emeus_constraint_layout_child_dispose;
  gobject_class->finalize = emeus_constraint_layout_child_finalize;

  //widget_class->get_preferred_width = emeus_constraint_layout_child_get_preferred_width;
  //widget_class->get_preferred_height = emeus_constraint_layout_child_get_preferred_height;
  //widget_class->get_preferred_width_for_height = emeus_constraint_layout_child_get_preferred_width_for_height;
  //widget_class->get_preferred_height_for_width = emeus_constraint_layout_child_get_preferred_height_for_width;
  //widget_class->draw = emeus_constraint_layout_child_draw;

  //gtk_container_class_handle_border_width (container_class);

  /**
   * EmeusConstraintLayoutChild:name:
   *
   * The name of the child.
   *
   * Since: 1.0
   */
  emeus_constraint_layout_child_properties[CHILD_PROP_NAME] =
    g_param_spec_string ("name", "Name", "The name of the child",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);
                         
  emeus_constraint_layout_child_properties[CHILD_PROP_ELEMENT] =
    g_param_spec_pointer ("element", "Element", "The shoes element",
                          G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (gobject_class, CHILD_N_PROPS,
                                     emeus_constraint_layout_child_properties);
  // Gtk 3.22 tie up:
  //gtk_widget_class_set_css_name (widget_class, "constraintlayoutchild");
}

static void
emeus_constraint_layout_child_init (EmeusConstraintLayoutChild *self)
{
  //gtk_widget_set_redraw_on_allocate (GTK_WIDGET (self), TRUE);

  self->constraints = g_hash_table_new_full (NULL, NULL,
                                             g_object_unref,
                                             NULL);

  self->bound_attributes = g_hash_table_new_full (NULL, NULL,
                                                  NULL,
                                                  (GDestroyNotify) variable_unref);
}

/**
 * emeus_constraint_layout_child_new: (constructor)
 * @name: (nullable): a name for the child
 *
 * Creates a new #EmeusConstraintLayoutChild widget.
 *
 * Returns: (transfer full): the newly created #EmeusConstraintLayoutChild widget
 *
 * Since: 1.0
 */
EmeusConstraintLayoutChild *
emeus_constraint_layout_child_new (const char *name, GshoesEle *ele)
{
  return g_object_new (EMEUS_TYPE_CONSTRAINT_LAYOUT_CHILD,
                       "name", name,
                       "element", ele,
                       NULL);
}

/**
 * emeus_constraint_layout_child_add_constraint:
 * @child: a #EmeusConstraintLayoutChild
 * @constraint: a #EmeusConstraint
 *
 * Adds the given @constraint to the list of constraints applied to
 * the @child of a #EmeusConstraintLayout
 *
 * The #EmeusConstraintLayoutChild will own the @constraint until the
 * @child is removed, or until the @constraint is removed.
 *
 * Since: 1.0
 */
// TODO cjc -- Fix the layout variable - Child should have pointer to parent
void
emeus_constraint_layout_child_add_constraint (EmeusConstraintLayoutChild *child,
                                              EmeusConstraint            *constraint)
{
  EmeusConstraintLayout *layout;
  //GtkWidget *widget;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child));
  //g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (child)) != NULL);
  g_return_if_fail (EMEUS_IS_CONSTRAINT (constraint));

  g_return_if_fail (!emeus_constraint_is_attached (constraint));

  //widget = GTK_WIDGET (child);
  //layout = EMEUS_CONSTRAINT_LAYOUT (gtk_widget_get_parent (widget));
  // TODO: layout is not set properly
  layout = NULL;
  add_child_constraint (layout, child, constraint);

  //if (gtk_widget_get_visible (widget))
  //  gtk_widget_queue_resize (widget);
}

/**
 * emeus_constraint_layout_child_remove_constraint:
 * @child: a #EmeusConstraintLayoutChild
 * @constraint: a #EmeusConstraint
 *
 * Removes the given @constraint from the list of constraints applied
 * to the @child of a #EmeusConstraintLayout.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_child_remove_constraint (EmeusConstraintLayoutChild *child,
                                                 EmeusConstraint            *constraint)
{
  EmeusConstraintLayout *layout = NULL;
  //GtkWidget *widget;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child));
  g_return_if_fail (EMEUS_IS_CONSTRAINT (constraint));
  //g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (child)) != NULL);
  g_return_if_fail (!emeus_constraint_is_attached (constraint));

  //widget = GTK_WIDGET (child);
  //layout = EMEUS_CONSTRAINT_LAYOUT (gtk_widget_get_parent (widget));

  if (!remove_child_constraint (layout, child, constraint))
    return;

  //if (gtk_widget_get_visible (widget))
  //  gtk_widget_queue_resize (widget);
}

/**
 * emeus_constraint_layout_child_clear_constraints:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Clears all the constraints associated with a child of #EmeusConstraintLayout.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_child_clear_constraints (EmeusConstraintLayoutChild *child)
{
  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child));

  g_hash_table_remove_all (child->constraints);
  g_hash_table_remove_all (child->bound_attributes);

  //gtk_widget_queue_resize (GTK_WIDGET (child));
}

/**
 * emeus_constraint_layout_child_get_top:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_TOP for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_top (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_TOP);

  return floor (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_right:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_right (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_bottom:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_bottom (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_left:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_LEFT for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_left (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_LEFT);

  return floor (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_width:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_width (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_height:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_height (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_center_x:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_center_x (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_get_center_y:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the value of the %EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y for the @child.
 *
 * Returns: the value of the attribute
 *
 * Since: 1.0
 */
int
emeus_constraint_layout_child_get_center_y (EmeusConstraintLayoutChild *child)
{
  Variable *res;

  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), 0);

  res = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y);

  return ceil (variable_get_value (res));
}

/**
 * emeus_constraint_layout_child_set_intrinsic_width:
 * @child: a #EmeusConstraintLayoutChild
 * @width: the intrinsic width
 *
 * Creates a new constraint on the width of the @child.
 *
 * If @width is a negative value, the constraint is removed.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_child_set_intrinsic_width (EmeusConstraintLayoutChild *child,
                                                   int                         width)
{
  Variable *attr;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child));

  if (child->solver == NULL)
    return;

  if (child->intrinsic_width == width)
    return;

  attr = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH);

  if (child->intrinsic_width < 0)
    {
      child->width_constraint =
        simplex_solver_add_edit_variable (child->solver, attr, STRENGTH_REQUIRED);
    }

  if (width < 0)
    {
      simplex_solver_remove_constraint (child->solver, child->width_constraint);
      child->width_constraint = NULL;
    }

  child->intrinsic_width = width;

  if (child->intrinsic_width > 0)
    {
      simplex_solver_suggest_value (child->solver, attr, child->intrinsic_width);
      simplex_solver_resolve (child->solver);
    }

  //if (gtk_widget_get_visible (GTK_WIDGET (child)))
  //  gtk_widget_queue_resize (GTK_WIDGET (child));
}

/**
 * emeus_constraint_layout_child_set_intrinsic_height:
 * @child: a #EmeusConstraintLayoutChild
 * @height: the intrinsic width
 *
 * Creates a new constraint on the height of the @child.
 *
 * If @height is a negative value, the constraint is removed.
 *
 * Since: 1.0
 */
void
emeus_constraint_layout_child_set_intrinsic_height (EmeusConstraintLayoutChild *child,
                                                    int                         height)
{
  Variable *attr;

  g_return_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child));

  if (child->solver == NULL)
    return;

  if (child->intrinsic_height == height)
    return;

  attr = get_child_attribute (child, EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT);

  if (child->intrinsic_height < 0)
    {
      child->height_constraint =
        simplex_solver_add_edit_variable (child->solver, attr, STRENGTH_REQUIRED - 1);
    }

  if (height < 0)
    {
      simplex_solver_remove_constraint (child->solver, child->height_constraint);
      child->height_constraint = NULL;
    }

  child->intrinsic_height = height;

  if (child->intrinsic_height > 0)
    {
      simplex_solver_suggest_value (child->solver, attr, height);
      simplex_solver_resolve (child->solver);
    }

  //if (gtk_widget_get_visible (GTK_WIDGET (child)))
  //  gtk_widget_queue_resize (GTK_WIDGET (child));
}

/**
 * emeus_constraint_layout_child_get_name:
 * @child: a #EmeusConstraintLayoutChild
 *
 * Retrieves the name of the @child.
 *
 * Returns: (nullable) (transfer none): the name of the child
 *
 * Since: 1.0
 */
const char *
emeus_constraint_layout_child_get_name (EmeusConstraintLayoutChild *child)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT_LAYOUT_CHILD (child), NULL);

  return child->name;
}
