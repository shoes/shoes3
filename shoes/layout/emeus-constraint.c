/* emeus-constraint.c: The base constraint object
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

/**
 * SECTION:emeus-constraint
 * @Title: EmeusConstraint
 * @Short_Description: The representation of a single constraint
 *
 * #EmeusConstraint is a type that describes a constraint between two widget
 * attributes which must be satisfied by a #GtkConstraintLayout.
 *
 * Each constraint is a linear equation in the form:
 *
 * |[<!-- language="C" -->
 *   target.attribute1 = source.attribute2 × multiplier + constant
 * ]|
 *
 * The #EmeusConstraintLayout will take all the constraints associated to
 * each of its children and solve the value of `attribute1` that satisfy the
 * system of equations.
 *
 * For instance, if a #EmeusConstraintLayout has two children, `button1` and
 * `button2`, and you wish for `button2` to follow `button1` with 8 pixels
 * of spacing between the two, using the direction of the text, you can
 * express this relationship as this constraint:
 *
 * |[<!-- language="C" -->
 *   button2.start = button1.end × 1.0 + 8.0
 * ]|
 *
 * If you also wish `button1` to have a minimum width of 120 pixels, and
 * `button2` to have the same size, you can use the following constraints:
 *
 * |[<!-- language="C" -->
 *   button1.width ≥ 120.0
 *   button2.width = button1.width × 1.0 + 0.0
 * ]|
 *
 * Each #EmeusConstraint instance references a target attribute; the target
 * object to which the attribute applies is a child of a #EmeusConstraintLayout,
 * and it's associated with the #EmeusConstraint instance once the constraint
 * is added to a child #GtkWidget of the layout.
 */

#include "config.h"
#include "shoes/layout/layouts.h"
#include "emeus-constraint-private.h"

#include "emeus-expression-private.h"
#include "emeus-simplex-solver-private.h"
#include "emeus-utils-private.h"
#include "emeus-utils.h"
#include "emeus-vfl-parser-private.h"
#include <math.h>
#include <float.h>
//#include <gtk/gtk.h>

enum {
  PROP_TARGET_OBJECT = 1,
  PROP_TARGET_ATTRIBUTE,
  PROP_RELATION,
  PROP_SOURCE_OBJECT,
  PROP_SOURCE_ATTRIBUTE,
  PROP_MULTIPLIER,
  PROP_CONSTANT,
  PROP_STRENGTH,
  PROP_ACTIVE,

  N_PROPERTIES
};

static GParamSpec *emeus_constraint_properties[N_PROPERTIES];

G_DEFINE_TYPE (EmeusConstraint, emeus_constraint, G_TYPE_INITIALLY_UNOWNED)

static void
emeus_constraint_finalize (GObject *gobject)
{
  EmeusConstraint *self = EMEUS_CONSTRAINT (gobject);

  g_free (self->description);

  G_OBJECT_CLASS (emeus_constraint_parent_class)->finalize (gobject);
}

