#include "gshoes-ele.h"

/*
 * Thin GObject Wrapper for Shoes elements (widgets, textblocks...)

 * Private structure definition. 
 */
typedef struct {
  gchar *name;        // Private not needed?
  /* stuff */
} GshoesElePrivate;

/* 
 * forward definitions
 */
static void
gshoes_ele_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec);
static void
gshoes_ele_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec);
                                
struct _GshoesEle
{
  GObject parent_instance;

  gpointer element;       // It's a Ruby VALUE
  const gchar *name;      // for debugging help?
  
};

G_DEFINE_TYPE (GshoesEle, gshoes_ele, G_TYPE_OBJECT)

enum
{
  PROP_ELEMENT = 1,
  PROP_NAME,
  N_PROPERTIES
};

static GParamSpec *gshoes_ele_properties[N_PROPERTIES] = { NULL, };

static void
gshoes_ele_class_init (GshoesEleClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->set_property = gshoes_ele_set_property;
  object_class->get_property = gshoes_ele_get_property;
  
  gshoes_ele_properties[PROP_ELEMENT] =
    g_param_spec_pointer ("element",
                         "Element",
                         "Shoes drawable element/widget.",
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);
  gshoes_ele_properties[PROP_NAME] =
    g_param_spec_string ("name",
                         "Name",
                         "Layout name of the widget.",
                         NULL  /* default value */,
                         G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

  // need to define other properties here - which ones?  GtkWidgets? 
  
  g_object_class_install_properties (object_class,
                                     N_PROPERTIES,
                                     gshoes_ele_properties);
}

static void
gshoes_ele_init (GshoesEle *self)
{
  //GShoesElePrivate *priv = gshoes_ele_get_instance_private (self);

  /* initialize all public and private members to reasonable default values.
   * They are all automatically initialized to 0 to begin with. */
}

static void
gshoes_ele_set_property (GObject      *gobject,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  GshoesEle *self = GSHOES_ELE (gobject);
  switch (prop_id) {
    case PROP_ELEMENT:
      self->element = g_value_get_pointer (value);
      break;
    case PROP_NAME:
      self->name = g_value_get_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
   }
}

static void
gshoes_ele_get_property (GObject    *gobject,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  GshoesEle *self = GSHOES_ELE (gobject);
  switch (prop_id) {
    case PROP_ELEMENT:
      g_value_set_pointer (value, self->element);
      break;
    case PROP_NAME:
      g_value_set_string (value, self->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
  }
}

GshoesEle *gshoes_ele_new(GString *str, gpointer *ele) {
    return g_object_new (GSHOES_TYPE_ELE,
                       "name", str,
                       "element", ele,
                       NULL);
}

gpointer gshoes_ele_get_element(GshoesEle *gs) {
   g_return_val_if_fail (GSHOES_IS_ELE (gs), NULL);

  return gs->element;

}
