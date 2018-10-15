from __future__ import print_function, unicode_literals, absolute_import, division

from unittest import TestCase
if not hasattr(TestCase, 'assertIsNotNone'):
    # For Python2.6 compatibility
    from unittest2 import TestCase

from cassowary import Variable

# Internals
from cassowary.expression import DummyVariable, SlackVariable, ObjectiveVariable


class VariableTestCase(TestCase):
    def assertExpressionEqual(self, expr, output):
        self.assertEqual(repr(expr), output)

    def test_Variable(self):
        "A Variable can be constructed."
        var = Variable('foo')

        self.assertEqual(var.name, 'foo')
        self.assertAlmostEqual(var.value, 0.0)
        self.assertFalse(var.is_dummy)
        self.assertTrue(var.is_external)
        self.assertFalse(var.is_pivotable)
        self.assertFalse(var.is_restricted)

        self.assertEqual(repr(var), 'foo[0.0]')

    def test_Variable_with_value(self):
        "A Variable can be constructed with a value."
        var = Variable('foo', 42.0)

        self.assertEqual(var.name, 'foo')
        self.assertAlmostEqual(var.value, 42.0)
        self.assertFalse(var.is_dummy)
        self.assertTrue(var.is_external)
        self.assertFalse(var.is_pivotable)
        self.assertFalse(var.is_restricted)

        self.assertEqual(repr(var), 'foo[42.0]')

    def test_DummyVariable(self):
        "A Dummy Variable can be constructed."
        var = DummyVariable(3)

        self.assertEqual(var.name, 'd3')
        self.assertTrue(var.is_dummy)
        self.assertFalse(var.is_external)
        self.assertFalse(var.is_pivotable)
        self.assertTrue(var.is_restricted)

        self.assertEqual(repr(var), 'd3:dummy')

    def test_SlackVariable(self):
        "A Slack Variable can be constructed."
        var = SlackVariable('foo', 3)

        self.assertEqual(var.name, 'foo3')
        self.assertFalse(var.is_dummy)
        self.assertFalse(var.is_external)
        self.assertTrue(var.is_pivotable)
        self.assertTrue(var.is_restricted)

        self.assertEqual(repr(var), 'foo3:slack')

    def test_ObjectiveVariable(self):
        "An Objective Variable can be constructed."
        var = ObjectiveVariable('foo')

        self.assertEqual(var.name, 'foo')
        self.assertFalse(var.is_dummy)
        self.assertFalse(var.is_external)
        self.assertFalse(var.is_pivotable)
        self.assertFalse(var.is_restricted)

        self.assertEqual(repr(var), 'foo:obj')

    def test_add(self):
        x = Variable('x', 167)

        # Add a constant to an expression
        self.assertExpressionEqual(x + 2, '2.0 + x[167.0]')
        self.assertExpressionEqual(3 + x, '3.0 + x[167.0]')

        # Any other type fails
        with self.assertRaises(TypeError):
            x + object()
        with self.assertRaises(TypeError):
            object() + x

    def test_sub(self):
        x = Variable('x', 167)

        # Subtract a constant from an expression
        self.assertExpressionEqual(x - 2, '-2.0 + x[167.0]')
        self.assertExpressionEqual(3 - x, '3.0 + -1.0*x[167.0]')

        # Any other type fails
        with self.assertRaises(TypeError):
            x - object()
        with self.assertRaises(TypeError):
            object() - x

    def test_mul(self):
        x = Variable('x', 167)

        # Multiply an expression by a constant
        self.assertExpressionEqual(x * 2, '2.0*x[167.0]')
        self.assertExpressionEqual(3 * x, '3.0*x[167.0]')

        # Any other type fails
        with self.assertRaises(TypeError):
            x * object()
        with self.assertRaises(TypeError):
            object() * x

    def test_div(self):
        x = Variable('x', 167)

        # Multiply an expression by a constant
        self.assertExpressionEqual(x / 2, '0.5*x[167.0]')

        # No reverse division, however
        with self.assertRaises(TypeError):
            2 / x

        # Any other type fails
        with self.assertRaises(TypeError):
            x / object()
        with self.assertRaises(TypeError):
            object() / x
