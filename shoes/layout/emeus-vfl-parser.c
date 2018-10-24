// CJC: slightly modified from emeus/src.emeus-vfl-parser.c
#include "config.h"

#include "emeus-vfl-parser-private.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
typedef enum {
  VFL_HORIZONTAL,
  VFL_VERTICAL
} VflOrientation;

typedef struct {
  OperatorType relation;

  double constant;
  double multiplier;
  const char *subject;
  char *object;
  const char *attr;

  double priority;
} VflPredicate;

typedef struct {
  double size;
  bool is_set;
  bool is_default;
  bool is_predicate;
  VflPredicate predicate;
} VflSpacing;

typedef struct _VflView VflView;

struct _VflView
{
  char *name;

  /* Decides which attributes are admissible */
  VflOrientation orientation;

  /* A set of predicates, which will be used to
   * set up constraints
   */
  GArray *predicates;

  VflSpacing spacing;

  VflView *prev_view;
  VflView *next_view;
};

struct _VflParser
{
  char *buffer;
  gsize buffer_len;

  int error_offset;
  int error_range;

  int default_spacing[2];

  /* Set<name, double> */
  GHashTable *metrics_set;
  /* Set<name, widget> */
  GHashTable *views_set;

  const char *cursor;

  /* Decides which attributes are admissible */
  VflOrientation orientation;

  VflView *leading_super;
  VflView *trailing_super;

  VflView *current_view;
  VflView *views;
};

GQuark
vfl_error_quark (void)
{
  return g_quark_from_static_string ("vfl-error-quark");
}

VflParser *
vfl_parser_new (int hspacing,
                int vspacing,
                GHashTable *metrics,
                GHashTable *views)
{
  VflParser *res = g_slice_new0 (VflParser);

  res->default_spacing[VFL_HORIZONTAL] = hspacing < 0 ? 8 : hspacing;
  res->default_spacing[VFL_VERTICAL] = vspacing < 0 ? 8 : vspacing;

  res->orientation = VFL_HORIZONTAL;

  res->metrics_set = metrics;
  res->views_set = views;

  return res;
}

void
vfl_parser_set_default_spacing (VflParser *parser,
                                int hspacing,
                                int vspacing)
{
  parser->default_spacing[VFL_HORIZONTAL] = hspacing;
  parser->default_spacing[VFL_VERTICAL] = vspacing;
}

void
vfl_parser_set_metrics (VflParser *parser,
                        GHashTable *metrics)
{
  parser->metrics_set = metrics;
}

void
vfl_parser_set_views (VflParser *parser,
                      GHashTable *views)
{
  parser->views_set = views;
}

static int
get_default_spacing (VflParser *parser)
{
  return parser->default_spacing[parser->orientation];
}

/* Default attributes, if unnamed, depending on the orientation */
static const char *default_attribute[2] = {
  [VFL_HORIZONTAL] = "width",
  [VFL_VERTICAL] = "height",
};

static bool
parse_relation (const char *str,
                OperatorType *relation,
                char **endptr,
                GError **error)
{
  const char *cur = str;

  if (*cur == '=')
    {
      cur += 1;

      if (*cur == '=')
        {
          *relation = OPERATOR_TYPE_EQ;
          *endptr = (char *) cur + 1;
          return true;
        }

      goto out;
    }
  else if (*cur == '>')
    {
      cur += 1;

      if (*cur == '=')
        {
          *relation = OPERATOR_TYPE_GE;
          *endptr = (char *) cur + 1;
          return true;
        }

      goto out;
    }
  else if (*cur == '<')
    {
      cur += 1;

      if (*cur == '=')
        {
          *relation = OPERATOR_TYPE_LE;
          *endptr = (char *) cur + 1;
          return true;
        }

      goto out;
    }

out:
  g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_RELATION,
               "Unknown relation; must be one of '==', '>=', or '<='");
  return false;
}

static bool
has_metric (VflParser *parser,
            const char *name)
{
  if (parser->metrics_set == NULL)
    return false;

  return g_hash_table_contains (parser->metrics_set, name);
}