static void
emeus_constraint_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  EmeusConstraint *self = EMEUS_CONSTRAINT (gobject);

  switch (prop_id)
    {
    case PROP_TARGET_OBJECT:
      self->target_object = g_value_get_object (value);
      break;

    case PROP_TARGET_ATTRIBUTE:
      self->target_attribute = g_value_get_enum (value);
      break;

    case PROP_RELATION:
      self->relation = g_value_get_enum (value);
      break;

    case PROP_SOURCE_OBJECT:
      self->source_object = g_value_get_object (value);
      break;

    case PROP_SOURCE_ATTRIBUTE:
      self->source_attribute = g_value_get_enum (value);
      break;

    case PROP_MULTIPLIER:
      self->multiplier = g_value_get_double (value);
      break;

    case PROP_CONSTANT:
      self->constant = g_value_get_double (value);
      break;

    case PROP_STRENGTH:
      self->strength = g_value_get_int (value);
      break;

    case PROP_ACTIVE:
      emeus_constraint_set_active (self, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
emeus_constraint_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  EmeusConstraint *self = EMEUS_CONSTRAINT (gobject);

  switch (prop_id)
    {
    case PROP_TARGET_OBJECT:
      g_value_set_object (value, self->target_object);
      break;

    case PROP_TARGET_ATTRIBUTE:
      g_value_set_enum (value, self->target_attribute);
      break;

    case PROP_RELATION:
      g_value_set_enum (value, self->relation);
      break;

    case PROP_SOURCE_OBJECT:
      g_value_set_object (value, self->source_object);
      break;

    case PROP_SOURCE_ATTRIBUTE:
      g_value_set_enum (value, self->source_attribute);
      break;

    case PROP_MULTIPLIER:
      g_value_set_double (value, self->multiplier);
      break;

    case PROP_CONSTANT:
      g_value_set_double (value, self->constant);
      break;

    case PROP_STRENGTH:
      g_value_set_int (value, self->strength);
      break;

    case PROP_ACTIVE:
      g_value_set_boolean (value, self->is_active);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
emeus_constraint_class_init (EmeusConstraintClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->set_property = emeus_constraint_set_property;
  gobject_class->get_property = emeus_constraint_get_property;
  gobject_class->finalize = emeus_constraint_finalize;

  /**
   * EmeusConstraint:target-object:
   *
   * The target object in the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_TARGET_OBJECT] =
    g_param_spec_object ("target-object", "Target Object", NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:target-attribute:
   *
   * The attribute on the #EmeusConstraint:target-object in
   * the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_TARGET_ATTRIBUTE] =
    g_param_spec_enum ("target-attribute", "Target Attribute", NULL,
                       EMEUS_TYPE_CONSTRAINT_ATTRIBUTE,
                       EMEUS_CONSTRAINT_ATTRIBUTE_INVALID,
                       G_PARAM_CONSTRUCT_ONLY |
                       G_PARAM_READWRITE |
                       G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:relation:
   *
   * The relation between target and source attributes in
   * the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_RELATION] =
    g_param_spec_enum ("relation", "Relation", NULL,
                       EMEUS_TYPE_CONSTRAINT_RELATION,
                       EMEUS_CONSTRAINT_RELATION_EQ,
                       G_PARAM_CONSTRUCT_ONLY |
                       G_PARAM_READWRITE |
                       G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:source-object:
   *
   * The source object in the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_SOURCE_OBJECT] =
    g_param_spec_object ("source-object", "Source Object", NULL,
                         GTK_TYPE_WIDGET,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:source-attribute:
   *
   * The attribute on the #EmeusConstraint:source-object in the
   * constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_SOURCE_ATTRIBUTE] =
    g_param_spec_enum ("source-attribute", "Source Attribute", NULL,
                       EMEUS_TYPE_CONSTRAINT_ATTRIBUTE,
                       EMEUS_CONSTRAINT_ATTRIBUTE_INVALID,
                       G_PARAM_CONSTRUCT_ONLY |
                       G_PARAM_READWRITE |
                       G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:multiplier:
   *
   * The multiplication factor to be applied to the value of
   * the #EmeusConstraint:source-attribute in the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_MULTIPLIER] =
    g_param_spec_double ("multiplier", "Multiplier", NULL,
                         -G_MAXDOUBLE, G_MAXDOUBLE,
                         1.0,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:constant:
   *
   * The constant factor to be applied to the value of
   * the #EmeusConstraint:source-attribute in the constraint.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_CONSTANT] =
    g_param_spec_double ("constant", "Constant", NULL,
                         -G_MAXDOUBLE, G_MAXDOUBLE,
                         0.0,
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:strength:
   *
   * The strength, or priority of the constraint.
   *
   * You can use any positive integer for custom priorities, or use
   * the values of the #EmeusConstraintStrength enumeration for common
   * strength values.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_STRENGTH] =
    g_param_spec_int ("strength", "Strength", NULL,
                      G_MININT, G_MAXINT,
                      EMEUS_CONSTRAINT_STRENGTH_REQUIRED,
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_READWRITE |
                      G_PARAM_STATIC_STRINGS);

  /**
   * EmeusConstraint:active:
   *
   * Whether a #EmeusConstraint participates in the layout.
   *
   * Since: 1.0
   */
  emeus_constraint_properties[PROP_ACTIVE] =
    g_param_spec_boolean ("active", "Active", NULL,
                          TRUE,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_install_properties (gobject_class, N_PROPERTIES, emeus_constraint_properties);
}

static void
emeus_constraint_init (EmeusConstraint *self)
{
  self->target_attribute = EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;
  self->relation = EMEUS_CONSTRAINT_RELATION_EQ;
  self->source_attribute = EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;
  self->multiplier = 1.0;
  self->constant = 0.0;
  self->strength = EMEUS_CONSTRAINT_STRENGTH_REQUIRED;
  self->is_active = TRUE;
}

/**
 * emeus_constraint_new: (constructor)
 * @target_object: (type Gtk.Widget) (nullable): the target widget, or
 *   %NULL for the parent layout
 * @target_attribute: the attribute to set on the target widget
 * @relation: the relation between the target and source attributes
 * @source_object: (type Gtk.Widget) (nullable): the source widget, or
 *   %NULL for the parent layout
 * @source_attribute: the attribute to get from the source widget
 * @multiplier: the multiplication coefficient to apply to the source
 *   attribute
 * @constant: the constant to add to the source attribute
 * @strength: the priority of the constraint
 *
 * Creates a new constraint using a value from the source widget's attribute
 * and applying it to the target widget's attribute.
 *
 * Returns: (transfer full): the newly created constraint
 *
 * Since: 1.0
 */
EmeusConstraint *
emeus_constraint_new (gpointer                 target_object,
                      EmeusConstraintAttribute target_attribute,
                      EmeusConstraintRelation  relation,
                      gpointer                 source_object,
                      EmeusConstraintAttribute source_attribute,
                      double                   multiplier,
                      double                   constant,
                      int                      strength)
{
  g_return_val_if_fail (target_object == NULL || GTK_IS_WIDGET (target_object), NULL);
  g_return_val_if_fail (source_object == NULL || GTK_IS_WIDGET (source_object), NULL);

  return g_object_new (EMEUS_TYPE_CONSTRAINT,
                       "target-object", target_object,
                       "target-attribute", target_attribute,
                       "relation", relation,
                       "source-object", source_object,
                       "source-attribute", source_attribute,
                       "multiplier", multiplier,
                       "constant", constant,
                       "strength", strength,
                       NULL);
}

/**
 * emeus_constraint_new_constant: (constructor)
 * @target_object: (type Gtk.Widget) (nullable): the target widget,
 *   or %NULL for the parent layout
 * @target_attribute: the attribute to set on the target widget
 * @relation: the relation between the target and the constant
 * @constant: the constant value of the constraint
 * @strength: the priority of the constraint
 *
 * Creates a new constant constraint.
 *
 * This function is the equivalent of creating a new #EmeusConstraint with:
 *
 *  - #EmeusConstraint:source_object set to %NULL
 *  - #EmeusConstraint:source_attribute set to %EMEUS_CONSTRAINT_ATTRIBUTE_INVALID
 *  - #EmeusConstraint:multiplier set to 1.0
 *
 * Returns: (transfer full): the newly created constraint
 *
 * Since: 1.0
 */
EmeusConstraint *
emeus_constraint_new_constant (gpointer                 target_object,
                               EmeusConstraintAttribute target_attribute,
                               EmeusConstraintRelation  relation,
                               double                   constant,
                               int                      strength)
{
  g_return_val_if_fail (target_object == NULL || GTK_IS_WIDGET (target_object), NULL);

  return g_object_new (EMEUS_TYPE_CONSTRAINT,
                       "target-object", target_object,
                       "target-attribute", target_attribute,
                       "relation", relation,
                       "source-object", NULL,
                       "source-attribute", EMEUS_CONSTRAINT_ATTRIBUTE_INVALID,
                       "multiplier", 1.0,
                       "constant", constant,
                       "strength", strength,
                       NULL);
}

/**
 * emeus_constraint_get_target_object:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the target object of the constraint.
 *
 * This function may return %NULL if the @constraint is not attached
 * to a #EmeusConstraintLayout. Use emeus_constraint_is_attached() to
 * before calling this function.
 *
 * Returns: (transfer none) (type Gtk.Widget) (nullable): the target
 *   object
 *
 * Since: 1.0
 */
gpointer
emeus_constraint_get_target_object (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), NULL);

  return constraint->target_object;
}

/**
 * emeus_constraint_get_target_attribute:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the attribute of the target object bound by this @constraint.
 *
 * Returns: a constraint attribute
 *
 * Since: 1.0
 */
EmeusConstraintAttribute
emeus_constraint_get_target_attribute (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), EMEUS_CONSTRAINT_ATTRIBUTE_INVALID);

  return constraint->target_attribute;
}

