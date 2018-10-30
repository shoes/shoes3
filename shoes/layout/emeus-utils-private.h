/* emeus-utils-private.h: Internal utility functions
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

#include "emeus-types.h"
#include "emeus-types-private.h"

G_BEGIN_DECLS

const char *get_attribute_name (EmeusConstraintAttribute attr);
const char *get_relation_symbol (EmeusConstraintRelation rel);

EmeusConstraintAttribute attribute_from_name (const char *name);

OperatorType relation_to_operator (EmeusConstraintRelation rel);
double strength_to_value (EmeusConstraintStrength strength);

EmeusConstraintRelation operator_to_relation (OperatorType op);
EmeusConstraintStrength value_to_strength (double strength);

const char *operator_to_string (OperatorType op);
const char *strength_to_string (double strength);

bool approx_val (double v1, double v2);

G_END_DECLS
