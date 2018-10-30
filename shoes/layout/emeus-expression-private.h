/* emeus-expression-private.h: A set of terms and a constant
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
variable_is_dummy (const Variable *variable)
{
  return variable->type == VARIABLE_DUMMY;
}

static inline bool
variable_is_objective (const Variable *variable)
{
  return variable->type == VARIABLE_OBJECTIVE;
}

static inline bool
variable_is_slack (const Variable *variable)
{
  return variable->type == VARIABLE_SLACK;
}

static inline bool
variable_is_external (const Variable *variable)
{
  return variable->is_external;
}

static inline bool
variable_is_pivotable (const Variable *variable)
{
  return variable->is_pivotable;
}

static inline gboolean
variable_is_restricted (const Variable *variable)
{
  return variable->is_restricted;
}

static inline double
variable_get_value (const Variable *variable)
{
  if (variable_is_dummy (variable) ||
      variable_is_objective (variable) ||
      variable_is_slack (variable))
    return 0.0;

  return variable->value;
}

Variable *variable_new (SimplexSolver *solver,
                        VariableType type);
Variable *variable_ref (Variable *variable);
void variable_unref (Variable *variable);

void variable_set_value (Variable *variable,
                         double value);

char *variable_to_string (const Variable *variable);

void variable_set_name (Variable *variable,
                        const char *name);

void variable_set_prefix (Variable *variable,
                          const char *prefix);

static inline Variable *
term_get_variable (const Term *term)
{
  return term->variable;
}

static inline double
term_get_coefficient (const Term *term)
{
  return term->coefficient;
}

static inline bool
expression_is_constant (const Expression *expression)
{
  return expression->terms == NULL;
}

static inline double
expression_get_constant (const Expression *expression)
{
  return expression->constant;
}

Expression *expression_new (SimplexSolver *solver,
                            double constant);

Expression *expression_new_from_constant (double constant);
Expression *expression_new_from_variable (Variable *variable);

Expression *expression_clone (Expression *expression);

Expression *expression_ref (Expression *expression);
void expression_unref (Expression *expression);

void expression_set_constant (Expression *expression,
                              double constant);

void expression_set_variable (Expression *expression,
                              Variable *variable,
                              double coefficient);

void expression_add_variable (Expression *expression,
                              Variable *variable,
                              double value,
                              Variable *subject);

void expression_remove_variable (Expression *expression,
                                 Variable *variable,
                                 Variable *subject);

bool expression_has_variable (Expression *expression,
                              Variable *variable);

void expression_add_expression (Expression *a,
                                Expression *b,
                                double n,
                                Variable *subject);

Expression *expression_plus (Expression *expression,
                             double constant);
Expression *expression_times (Expression *expression,
                              double multiplier);
Expression *expression_minus (Expression *expression,
                              double constant);
Expression *expression_divide (Expression *expression,
                               double constant);

Expression *expression_plus_variable (Expression *expression,
                                      Variable *variable);
Expression *expression_minus_variable (Expression *expression,
                                       Variable *variable);

double expression_get_coefficient (const Expression *expression,
                                   Variable *variable);

double expression_get_value (const Expression *expression);

typedef bool (* ExpressionForeachTermFunc) (Term *term, gpointer data);

void expression_terms_foreach (Expression *expression,
                               ExpressionForeachTermFunc func,
                               gpointer data);

GList *expression_get_terms (Expression *expression);

void expression_change_subject (Expression *expression,
                                Variable *old_subject,
                                Variable *new_subject);

double expression_new_subject (Expression *expression,
                               Variable *subject);

void expression_substitute_out (Expression *expression,
                                Variable *out_var,
                                Expression *expr,
                                Variable *subject);

Variable *expression_get_pivotable_variable (Expression *expression);

char *expression_to_string (const Expression *expression);

G_END_DECLS