/**
 * emeus_constraint_get_relation:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the relation between the target and source attributes.
 *
 * Returns: a constraint relation
 *
 * Since: 1.0
 */
EmeusConstraintRelation
emeus_constraint_get_relation (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), EMEUS_CONSTRAINT_RELATION_EQ);

  return constraint->relation;
}

/**
 * emeus_constraint_get_source_object:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the source object of the @constraint.
 *
 * Returns: (transfer none) (nullable) (type Gtk.Widget): the source object
 */
gpointer
emeus_constraint_get_source_object (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), NULL);

  return constraint->source_object;
}

/**
 * emeus_constraint_get_source_attribute:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the attribute of the source object bound by this @constraint.
 *
 * Returns: a constraint attribute
 *
 * Since: 1.0
 */
EmeusConstraintAttribute
emeus_constraint_get_source_attribute (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), EMEUS_CONSTRAINT_ATTRIBUTE_INVALID);

  return constraint->source_attribute;
}

/**
 * emeus_constraint_get_multiplier:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the multiplication factor of the @constraint.
 *
 * Returns: a factor
 *
 * Since: 1.0
 */
double
emeus_constraint_get_multiplier (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), 1.0);

  return constraint->multiplier;
}

/**
 * emeus_constraint_get_constant:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the additional constant of the @constraint.
 *
 * Returns: a constant
 *
 * Since: 1.0
 */
