#pragma once

#include <glib.h>
#include "emeus-types-private.h"

G_BEGIN_DECLS

#define VFL_ERROR (vfl_error_quark ())

typedef enum {
  VFL_ERROR_INVALID_SYMBOL,
  VFL_ERROR_INVALID_ATTRIBUTE,
  VFL_ERROR_INVALID_VIEW,
  VFL_ERROR_INVALID_METRIC,
  VFL_ERROR_INVALID_PRIORITY,
  VFL_ERROR_INVALID_RELATION
} VflError;

typedef struct _VflParser       VflParser;

typedef struct {
  const char *view1;
  const char *attr1;
  OperatorType relation;
  const char *view2;
  const char *attr2;
  double constant;
  double multiplier;
  double strength;
} VflConstraint;

GQuark vfl_error_quark (void);

VflParser *vfl_parser_new (int hspacing,
                           int vspacing,
                           GHashTable *metrics,
                           GHashTable *views);
void vfl_parser_free (VflParser *parser);

void vfl_parser_set_default_spacing (VflParser *parser,
                                     int hspacing,
                                     int vspacing);

void vfl_parser_set_metrics (VflParser *parser,
                             GHashTable *metrics);
void vfl_parser_set_views (VflParser *parser,
                           GHashTable *views);

bool vfl_parser_parse_line (VflParser *parser,
                            const char *line,
                            gssize len,
                            GError **error);

int vfl_parser_get_error_offset (VflParser *parser);
int vfl_parser_get_error_range (VflParser *parser);

VflConstraint *vfl_parser_get_constraints (VflParser *parser,
                                           int *n_constraints);

G_END_DECLS