static bool
has_view (VflParser *parser,
          const char *name)
{
  if (parser->views_set == NULL)
    return false;

  if (!g_hash_table_contains (parser->views_set, name))
    return false;

  return g_hash_table_lookup (parser->views_set, name) != NULL;
}

/* Valid attributes */
static const struct {
  int len;
  const char *name;
} valid_attributes[] = {
  { 5, "width" },
  { 6, "height" },
  { 7, "centerX" },
  { 7, "centerY" },
  { 3, "top" },
  { 6, "bottom" },
  { 4, "left" },
  { 5, "right" },
  { 5, "start" },
  { 3, "end" },
  { 8, "baseline" }
};

static char *
get_offset_to (const char *str,
               const char *tokens)
{
  char *offset = NULL;
  int n_tokens = strlen (tokens);

  for (int i = 0; i < n_tokens; i++)
    {
      if ((offset = strchr (str, tokens[i])) != NULL)
        break;
    }

  return offset;
}

static bool
parse_predicate (VflParser *parser,
                 const char *cursor,
                 VflPredicate *predicate,
                 char **endptr,
                 GError **error)
{
  VflOrientation orientation = parser->orientation;
  const char *end = cursor;

  predicate->object = NULL;
  predicate->multiplier = 1.0;

  /*         <predicate> = (<relation>)? (<objectOfPredicate>) ('.'<attribute>)? (<operator>)? ('@'<priority>)?
   *          <relation> = '==' | '<=' | '>='
   * <objectOfPredicate> = <constant> | <viewName>
   *          <constant> = <number> | <metricName>
   *          <viewName> = [A-Za-z_]([A-Za-z0-9_]*)
   *        <metricName> = [A-Za-z_]([A-Za-z0-9_]*)
   *          <operator> = (['*'|'/']<positiveNumber>)? (['+'|'-']<positiveNumber>)?
   *          <priority> = <positiveNumber> | 'weak' | 'medium' | 'strong' | 'required'
   */

  /* Parse relation */
  if (*end == '=' || *end == '>' || *end == '<')
    {
      OperatorType relation;
      char *tmp;

      if (!parse_relation (end, &relation, &tmp, error))
        {
          parser->error_offset = end - parser->cursor;
          parser->error_range = 0;
          return false;
        }

      predicate->relation = relation;

      end = tmp;
    }
  else
    predicate->relation = OPERATOR_TYPE_EQ;

  /* Parse object of predicate */
  if (g_ascii_isdigit (*end))
    {
      char *tmp;

      /* <constant> */
      predicate->object = NULL;
      predicate->attr = default_attribute[orientation];
      predicate->constant = g_ascii_strtod (end, &tmp);

      end = tmp;
    }
  else if (g_ascii_isalpha (*end) || *end == '_')
    {
      const char *name_start = end;

      while (g_ascii_isalnum (*end) || *end == '_')
        end += 1;

      char *name = g_strndup (name_start, end - name_start);

      /* We only accept view names if the subject of the predicate
       * is a view, i.e. we do not allow view names inside a spacing
       * predicate
       */
      if (predicate->subject == NULL)
        {
          if (parser->metrics_set == NULL || !has_metric (parser, name))
            {
              parser->error_offset = name_start - parser->cursor;
              parser->error_range = end - name_start;
              g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_METRIC,
                           "Unable to find metric with name '%s'", name);
              g_free (name);
              return false;
            }

          double *val = g_hash_table_lookup (parser->metrics_set, name);

          predicate->object = NULL;
          predicate->attr = default_attribute[orientation];
          predicate->constant = *val;

          g_free (name);

          goto parse_operators;
        }

      if (has_metric (parser, name))
        {
          double *val = g_hash_table_lookup (parser->metrics_set, name);

          predicate->object = NULL;
          predicate->attr = default_attribute[orientation];
          predicate->constant = *val;

          g_free (name);

          goto parse_operators;
        }

      if (has_view (parser, name))
        {
          /* Transfer name's ownership to the predicate */
          predicate->object = name;
          predicate->attr = default_attribute[orientation];
          predicate->constant = 0;

          goto parse_attribute;
        }

      parser->error_offset = name_start - parser->cursor;
      parser->error_range = end - name_start;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_VIEW,
                   "Unable to find view with name '%s'", name);
      g_free (name);
      return false;
    }
  else
    {
      parser->error_offset = end - parser->cursor;
      parser->error_range = 0;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "Expected constant, view name, or metric");
      return false;
    }

