from __future__ import print_function, unicode_literals, absolute_import, division


class CassowaryException(Exception):
    pass


class InternalError(CassowaryException):
    pass


class ConstraintNotFound(CassowaryException):
    pass


class RequiredFailure(CassowaryException):
    pass
