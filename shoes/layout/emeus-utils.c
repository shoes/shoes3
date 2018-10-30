/* emeus-utils.c: Internal utility functions
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

#include "config.h"

#include "emeus-utils-private.h"

#include <string.h>
#include <math.h>
#include <float.h>

static const char *attribute_names[] = {
  [EMEUS_CONSTRAINT_ATTRIBUTE_INVALID]  = "invalid",
  [EMEUS_CONSTRAINT_ATTRIBUTE_LEFT]     = "left",
  [EMEUS_CONSTRAINT_ATTRIBUTE_RIGHT]    = "right",
  [EMEUS_CONSTRAINT_ATTRIBUTE_TOP]      = "top",
  [EMEUS_CONSTRAINT_ATTRIBUTE_BOTTOM]   = "bottom",
  [EMEUS_CONSTRAINT_ATTRIBUTE_START]    = "start",
  [EMEUS_CONSTRAINT_ATTRIBUTE_END]      = "end",
  [EMEUS_CONSTRAINT_ATTRIBUTE_WIDTH]    = "width",
  [EMEUS_CONSTRAINT_ATTRIBUTE_HEIGHT]   = "height",
  [EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X] = "center-x",
  [EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y] = "center-y",
  [EMEUS_CONSTRAINT_ATTRIBUTE_BASELINE] = "baseline",
};

static const char *relation_symbols[] = {
  [EMEUS_CONSTRAINT_RELATION_EQ] = "==",
  [EMEUS_CONSTRAINT_RELATION_LE] = "<=",
  [EMEUS_CONSTRAINT_RELATION_GE] = ">=",
};

const char *
get_attribute_name (EmeusConstraintAttribute attr)
{
  return attribute_names[attr];
}

const char *
get_relation_symbol (EmeusConstraintRelation rel)
{
  return relation_symbols[rel];
}

EmeusConstraintAttribute
attribute_from_name (const char *name)
{
  if (name == NULL || *name == '\0')
    return EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;

  /* We sadly need to special case these two because the name does
   * not match the VFL grammar rules
   */
  if (strcmp (name, "centerX") == 0)
    return EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_X;

  if (strcmp (name, "centerY") == 0)
    return EMEUS_CONSTRAINT_ATTRIBUTE_CENTER_Y;

  for (int i = 0; i < G_N_ELEMENTS (attribute_names); i++)
    {
      if (strcmp (attribute_names[i], name) == 0)
        return i;
    }

  return EMEUS_CONSTRAINT_ATTRIBUTE_INVALID;
}

OperatorType
relation_to_operator (EmeusConstraintRelation rel)
{
  switch (rel)
    {
    case EMEUS_CONSTRAINT_RELATION_EQ:
      return OPERATOR_TYPE_EQ;
    case EMEUS_CONSTRAINT_RELATION_LE:
      return OPERATOR_TYPE_LE;
    case EMEUS_CONSTRAINT_RELATION_GE:
      return OPERATOR_TYPE_GE;
    }

  return OPERATOR_TYPE_EQ;
}

EmeusConstraintRelation
operator_to_relation (OperatorType op)
{
  switch (op)
    {
    case OPERATOR_TYPE_EQ:
      return EMEUS_CONSTRAINT_RELATION_EQ;
    case OPERATOR_TYPE_LE:
      return EMEUS_CONSTRAINT_RELATION_LE;
    case OPERATOR_TYPE_GE:
      return EMEUS_CONSTRAINT_RELATION_GE;
    }

  return EMEUS_CONSTRAINT_RELATION_EQ;
}

double
strength_to_value (int strength)
{
  switch (strength)
    {
    case EMEUS_CONSTRAINT_STRENGTH_REQUIRED:
      return STRENGTH_REQUIRED;

    case EMEUS_CONSTRAINT_STRENGTH_WEAK:
      return STRENGTH_WEAK;

    case EMEUS_CONSTRAINT_STRENGTH_MEDIUM:
      return STRENGTH_MEDIUM;

    case EMEUS_CONSTRAINT_STRENGTH_STRONG:
      return STRENGTH_STRONG;
    }

  return strength;
}

EmeusConstraintStrength
value_to_strength (double strength)
{
  if (strength >= STRENGTH_REQUIRED)
    return EMEUS_CONSTRAINT_STRENGTH_REQUIRED;

  if (strength >= STRENGTH_STRONG)
    return EMEUS_CONSTRAINT_STRENGTH_STRONG;

  if (strength >= STRENGTH_MEDIUM)
    return EMEUS_CONSTRAINT_STRENGTH_MEDIUM;

  return EMEUS_CONSTRAINT_STRENGTH_WEAK;
}

static const char *operators[] = {
  "<=",
  "==",
  ">=",
};

const char *
operator_to_string (OperatorType o)
{
  return operators[o + 1];
}

const char *
strength_to_string (double s)
{
  if (s >= STRENGTH_REQUIRED)
    return "required";

  if (s >= STRENGTH_STRONG)
    return "strong";

  if (s >= STRENGTH_MEDIUM)
    return "medium";

  return "weak";
}

bool
approx_val (double v1,
            double v2)
{
  return fabs (v1 - v2) < DBL_EPSILON;
}
