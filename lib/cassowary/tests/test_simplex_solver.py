from __future__ import print_function, unicode_literals, absolute_import, division

from unittest import TestCase
if not hasattr(TestCase, 'assertIsNotNone'):
    # For Python2.6 compatibility
    from unittest2 import TestCase

from cassowary import Variable, SimplexSolver, STRONG, REQUIRED

# internals
from cassowary.expression import Constraint


class SimplexSolverTestCase(TestCase):
    def test_constructor(self):
        "A solver can be constructed"
        solver = SimplexSolver()

        self.assertEqual(len(solver.columns), 0)
        self.assertEqual(len(solver.rows), 1)
        self.assertEqual(len(solver.infeasible_rows), 0)
        self.assertEqual(len(solver.external_rows), 0)
        self.assertEqual(len(solver.external_parametric_vars), 0)

    def test_add_edit_var_required(self):
        "Solver works with REQUIRED strength"
        solver = SimplexSolver()

        a = Variable(name='a')

        solver.add_stay(a, STRONG, 0)
        solver.resolve()

        self.assertEqual(a.value, 0)

        solver.add_edit_var(a, REQUIRED)
        solver.begin_edit()
        solver.suggest_value(a, 2)
        solver.resolve()

        self.assertEqual(a.value, 2)

    def test_add_edit_var_required_after_suggestions(self):
        "Solver works with REQUIRED strength after many suggestions"
        solver = SimplexSolver()
        a = Variable(name='a')
        b = Variable(name='b')

        solver.add_stay(a, STRONG, 0)
        solver.add_constraint(Constraint(a, Constraint.EQ, b, REQUIRED))
        solver.resolve()

        self.assertEqual(b.value, 0)
        self.assertEqual(a.value, 0)

        solver.add_edit_var(a, REQUIRED)
        solver.begin_edit()
        solver.suggest_value(a, 2)
        solver.resolve()

        self.assertEqual(a.value, 2)
        self.assertEqual(b.value, 2)

        solver.suggest_value(a, 10)
        solver.resolve()

        self.assertEqual(a.value, 10)
        self.assertEqual(b.value, 10)