parse_attribute:
  if (*end == '.')
    {
      end += 1;
      predicate->attr = NULL;

      for (int i = 0; i < G_N_ELEMENTS (valid_attributes); i++)
        {
          if (g_ascii_strncasecmp (valid_attributes[i].name, end, valid_attributes[i].len) == 0)
            {
              predicate->attr = valid_attributes[i].name;
              end += valid_attributes[i].len;
            }
        }

      if (predicate->attr == NULL)
        {
          char *range_end = get_offset_to (end, "*/+-@,)]");

          if (range_end != NULL)
            parser->error_range = range_end - end - 1;
          else
            parser->error_range = 0;

          g_free (predicate->object);

          parser->error_offset = end - parser->cursor;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_ATTRIBUTE,
                       "Attribute must be on one of 'width', 'height', "
                       "'centerX', 'centerY', 'top', 'bottom', "
                       "'left', 'right', 'start', 'end', 'baseline'");
          return false;
        }
    }

parse_operators:
  /* Parse multiplier operator */
  while (g_ascii_isspace (*end))
    end += 1;

  if ((*end == '*') || (*end == '/'))
    {
      double multiplier;
      const char *operator;

      operator = end;
      end += 1;

      while (g_ascii_isspace (*end))
        end += 1;

      if (g_ascii_isdigit (*end))
        {
          char *tmp;

          multiplier = g_ascii_strtod (end, &tmp);
          end = tmp;
        }
      else
        {
          g_free (predicate->object);

          parser->error_offset = end - parser->cursor;
          parser->error_range = 0;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                       "Expected a positive number as a multiplier");
          return false;
        }

      if (predicate->object != NULL)
        {
          if (*operator == '*')
            predicate->multiplier = multiplier;
          else
            predicate->multiplier = 1.0 / multiplier;
        }
      else
        {
          /* If the subject is a constant then apply multiplier directly */
          if (*operator == '*')
            predicate->constant *= multiplier;
          else
            predicate->constant *= 1.0 / multiplier;
        }
    }

  /* Parse constant operator */
  while (g_ascii_isspace (*end))
    end += 1;

  if ((*end == '+') || (*end == '-'))
    {
      double constant;
      const char *operator;

      operator = end;
      end += 1;

      while (g_ascii_isspace (*end))
        end += 1;

      if (g_ascii_isdigit (*end))
        {
          char *tmp;

          constant = g_ascii_strtod (end, &tmp);
          end = tmp;
        }
      else
        {
          g_free (predicate->object);

          parser->error_offset = end - parser->cursor;
          parser->error_range = 0;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                       "Expected positive number as a constant");
          return false;
        }

      if (*operator == '+')
        predicate->constant += constant;
      else
        predicate->constant += -1.0 * constant;
    }

  /* Parse priority */
  if (*end == '@')
    {
      double priority;
      end += 1;

      if (g_ascii_isdigit (*end))
        {
          char *tmp;

          priority = g_ascii_strtod (end, &tmp);
          end = tmp;
        }
      else if (strncmp (end, "weak", 4) == 0)
        {
          priority = STRENGTH_WEAK;
          end += 4;
        }
      else if (strncmp (end, "medium", 6) == 0)
        {
          priority = STRENGTH_MEDIUM;
          end += 6;
        }
      else if (strncmp (end, "strong", 6) == 0)
        {
          priority = STRENGTH_STRONG;
          end += 6;
        }
      else if (strncmp (end, "required", 8) == 0)
        {
          priority = STRENGTH_REQUIRED;
          end += 8;
        }
      else
        {
          char *range_end = get_offset_to (end, ",)]");

          g_free (predicate->object);

          if (range_end != NULL)
            parser->error_range = range_end - end - 1;
          else
            parser->error_range = 0;

          parser->error_offset = end - parser->cursor;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_PRIORITY,
                       "Priority must be a positive number or one of "
                       "'weak', 'medium', 'strong', and 'required'");
          return false;
        }

      predicate->priority = priority;
    }
  else
    predicate->priority = STRENGTH_REQUIRED;

  if (endptr != NULL)
    *endptr = (char *) end;

  return true;
}

