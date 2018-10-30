/* emeus-types.h: Shared public types
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

#pragma once

#include <glib-object.h>

#include "shoes/layout/emeus-version-macros.h"

G_BEGIN_DECLS

/**
 * EmeusConstraintStrength:
 * @EMEUS_CONSTRAINT_STRENGTH_WEAK: Weak constraint
 * @EMEUS_CONSTRAINT_STRENGTH_MEDIUM: Medium constraint
 * @EMEUS_CONSTRAINT_STRENGTH_STRONG: Strong constraint
 * @EMEUS_CONSTRAINT_STRENGTH_REQUIRED: Requires constraint
 *
 * The possible strengths for a constraint.
 *
 * The order is:
 *
 * |[<!-- language="plain" -->
 *   weak < medium < strong < required
 * ]|
 *
 * Since: 1.0
 */
typedef enum {
  EMEUS_CONSTRAINT_STRENGTH_REQUIRED = 0,
  EMEUS_CONSTRAINT_STRENGTH_WEAK     = -1,
  EMEUS_CONSTRAINT_STRENGTH_MEDIUM   = -2,
  EMEUS_CONSTRAINT_STRENGTH_STRONG   = -3
} EmeusConstraintStrength;

#define EMEUS_TYPE_CONSTRAINT_STRENGTH (emeus_constraint_strength_get_type())

EMEUS_AVAILABLE_IN_1_0
GType emeus_constraint_strength_get_type (void) G_GNUC_CONST;

/**
 * EmeusConstraintRelation:
 * @EMEUS_CONSTRAINT_RELATION_LE: Less than, or equal
 * @EMEUS_CONSTRAINT_RELATION_EQ: Equal
 * @EMEUS_CONSTRAINT_RELATION_GE: Greater than, or equal
 *
 * The relation between the two terms of the constraint.
 *
 * Since: 1.0
 */
typedef enum
{
  EMEUS_CONSTRAINT_RELATION_LE,
  EMEUS_CONSTRAINT_RELATION_EQ,
  EMEUS_CONSTRAINT_RELATION_GE
} EmeusConstraintRelation;

#define EMEUS_TYPE_CONSTRAINT_RELATION (emeus_constraint_relation_get_type())

EMEUS_AVAILABLE_IN_1_0
GType emeus_constraint_relation_get_type (void) G_GNUC_CONST;

/**
 * EmeusConstraintAttribute:
 * @EMEUS_CONSTRAINT_ATTRIBUTE_INVALID: Used for constants
 * @EMEUS_CONSTRAINT_ATTRIBUTE_LEFT: The left edge of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT: The right edge of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_TOP: The top edge of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM: The bottom edge of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_START: The leading edge of a widget, depending
 *   on the text direction (left for LTR languages, right for RTL ones)
 * @EMEUS_CONSTRAINT_ATTRIBUTE_END: The trailing edge of a widget, depending
 *   on the text direction (right for LTR languages, left for RTL ones)
 * @EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH: The width of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT: The height of a widget
 * @EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X: The center of a widget, on the horizontal
 *   axis
 * @EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y: The center of a widget, on the vertical
 *   axis
 * @EMEUS_CONSTRAINT_ATTRIBUTE_BASELINE: The baseline of a widget
 *
 * The attributes that can be used to build a constraint.
 *
 * Since: 1.0
 */
typedef enum
{
  EMEUS_CONSTRAINT_ATTRIBUTE_INVALID,

  EMEUS_CONSTRAINT_ATTRIBUTE_LEFT,
  EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT,
  EMEUS_CONSTRAINT_ATTRIBUTE_TOP,
  EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM,
  EMEUS_CONSTRAINT_ATTRIBUTE_START,
  EMEUS_CONSTRAINT_ATTRIBUTE_END,
  EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH,
  EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT,
  EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X,
  EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y,
  EMEUS_CONSTRAINT_ATTRIBUTE_BASELINE
} EmeusConstraintAttribute;

#define EMEUS_TYPE_CONSTRAINT_ATTRIBUTE (emeus_constraint_attribute_get_type())

EMEUS_AVAILABLE_IN_1_0
GType emeus_constraint_attribute_get_type (void) G_GNUC_CONST;

G_END_DECLS
