/* emeus-simplex-solver-private.h: The constraint solver
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

#include "emeus-types-private.h"

G_BEGIN_DECLS

static inline bool
constraint_is_inequality (const Constraint *constraint)
{
  return constraint->op_type != OPERATOR_TYPE_EQ;
}

static inline bool
constraint_is_required (const Constraint *constraint)
{
  return constraint->strength >= STRENGTH_REQUIRED;
}

static inline bool
constraint_is_stay (const Constraint *constraint)
{
  return constraint->is_stay;
}

static inline bool
constraint_is_edit (const Constraint *constraint)
{
  return constraint->is_edit;
}

void simplex_solver_init (SimplexSolver *solver);
void simplex_solver_clear (SimplexSolver *solver);
void simplex_solver_reset (SimplexSolver *solver);

void simplex_solver_freeze (SimplexSolver *solver);
void simplex_solver_thaw (SimplexSolver *solver);

Variable *simplex_solver_create_variable (SimplexSolver *solver,
                                          const char *name,
                                          double value);
Expression *simplex_solver_create_expression (SimplexSolver *solver,
                                              double constant);

Constraint *simplex_solver_add_constraint (SimplexSolver *solver,
                                           Variable *variable,
                                           OperatorType op,
                                           Expression *expression,
                                           double strength);

Constraint *simplex_solver_add_stay_variable (SimplexSolver *solver,
                                              Variable *variable,
                                              double strength);

Constraint *simplex_solver_add_edit_variable (SimplexSolver *solver,
                                              Variable *variable,
                                              double strength);

bool simplex_solver_has_edit_variable (SimplexSolver *solver,
                                       Variable *variable);

bool simplex_solver_has_stay_variable (SimplexSolver *solver,
                                       Variable *variable);

void simplex_solver_remove_constraint (SimplexSolver *solver,
                                       Constraint *constraint);

void simplex_solver_remove_edit_variable (SimplexSolver *solver,
                                          Variable *variable);

void simplex_solver_remove_stay_variable (SimplexSolver *solver,
                                          Variable *variable);

void simplex_solver_suggest_value (SimplexSolver *solver,
                                   Variable *variable,
                                   double value);

void simplex_solver_resolve (SimplexSolver *solver);

void simplex_solver_begin_edit (SimplexSolver *solver);
void simplex_solver_end_edit (SimplexSolver *solver);

/* Internal */
void simplex_solver_note_added_variable (SimplexSolver *solver,
                                         Variable *variable,
                                         Variable *subject);
void simplex_solver_note_removed_variable (SimplexSolver *solver,
                                           Variable *variable,
                                           Variable *subject);

G_END_DECLS