static bool
parse_view (VflParser *parser,
            const char *cursor,
            VflView *view,
            char **endptr,
            GError **error)
{
  const char *end = cursor;

  /*     <view> = '[' <viewName> (<predicateListWithParens>)? ']'
   * <viewName> = [A-Za-z_]([A-Za-z0-9_]+)
   */

  g_assert (*end == '[');

  /* Skip '[' */
  end += 1;

  if (!(g_ascii_isalpha (*end) || *end == '_'))
    {
      parser->error_offset = end - parser->cursor;
      parser->error_range = 0;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_VIEW,
                   "View identifiers must be valid C identifiers");
      return false;
    }

  while (g_ascii_isalnum (*end))
    end += 1;

  if (*end == '\0')
    {
      parser->error_offset = end - parser->cursor;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "A view must end with ']'");
      return false;
    }

  char *name = g_strndup (cursor + 1, end - cursor - 1);
  if (!has_view (parser, name))
    {
      parser->error_offset = (cursor + 1) - parser->cursor;
      parser->error_range = end - cursor - 1;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_VIEW,
                   "Unable to find view with name '%s'", name);
      g_free (name);
      return false;
    }

  view->name = name;
  view->predicates = g_array_new (FALSE, FALSE, sizeof (VflPredicate));

  if (*end == ']')
    {
      if (endptr != NULL)
        *endptr = (char *) end + 1;

      return true;
    }

  /* <predicateListWithParens> = '(' <predicate> (',' <predicate>)* ')' */
  if (*end != '(')
    {
      parser->error_offset = end - parser->cursor;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "A predicate must follow a view name");
      return false;
    }

  end += 1;

  while (*end != '\0')
    {
      VflPredicate cur_predicate;
      char *tmp;

      if (*end == ']' || *end == '\0')
        {
          parser->error_offset = end - parser->cursor;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "A predicate on a view must end with ')'");
          return false;
        }

      memset (&cur_predicate, 0, sizeof (VflPredicate));

      cur_predicate.subject = view->name;
      if (!parse_predicate (parser, end, &cur_predicate, &tmp, error))
        return false;

      end = tmp;

#ifdef EMEUS_ENABLE_DEBUG
      g_debug ("*** Found predicate: %s.%s %s %g %s\n",
               cur_predicate.object != NULL ? cur_predicate.object : view->name,
               cur_predicate.attr,
               cur_predicate.relation == OPERATOR_TYPE_EQ ? "==" :
               cur_predicate.relation == OPERATOR_TYPE_LE ? "<=" :
               cur_predicate.relation == OPERATOR_TYPE_GE ? ">=" :
               "unknown relation",
               cur_predicate.constant,
               cur_predicate.priority == STRENGTH_WEAK ? "weak" :
               cur_predicate.priority == STRENGTH_MEDIUM ? "medium" :
               cur_predicate.priority == STRENGTH_STRONG ? "strong" :
               cur_predicate.priority == STRENGTH_REQUIRED ? "required" :
               "unknown strength");