double
emeus_constraint_get_constant (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), 0.0);

  return constraint->constant;
}

/**
 * emeus_constraint_get_strength:
 * @constraint: a #EmeusConstraint
 *
 * Retrieves the strength of the @constraint.
 *
 * Returns: a constraint strength
 *
 * Since: 1.0
 */
int
emeus_constraint_get_strength (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), EMEUS_CONSTRAINT_STRENGTH_REQUIRED);

  return constraint->strength;
}

/**
 * emeus_constraint_is_required:
 * @constraint: a #EmeusConstraint
 *
 * Checks whether a @constraint is marked as required.
 *
 * Returns: %TRUE if the constraint is required
 *
 * Since: 1.0
 */
gboolean
emeus_constraint_is_required (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), FALSE);

  return constraint->strength == EMEUS_CONSTRAINT_STRENGTH_REQUIRED;
}

const char *
emeus_constraint_to_string (EmeusConstraint *constraint)
{
  GString *buf;

  if (constraint->description != NULL)
    return constraint->description;

  buf = g_string_new (NULL);

  if (constraint->target_object != NULL)
    {
      g_string_append (buf, G_OBJECT_TYPE_NAME (constraint->target_object));
      g_string_append (buf, ".");
    }

  g_string_append (buf, get_attribute_name (constraint->target_attribute));
  g_string_append (buf, " ");
  g_string_append (buf, get_relation_symbol (constraint->relation));
  g_string_append (buf, " ");

  if (constraint->target_attribute != EMEUS_CONSTRAINT_ATTRIBUTE_INVALID)
    {
      if (constraint->source_object != NULL)
        g_string_append (buf, G_OBJECT_TYPE_NAME (constraint->source_object));
      else
        g_string_append (buf, "parent");

      g_string_append (buf, ".");
      g_string_append (buf, G_OBJECT_TYPE_NAME (constraint->source_attribute));

      if (fabs (constraint->multiplier - 1.0) > DBL_EPSILON)
        g_string_append_printf (buf, " * %g", constraint->multiplier);

      if (fabs (constraint->constant - 0.0) > DBL_EPSILON)
        g_string_append (buf, " + ");
    }

  g_string_append_printf (buf, "%g", constraint->constant);

  g_string_append (buf, " ");

  double strength = strength_to_value (constraint->strength);
  g_string_append_printf (buf, "[%s (%g)]",
                          strength_to_string (strength),
                          strength);

  constraint->description = g_string_free (buf, FALSE);

  return constraint->description;
}

gboolean
emeus_constraint_attach (EmeusConstraint       *constraint,
                         EmeusConstraintLayout *layout)
{
  constraint->layout = layout;
  constraint->solver = emeus_constraint_layout_get_solver (layout);

  return TRUE;
}

void
emeus_constraint_detach (EmeusConstraint *constraint)
{
  if (constraint->constraint != NULL)
    simplex_solver_remove_constraint (constraint->solver, constraint->constraint);

  constraint->constraint = NULL;
  constraint->layout = NULL;
  constraint->solver = NULL;
}

