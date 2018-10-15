from __future__ import print_function, unicode_literals, absolute_import, division

from unittest import TestCase
if not hasattr(TestCase, 'assertIsNotNone'):
    # For Python2.6 compatibility
    from unittest2 import TestCase

from cassowary import InternalError, Variable

# Internals
from cassowary.expression import Expression, SlackVariable


class ExpressionTestCase(TestCase):
    def assertExpressionEqual(self, expr, output):
        self.assertEqual(repr(expr), output)

    def test_empty_expression(self):
        expr = Expression()
        self.assertExpressionEqual(expr, '0.0')
        self.assertAlmostEqual(expr.constant, 0.0)
        self.assertEqual(len(expr.terms), 0)

    def test_full_expression(self):
        x = Variable('x', 167)

        expr = Expression(x, 2, 3)
        self.assertExpressionEqual(expr, '3.0 + 2.0*x[167.0]')
        self.assertAlmostEqual(expr.constant, 3.0)
        self.assertEqual(len(expr.terms), 1)
        self.assertAlmostEqual(expr.terms.get(x), 2.0)

    def test_variable_expression(self):
        x = Variable('x', 167)

        expr = Expression(x)
        self.assertExpressionEqual(expr, 'x[167.0]')
        self.assertAlmostEqual(expr.constant, 0.0)
        self.assertEqual(len(expr.terms), 1)
        self.assertAlmostEqual(expr.terms.get(x), 1.0)

        expr = Expression(x, 3)
        self.assertExpressionEqual(expr, '3.0*x[167.0]')
        self.assertAlmostEqual(expr.constant, 0.0)
        self.assertEqual(len(expr.terms), 1)
        self.assertAlmostEqual(expr.terms.get(x), 3.0)

    def test_constant_expression(self):
        expr = Expression(constant=4)
        self.assertExpressionEqual(expr, '4.0')
        self.assertAlmostEqual(expr.constant, 4.0)
        self.assertEqual(len(expr.terms), 0)

    def test_add(self):
        x = Variable('x', 167)
        y = Variable('y', 42)

        # Add a constant to an expression
        self.assertExpressionEqual(Expression(x) + 2, '2.0 + x[167.0]')
        self.assertExpressionEqual(3 + Expression(x), '3.0 + x[167.0]')

        # Add a variable to an expression
        self.assertExpressionEqual(y + Expression(x), 'x[167.0] + y[42.0]')
        self.assertExpressionEqual(Expression(x) + y, 'x[167.0] + y[42.0]')

        # Add an expression to an expression
        self.assertExpressionEqual(Expression(x) + Expression(y), 'x[167.0] + y[42.0]')
        self.assertExpressionEqual(Expression(x, 20, 2) + Expression(y, 10, 5), '7.0 + 20.0*x[167.0] + 10.0*y[42.0]')

    def test_sub(self):
        x = Variable('x', 167)
        y = Variable('y', 42)

        # Subtract a constant from an expression
        self.assertExpressionEqual(Expression(x) - 2, '-2.0 + x[167.0]')
        self.assertExpressionEqual(3 - Expression(x), '3.0 + -1.0*x[167.0]')

        # Subtract a variable from an expression
        self.assertExpressionEqual(y - Expression(x), '-1.0*x[167.0] + y[42.0]')
        self.assertExpressionEqual(Expression(x) - y, 'x[167.0] + -1.0*y[42.0]')

        # Subtract an expression from an expression
        self.assertExpressionEqual(Expression(x) - Expression(y), 'x[167.0] + -1.0*y[42.0]')
        self.assertExpressionEqual(Expression(x, 20, 2) - Expression(y, 10, 5), '-3.0 + 20.0*x[167.0] + -10.0*y[42.0]')

    def test_mul(self):
        x = Variable('x', 167)
        y = Variable('y', 42)

        # Multiply an expression by a constant
        self.assertExpressionEqual(Expression(x) * 2, '2.0*x[167.0]')
        self.assertExpressionEqual(3 * Expression(x), '3.0*x[167.0]')

        # Can't multiply an expression by a variable unless the expression is a constant
        with self.assertRaises(TypeError):
            y * Expression(x)
        with self.assertRaises(TypeError):
            Expression(x) * y
        self.assertExpressionEqual(x * Expression(constant=2), '2.0*x[167.0]')
        self.assertExpressionEqual(Expression(constant=3) * x, '3.0*x[167.0]')

        # Can't multiply an expression by an expression unless
        # one of the expressions is a constant.
        with self.assertRaises(TypeError):
            Expression(x) * Expression(y)
        with self.assertRaises(TypeError):
            Expression(x, 20, 2) * Expression(y, 10, 5)
        self.assertExpressionEqual(Expression(x, 20, 2) * Expression(constant=5), '10.0 + 100.0*x[167.0]')
        self.assertExpressionEqual(Expression(x, 20) * Expression(constant=5), '100.0*x[167.0]')
        self.assertExpressionEqual(Expression(constant=2) * Expression(y, 10, 5), '10.0 + 20.0*y[42.0]')
        self.assertExpressionEqual(Expression(constant=2) * Expression(y, 10), '20.0*y[42.0]')

    def test_complex_math(self):
        x = Variable('x', 167)
        y = Variable('y', 2)
        ex = 4 + x * 3 + 2 * y
        self.assertExpressionEqual(ex, '4.0 + 3.0*x[167.0] + 2.0*y[2.0]')

    def test_clone(self):
        v = Variable('v', 10)
        expr = Expression(v, 20, 2)
        clone = expr.clone()

        self.assertEqual(clone.constant, expr.constant)
        self.assertEqual(len(clone.terms), len(expr.terms))
        self.assertEqual(clone.terms.get(v), 20)

    def test_is_constant(self):
        e1 = Expression()
        e2 = Expression(constant=10)
        e3 = Expression(Variable('o', 10), 20)
        e4 = Expression(Variable('o', 10), 20, 2)

        self.assertTrue(e1.is_constant)
        self.assertTrue(e2.is_constant)
        self.assertFalse(e3.is_constant)
        self.assertFalse(e4.is_constant)

    def test_multiply(self):
        v = Variable('v', 10)
        expr = Expression(v, 20, 2)
        expr.multiply(-1)

        self.assertExpressionEqual(expr, '-2.0 + -20.0*v[10.0]')

    def test_add_variable(self):
        o = Variable('o', 10)
        a = Expression(o, 20, 2)
        v = Variable('v', 20)

        self.assertEqual(len(a.terms), 1)
        self.assertAlmostEqual(a.terms.get(o), 20.0)

        # implicit coefficient of 1
        a.add_variable(v)
        self.assertEqual(len(a.terms), 2)
        self.assertAlmostEqual(a.terms.get(v), 1.0)

        # add again, with different coefficient
        a.add_variable(v, 2.0)
        self.assertEqual(len(a.terms), 2)
        self.assertAlmostEqual(a.terms.get(v), 3.0)

        # add again, with resulting 0 coefficient. should remove the term.
        a.add_variable(v, -3)
        self.assertEqual(len(a.terms), 1)
        self.assertIsNone(a.terms.get(v))

        # try adding the removed term back, with 0 coefficient
        a.add_variable(v, 0)
        self.assertEqual(len(a.terms), 1)
        self.assertIsNone(a.terms.get(v))

    def test_add_expression_variable(self):
        a = Expression(Variable('o', 10), 20, 2)
        v = Variable('v', 20)

        # should work just like add_variable
        a.add_expression(v, 2)
        self.assertEqual(len(a.terms), 2)
        self.assertEqual(a.terms.get(v), 2)

    def test_add_expression(self):
        va = Variable('a', 10)
        vb = Variable('b', 20)
        vc = Variable('c', 5)
        a = Expression(va, 20, 2)

        # different variable and implicit coefficient of 1, should make new term
        a.add_expression(Expression(vb, 10, 5))
        self.assertEqual(len(a.terms), 2)
        self.assertEqual(a.constant, 7)
        self.assertEqual(a.terms.get(vb), 10)

        # same variable, should reuse existing term
        a.add_expression(Expression(vb, 2, 5))
        self.assertEqual(len(a.terms), 2)
        self.assertEqual(a.constant, 12)
        self.assertEqual(a.terms.get(vb), 12)

        # another variable and a coefficient,
        # should multiply the constant and all terms in the new expression
        a.add_expression(Expression(vc, 1, 2), 2)
        self.assertEqual(len(a.terms), 3)
        self.assertEqual(a.constant, 16)
        self.assertEqual(a.terms.get(vc), 2)

    def test_coefficient_for(self):
        va = Variable('a', 10)
        vb = Variable('b', 20)
        a = Expression(va, 20, 2)

        self.assertEqual(a.coefficient_for(va), 20)
        self.assertEqual(a.coefficient_for(vb), 0)

    def test_set_variable(self):
        va = Variable('a', 10)
        vb = Variable('b', 20)
        a = Expression(va, 20, 2)

        # set existing variable
        a.set_variable(va, 2)
        self.assertEqual(len(a.terms), 1)
        self.assertEqual(a.coefficient_for(va), 2)

        # set new variable
        a.set_variable(vb, 2)
        self.assertEqual(len(a.terms), 2)
        self.assertEqual(a.coefficient_for(vb), 2)

    def test_any_pivotable_variable(self):
        # t.e(c.InternalError, Expression(10), 'any_pivotable_variable')
        e = Expression(constant=10)
        with self.assertRaises(InternalError):
            e.any_pivotable_variable()
        # t.e(c.InternalError, Expression(10), 'any_pivotable_variable')

        va = Variable('a', 10)
        vb = SlackVariable('slack', 1)
        a = Expression(va, 20, 2)

        self.assertIsNone(a.any_pivotable_variable())

        a.set_variable(vb, 2)
        self.assertEqual(vb, a.any_pivotable_variable())

    def test_substitute_out(self):
        v1 = Variable('1', 20)
        v2 = Variable('2', 2)
        a = Expression(v1, 2, 2)  # 2*v1 + 2

        # new variable
        a.substitute_out(v1, Expression(v2, 4, 4))
        self.assertEqual(a.constant, 10)
        self.assertIsNone(a.terms.get(v1))
        self.assertEqual(a.terms.get(v2), 8)

        # existing variable
        a.set_variable(v1, 1)
        a.substitute_out(v2, Expression(v1, 2, 2))

        self.assertEqual(a.constant, 26)
        self.assertIsNone(a.terms.get(v2))
        self.assertEqual(a.terms.get(v1), 17)

    def test_new_subject(self):
        v = Variable('v', 10)
        e = Expression(v, 2, 5)

        self.assertEqual(e.new_subject(v), 0.5)
        self.assertEqual(e.constant, -2.5)
        self.assertIsNone(e.terms.get(v))
        self.assertTrue(e.is_constant)

    def test_change_subject(self):
        va = Variable('a', 10)
        vb = Variable('b', 5)
        e = Expression(va, 2, 5)

        e.change_subject(vb, va)
        self.assertEqual(e.constant, -2.5)
        self.assertIsNone(e.terms.get(va))
        self.assertEqual(e.terms.get(vb), 0.5)