#endif

      g_array_append_val (view->predicates, cur_predicate);

      /* If the predicate is a list, iterate again */
      if (*end == ',')
        {
          end += 1;
          continue;
        }

      /* We reached the end of the predicate */
      if (*end == ')')
        {
          end += 1;
          break;
        }

      parser->error_offset = end - parser->cursor;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "Expected ')' at the end of a predicate, not '%c'", *end);
      return false;
    }

  if (*end != ']')
    {
      parser->error_offset = end - parser->cursor;
      g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                   "Expected ']' at the end of a view, not '%c'", *end);
      return false;
    }

  if (endptr != NULL)
    *endptr = (char *) end + 1;

  return true;
}

static void
vfl_view_free (VflView *view)
{
  if (view == NULL)
    return;

  g_free (view->name);

  if (view->predicates != NULL)
    {
      for (int i = 0; i < view->predicates->len; i++)
        {
          VflPredicate *p = &g_array_index (view->predicates, VflPredicate, i);

          g_free (p->object);
        }

      g_array_free (view->predicates, TRUE);
      view->predicates = NULL;
    }

  view->prev_view = NULL;
  view->next_view = NULL;

  g_slice_free (VflView, view);
}

static void
vfl_parser_clear (VflParser *parser)
{
  parser->error_offset = 0;
  parser->error_range = 0;

  VflView *iter = parser->views; 
  while (iter != NULL)
    {
      VflView *next = iter->next_view;

      vfl_view_free (iter);

      iter = next;
    }

  parser->views = NULL;
  parser->current_view = NULL;
  parser->leading_super = NULL;
  parser->trailing_super = NULL;

  parser->cursor = NULL;

  g_free (parser->buffer);
  parser->buffer_len = 0;
}

void
vfl_parser_free (VflParser *parser)
{
  if (parser == NULL)
    return;

  vfl_parser_clear (parser);

  g_slice_free (VflParser, parser);
}