/**
 * emeus_constraint_is_attached:
 * @constraint: a #EmeusConstraint
 *
 * Checks whether the @constraint is attached to a child of a
 * #EmeusConstraintLayout.
 *
 * Returns: %TRUE if the constraint is attached
 *
 * Since: 1.0
 */
gboolean
emeus_constraint_is_attached (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), FALSE);

  return constraint->solver != NULL;
}

/**
 * emeus_constraint_set_active:
 * @constraint: a #EmeusConstraint
 * @active: whether the @constraint should be active
 *
 * Sets whether a @constraint should be actively used by
 * a #EmeusConstraintLayout or not.
 *
 * Since: 1.0
 */
void
emeus_constraint_set_active (EmeusConstraint *constraint,
                             gboolean         active)
{
  g_return_if_fail (EMEUS_IS_CONSTRAINT (constraint));

  active = !!active;

  if (constraint->is_active == active)
    return;

  constraint->is_active = active;

  if (constraint->layout != NULL)
    {
      if (constraint->is_active)
        emeus_constraint_layout_activate_constraint (constraint->layout, constraint);
      else
        emeus_constraint_layout_deactivate_constraint (constraint->layout, constraint);
    }

  g_object_notify_by_pspec (G_OBJECT (constraint), emeus_constraint_properties[PROP_ACTIVE]);
}

/**
 * emeus_constraint_get_active:
 * @constraint: a #EmeusConstraint
 *
 * Checks whether a @constraint is active.
 *
 * Returns: %TRUE if the constraint is active
 *
 * Since: 1.0
 */
gboolean
emeus_constraint_get_active (EmeusConstraint *constraint)
{
  g_return_val_if_fail (EMEUS_IS_CONSTRAINT (constraint), FALSE);

  return constraint->is_active;
}

/**
 * emeus_create_constraints_from_description:
 * @lines: (array length=n_lines): an array of Visual Format Language lines
 *   defining a set of constraints
 * @n_lines: the number of lines
 * @hspacing: default horizontal spacing value, or -1 for the fallback value
 * @vspacing: default vertical spacing value, or -1 for the fallback value
 * @views: (element-type utf8 Gtk.Widget): a dictionary of [ name, widget ]
 *   pairs; the `name` keys map to the view names in the VFL lines, while
 *   the `widget` values map to the children of a #EmeusConstraintLayout
 * @metrics: (element-type utf8 double) (nullable): a dictionary of
 *   [ name, value ] pairs; the `name` keys map to the metric names in the
 *   VFL lines, while the `value` values maps to its numeric value
 *
 * Creates a list of constraints they formal description using the
 * [Visual Format Language](https://developer.apple.com/library/content/documentation/UserExperience/Conceptual/AutolayoutPG/VisualFormatLanguage.html)
 * syntax.
 *
 * The @views dictionary is used to match widgets to the symbolic view name
 * inside the VFL; the @metrics dictionary is used to substitute symbolic
 * names with numeric values, to avoid hard coding constants inside the
 * VFL strings.
 *
 * The VFL grammar is:
 *
 * |[<!-- language="plain" -->
 *        <visualFormatString> = (<orientation>)?
 *                               (<superview><connection>)?
 *                               <view>(<connection><view>)*
 *                               (<connection><superview>)?
 *               <orientation> = 'H' | 'V'
 *                 <superview> = '|'
 *                <connection> = '' | '-' <predicateList> '-' | '-'
 *             <predicateList> = <simplePredicate> | <predicateListWithParens>
 *           <simplePredicate> = <metricName> | <positiveNumber>
 *   <predicateListWithParens> = '(' <predicate> (',' <predicate>)* ')'
 *                 <predicate> = (<relation>)? <objectOfPredicate> (<operatorList>)? ('@' <priority>)?
 *                  <relation> = '==' | '<=' | '>='
 *         <objectOfPredicate> = <constant> | <viewName> | <metricName> ('.' <attributeName>)?
 *                  <priority> = <positiveNumber> | 'required' | 'strong' | 'medium' | 'weak'
 *                  <constant> = <number>
 *              <operatorList> = (<multiplyOperator>)? (<addOperator>)?
 *          <multiplyOperator> = [ '*' | '/' ] <positiveNumber>
 *               <addOperator> = [ '+' | '-' ] <positiveNumber>
 *                  <viewName> = [A-Za-z_]([A-Za-z0-9_]*) // A C identifier
 *                <metricName> = [A-Za-z_]([A-Za-z0-9_]*) // A C identifier
 *             <attributeName> = 'top' | 'bottom' | 'left' | 'right' | 'width' | 'height' |
 *                               'start' | 'end' | 'centerX' | 'centerY' | 'baseline'
 *            <positiveNumber> // A positive real number parseable by g_ascii_strtod()
 *                    <number> // A real number parseable by g_ascii_strtod()
 * ]|
 *
 * **Note**: The VFL grammar is slightly different than the one defined by Apple,
 * as it uses symbolic values for the constraint's priority instead of numeric
 * values.
 *
 * Examples of VFL descriptions are:
 *
 * |[<!-- language="plain" -->
 *   // Default spacing
 *   [button]-[textField]
 *
 *   // Width constraint
 *   [button(>=50)]
 *
 *   // Connection to super view
 *   |-50-[purpleBox]-50-|
 *
 *   // Vertical layout
 *   V:[topField]-10-[bottomField]
 *
 *   // Flush views
 *   [maroonView][blueView]
 *
 *   // Priority
 *   [button(100@strong)]
 *
 *   // Equal widths
 *   [button1(==button2)]
 *
 *   // Multiple predicates
 *   [flexibleButton(>=70,<=100)]
 *
 *   // A complete line of layout
 *   |-[find]-[findNext]-[findField(>=20)]-|
 *
 *   // Operators
 *   [button1(button2 / 3 + 50)]
 *
 *   // Named attributes
 *   [button1(==button2.height)]
 * ]|
 *
 * Returns: (transfer container) (element-type Emeus.Constraint): a list of #EmeusConstraint
 *   instances, to be used with an #EmeusConstraintLayout
 *
 * Since: 1.0
 */
