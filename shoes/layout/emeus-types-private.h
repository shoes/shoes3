/* emeus-types-private.h: Shared private types
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

#include <stdbool.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _SimplexSolver   SimplexSolver;

typedef enum {
  VARIABLE_DUMMY     = 'd',
  VARIABLE_OBJECTIVE = 'o',
  VARIABLE_SLACK     = 's',
  VARIABLE_REGULAR   = 'v'
} VariableType;

typedef struct {
  int ref_count;

  unsigned long id_;

  VariableType type;

  const char *prefix;
  const char *name;

  double value;

  bool is_external;
  bool is_pivotable;
  bool is_restricted;

  SimplexSolver *solver;
} Variable;

typedef struct {
  double coefficient;

  Variable *variable;
} Term;

typedef struct {
  int ref_count;

  double constant;

  /* HashTable<Variable,Term> */
  GHashTable *terms;

  GList *ordered_terms;

  SimplexSolver *solver;
} Expression;

typedef enum {
  OPERATOR_TYPE_LE = -1,
  OPERATOR_TYPE_EQ = 0,
  OPERATOR_TYPE_GE = 1
} OperatorType;

G_GNUC_PURE
static inline double
create_strength (double a,
                 double b,
                 double c,
                 double w)
{
  double res = 0.0;

  res += CLAMP (a * w, 0.0, 1000.0) * 1000000.0;
  res += CLAMP (b * w, 0.0, 1000.0) * 1000.0;
  res += CLAMP (c * w, 0.0, 1000.0);

  return res;
}

#define STRENGTH_REQUIRED       (create_strength (1000.0, 1000.0, 1000.0, 1.0))
#define STRENGTH_STRONG         (create_strength (   1.0,    0.0,    0.0, 1.0))
#define STRENGTH_MEDIUM         (create_strength (   0.0,    1.0,    0.0, 1.0))
#define STRENGTH_WEAK           (create_strength (   0.0,    0.0,    1.0, 1.0))

typedef struct {
  /* While the Constraint's normal form is expressed in terms of a linear
   * equation between two terms, e.g.:
   *
   *   x = y * coefficient + constant
   *
   * we use a single expression and compare with zero:
   *
   *   x - (y * coefficient + constant) = 0
   *
   * as this simplifies tableau we set up for the simplex solver
   */
  Expression *expression;
  OperatorType op_type;

  /* The variable used by edit and stay constraints */
  Variable *variable;

  double strength;

  bool is_edit;
  bool is_stay;

  SimplexSolver *solver;
} Constraint;

#define SIMPLEX_SOLVER_INIT     \
  { false, \
    NULL, NULL, \
    NULL, NULL, NULL, \
    NULL, \
    NULL, NULL, \
    NULL, NULL, \
    NULL, \
    NULL, \
    0, 0, 0, 0, 0, \
    false, false, \
  }

struct _SimplexSolver {
  bool initialized;

  /* HashTable<Variable, HashSet<Variable>> */
  GHashTable *columns;

  /* HashTable<Variable, Expression> */
  GHashTable *rows;

  /* Sets */
  GHashTable *external_rows;
  GHashTable *external_parametric_vars;

  GPtrArray *infeasible_rows;
  GPtrArray *stay_error_vars;

  GHashTable *error_vars;
  GHashTable *marker_vars;

  GHashTable *edit_var_map;
  GHashTable *stay_var_map;

  Variable *objective;

  GHashTable *constraints;

  int slack_counter;
  int artificial_counter;
  int dummy_counter;
  int optimize_count;
  int freeze_count;

  bool auto_solve;
  bool needs_solving;
};

G_END_DECLS