bool
vfl_parser_parse_line (VflParser *parser,
                       const char *buffer,
                       gssize len,
                       GError **error)
{
  vfl_parser_clear (parser);

  if (len > 0)
    {
      parser->buffer = g_strndup (buffer, len);
      parser->buffer_len = len;
    }
  else
    {
      parser->buffer = g_strdup (buffer);
      parser->buffer_len = strlen (buffer);
    }

  parser->cursor = parser->buffer;

  const char *cur = parser->cursor;

  /* Skip leading whitespace */
  while (g_ascii_isspace (*cur))
    cur += 1;

  /* Check orientation; if none is specified, then we assume horizontal */
  parser->orientation = VFL_HORIZONTAL;
  if (*cur == 'H')
    {
      cur += 1;

      if (*cur != ':')
        {
          parser->error_offset = cur - parser->cursor;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                       "Expected ':' after horizontal orientation");
          return false;
        }

      parser->orientation = VFL_HORIZONTAL;
      cur += 1;
    }
  else if (*cur == 'V')
    {
      cur += 1;

      if (*cur != ':')
        {
          parser->error_offset = cur - parser->cursor;
          g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                       "Expected ':' after vertical orientation");
          return false;
        }

      parser->orientation = VFL_VERTICAL;
      cur += 1;
    }

  while (*cur != '\0')
    {
      /* Super-view */
      if (*cur == '|')
        {
          if (parser->views == NULL && parser->leading_super == NULL)
            {
              parser->leading_super = g_slice_new0 (VflView);

              parser->leading_super->name = g_strdup ("super");
              parser->leading_super->orientation = parser->orientation;

              parser->current_view = parser->leading_super;
              parser->views = parser->leading_super;
            }
          else if (parser->trailing_super == NULL)
            {
              parser->trailing_super = g_slice_new0 (VflView);

              parser->trailing_super->name = g_strdup ("super");
              parser->trailing_super->orientation = parser->orientation;

              parser->current_view->next_view = parser->trailing_super;
              parser->trailing_super->prev_view = parser->current_view;

              parser->current_view = parser->trailing_super;

              /* If we reached the second '|' then we completed a line
               * of layout, and we can stop
               */
              break;
            }
          else
            {
              parser->error_offset = cur - parser->cursor;
              g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                           "Super views can only appear at the beginning "
                           "and end of the layout, and not multiple times");
              return false;
            }

          cur += 1;

          continue;
        }

      /* Spacing */
      if (*cur == '-')
        {
          if (*(cur + 1) == '\0')
            {
              parser->error_offset = cur - parser->cursor;
              g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                           "Unterminated spacing");
              return false;
            }

          if (parser->current_view == NULL)
            {
              parser->error_offset = cur - parser->cursor;
              g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                           "Spacing cannot be set without a view");
              return false;
            }

          if (*(cur + 1) == '|' || *(cur + 1) == '[')
            {
              VflSpacing *spacing = &(parser->current_view->spacing);

              /* Default spacer */
              spacing->is_set = true;
              spacing->is_default = true;
              spacing->is_predicate = false;
              spacing->size = 0;

              cur += 1;

              continue;
            }
          else if (*(cur + 1) == '(')
            {
              VflPredicate *predicate;
              VflSpacing *spacing;
              char *tmp;

              /* Predicate */
              cur += 1;

              spacing = &(parser->current_view->spacing);
              spacing->is_set = true;
              spacing->is_default = false;
              spacing->is_predicate = true;
              spacing->size = 0;

              /* Spacing predicates have no subject */
              predicate = &(spacing->predicate);
              predicate->subject = NULL;

              cur += 1;
              if (!parse_predicate (parser, cur, predicate, &tmp, error))
                return false;

              if (*tmp != ')')
                {
                  g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                                "Expected ')' at the end of a predicate, not '%c'", *tmp);
                  return false;
                }

              cur = tmp + 1;
              if (*cur != '-')
                {
                  g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                               "Explicit spacing must be followed by '-'");
                  return false;
                }

              cur += 1;

              continue;
            }
          else if (g_ascii_isdigit (*(cur + 1)))
            {
              VflSpacing *spacing;
              char *tmp;

              /* Explicit spacing */
              cur += 1;

              spacing = &(parser->current_view->spacing);
              spacing->is_set = true;
              spacing->is_default = false;
              spacing->is_predicate = false;
              spacing->size = g_ascii_strtod (cur, &tmp);

              if (tmp == cur)
                {
                  parser->error_offset = cur - parser->cursor;
                  g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                               "Spacing must be a number");
                  return false;
                }

              if (*tmp != '-')
                {
                  parser->error_offset = cur - parser->cursor;
                  parser->error_range = tmp - cur;
                  g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                               "Explicit spacing must be followed by '-'");
                  return false;
                }

              cur = tmp + 1;

              continue;
            }
          else
            {
              parser->error_offset = cur - parser->cursor;
              g_set_error (error, VFL_ERROR, VFL_ERROR_INVALID_SYMBOL,
                           "Spacing can either be '-' or a number");
              return false;
            }
        }

      if (*cur == '[')
        {
          VflView *view = g_slice_new0 (VflView);
          char *tmp;

          view->orientation = parser->orientation;

          if (!parse_view (parser, cur, view, &tmp, error))
            {
              vfl_view_free (view);
              return false;
            }

          cur = tmp;

          if (parser->views == NULL)
            parser->views = view;

          view->prev_view = parser->current_view;

          if (parser->current_view != NULL)
            parser->current_view->next_view = view;

          parser->current_view = view;

          continue;
        }

      cur += 1;
    }

  return true;
}

