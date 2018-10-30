/* emeus-constraint-private.h: The base constraint object
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

#include "emeus-constraint.h"
//#include "emeus-constraint-layout-private.h"
#include "emeus-types-private.h"

G_BEGIN_DECLS

struct _EmeusConstraint
{
  GInitiallyUnowned parent_instance;

  gpointer target_object;
  EmeusConstraintAttribute target_attribute;

  EmeusConstraintRelation relation;

  gpointer source_object;
  EmeusConstraintAttribute source_attribute;

  double multiplier;
  double constant;

  int strength;

  gboolean is_active;

  char *description;
  SimplexSolver *solver;
  Constraint *constraint;
  EmeusConstraintLayout *layout;
};

gboolean        emeus_constraint_attach                 (EmeusConstraint       *constraint,
                                                         EmeusConstraintLayout *layout);
void            emeus_constraint_detach                 (EmeusConstraint       *constraint);

Constraint *    emeus_constraint_get_real_constraint    (EmeusConstraint       *constraint);

const char *    emeus_constraint_to_string              (EmeusConstraint       *constraint);

G_END_DECLS
