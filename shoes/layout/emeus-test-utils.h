#pragma once

#include <glib.h>
#include <math.h>
#include <string.h>
#include <float.h>

#define emeus_fuzzy_equals(n1,n2,epsilon) \
  (((n1) > (n2) ? ((n1) - (n2)) : ((n2) - (n1))) < (epsilon))

#define emeus_assert_almost_equals(n1,n2) \
  G_STMT_START { \
    double __n1 = (n1), __n2 = (n2); \
    if (emeus_fuzzy_equals (__n1, __n2, DBL_EPSILON)) ; else { \
      g_assertion_message_cmpnum (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, \
                                  #n1 " == " #n2 " (+/- 1e-9)", \
                                  __n1, "==", __n2, 'f'); \
    } \
  } G_STMT_END