VflConstraint *
vfl_parser_get_constraints (VflParser *parser,
                            int *n_constraints)
{
  GArray *constraints;
  VflView *iter;

  constraints = g_array_new (FALSE, FALSE, sizeof (VflConstraint));

  iter = parser->views;
  while (iter != NULL)
    {
      VflConstraint c;

      if (iter->predicates != NULL)
        {
          for (int i = 0; i < iter->predicates->len; i++)
            {
              const VflPredicate *p = &g_array_index (iter->predicates, VflPredicate, i);

              c.view1 = iter->name;
              c.attr1 = iter->orientation == VFL_HORIZONTAL ? "width" : "height";
              if (p->object != NULL)
                {
                  c.view2 = p->object;
                  c.attr2 = p->attr;
                }
              else
                {
                  c.view2 = NULL;
                  c.attr2 = NULL;
                }
              c.relation = p->relation;
              c.constant = p->constant;
              c.multiplier = p->multiplier;
              c.strength = p->priority;

              g_array_append_val (constraints, c);
            }
        }

      if (iter->spacing.is_set)
        {
          c.view1 = iter->name;

          /* If this is the first view, we need to anchor the leading edge */
          if (iter == parser->leading_super)
            c.attr1 = iter->orientation == VFL_HORIZONTAL ? "start" : "top";
          else
            c.attr1 = iter->orientation == VFL_HORIZONTAL ? "end" : "bottom";

          c.view2 = iter->next_view != NULL ? iter->next_view->name : "super";

          if (iter == parser->trailing_super || iter->next_view == parser->trailing_super)
            c.attr2 = iter->orientation == VFL_HORIZONTAL ? "end" : "bottom";
          else
            c.attr2 = iter->orientation == VFL_HORIZONTAL ? "start" : "top";

          if (iter->spacing.is_predicate)
            {
              const VflPredicate *p = &(iter->spacing.predicate);

              c.constant = p->constant * -1.0;
              c.relation = p->relation;
              c.strength = p->priority;
            }
          else if (iter->spacing.is_default)
            {
              c.constant = get_default_spacing (parser) * -1.0;
              c.relation = OPERATOR_TYPE_EQ;
              c.strength = STRENGTH_REQUIRED;
            }
          else
            {
              c.constant = iter->spacing.size * -1.0;
              c.relation = OPERATOR_TYPE_EQ;
              c.strength = STRENGTH_REQUIRED;
            }

          c.multiplier = 1.0;

          g_array_append_val (constraints, c);
        }
      else if (iter->next_view != NULL)
        {
          c.view1 = iter->name;

          /* If this is the first view, we need to anchor the leading edge */
          if (iter == parser->leading_super)
            c.attr1 = iter->orientation == VFL_HORIZONTAL ? "start" : "top";
          else
            c.attr1 = iter->orientation == VFL_HORIZONTAL ? "end" : "bottom";

          c.relation = OPERATOR_TYPE_EQ;

          c.view2 = iter->next_view->name;

          if (iter->next_view == parser->trailing_super)
            c.attr2 = iter->orientation == VFL_HORIZONTAL ? "end" : "bottom";
          else
            c.attr2 = iter->orientation == VFL_HORIZONTAL ? "start" : "top";

          c.constant = 0.0;
          c.multiplier = 1.0;
          c.strength = STRENGTH_REQUIRED;

          g_array_append_val (constraints, c);
        }

      iter = iter->next_view;
    }

  if (n_constraints != NULL)
    *n_constraints = constraints->len;

//#ifdef EMEUS_ENABLE_DEBUG
#if 0
  for (int i = 0; i < constraints->len; i++)
    {
      const VflConstraint *c = &g_array_index (constraints, VflConstraint, i);

      fprintf(stderr,"{\n"
               "  .view1: '%s',\n"
               "  .attr1: '%s',\n"
               "  .relation: '%d',\n"
               "  .view2: '%s',\n"
               "  .attr2: '%s',\n"
               "  .constant: %g,\n"
               "  .multiplier: %g,\n"
               "  .strength: %g\n"
               "}\n",
               c->view1,
               c->attr1,
               c->relation,
               c->view2 != NULL ? c->view2 : "none",
               c->attr2 != NULL ? c->attr2 : "none",
               c->constant,
               c->multiplier,
               c->strength);
    }
#endif

  return (VflConstraint *) g_array_free (constraints, FALSE);
}

int
vfl_parser_get_error_offset (VflParser *parser)
{
  return parser->error_offset;
}

int
vfl_parser_get_error_range (VflParser *parser)
{
  return parser->error_range;
}
