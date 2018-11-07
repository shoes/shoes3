/* emeus-constraint-layout-private.h: The constraint layout manager
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

#include "emeus-constraint-layout.h"
#include "emeus-types-private.h"
#include "shoes/layout/gshoes-ele.h"

G_BEGIN_DECLS

struct _EmeusConstraintLayoutChild
{
  //GtkBin parent_instance;
  GshoesEle *parent_instance;

  /* An optional name for the child, to be used when building
   * constraints from string descriptions
   */
  char *name;

  /* Position in the layout's GSequence */
  GSequenceIter *iter;

  /* Back pointer to the solver in the layout */
  SimplexSolver *solver;

  /* HashTable<static string, Variable>; a hash table of variables,
   * one for each attribute; we use these to query and suggest the
   * values for the solver. The string is static and does not need
   * to be freed.
   */
  GHashTable *bound_attributes;

  /* HashSet<EmeusConstraint>; the set of constraints on the
   * widget, using the public API objects.
   */
  GHashTable *constraints;

  double intrinsic_width;
  double intrinsic_height;

  /* Internal constraints, created to satisfy specific bound
   * attributes; may be unset.
   */
  Constraint *right_constraint;
  Constraint *bottom_constraint;
  Constraint *center_x_constraint;
  Constraint *center_y_constraint;
  Constraint *width_constraint;
  Constraint *height_constraint;
};

struct _EmeusConstraintLayout
{
  //GtkContainer parent_instance;
  void * parent_instance;  // points to shoes_layout struct 

  GSequence *children;

  SimplexSolver solver;

  GHashTable *bound_attributes;

  /* HashSet<EmeusConstraint>; the set of constraints on the
   * widget, using the public API objects.
   */
  GHashTable *constraints;

  /* Internal constraints */
  struct {
    Constraint *top;
    Constraint *left;
    Constraint *width;
    Constraint *height;
  } stays;
};

SimplexSolver * emeus_constraint_layout_get_solver      (EmeusConstraintLayout *layout);

gboolean        emeus_constraint_layout_has_child_data  (EmeusConstraintLayout *layout,
                                                         GshoesEle             *widget);

void            emeus_constraint_layout_activate_constraint     (EmeusConstraintLayout *layout,
                                                                 EmeusConstraint       *constraint);
void            emeus_constraint_layout_deactivate_constraint   (EmeusConstraintLayout *layout,
                                                                 EmeusConstraint       *constraint);

G_END_DECLS