GList *
emeus_create_constraints_from_description (const char * const  lines[],
                                           guint               n_lines,
                                           int                 hspacing,
                                           int                 vspacing,
                                           GHashTable         *views,
                                           GHashTable         *metrics)
{
  g_return_val_if_fail (lines != NULL && n_lines != 0, NULL);

  VflParser *parser = vfl_parser_new (hspacing, vspacing, metrics, views);

  GList *res = NULL;

  for (guint i = 0; i < n_lines; i++)
    {
      const char *line = lines[i];
      GError *error = NULL;

      vfl_parser_parse_line (parser, line, -1, &error);
      if (error != NULL)
        {
          int offset = vfl_parser_get_error_offset (parser);
          int range = vfl_parser_get_error_range (parser);
          char *squiggly = NULL;

          if (range > 0)
            {
              squiggly = g_new (char, range + 1);

              for (int r = 0; r < range; i++)
                squiggly[r] = '~';

              squiggly[range] = '\0';
            }

          g_critical ("VFL parsing error:%d:%d: %s\n"
                      "%s\n"
                      "%*s^%s",
                      i, offset + 1,
                      error->message,
                      line,
                      offset, " ", squiggly != NULL ? squiggly : "");

          g_free (squiggly);
          g_error_free (error);
          vfl_parser_free (parser);
          continue;
        }

      int n_constraints = 0;
      VflConstraint *constraints = vfl_parser_get_constraints (parser, &n_constraints);
      for (int j = 0; j < n_constraints; j++)
        {
          const VflConstraint *c = &constraints[j];
          gpointer source, target;
          EmeusConstraintAttribute source_attr, target_attr;
          EmeusConstraintRelation relation;

          target = g_hash_table_lookup (views, c->view1);
          target_attr = attribute_from_name (c->attr1);

          if (c->view2 != NULL)
            source = g_hash_table_lookup (views, c->view2);
          else
            source = NULL;

          if (c->attr2 != NULL)
            source_attr = attribute_from_name (c->attr2);
          else
            source_attr = EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;

          relation = operator_to_relation (c->relation);

          EmeusConstraint *constraint =
            emeus_constraint_new (target, target_attr,
                                  relation,
                                  source, source_attr,
                                  c->multiplier,
                                  c->constant,
                                  c->strength);

          res = g_list_prepend (res, constraint);
        }

      g_free (constraints);
    }

  return g_list_reverse (res);
}
