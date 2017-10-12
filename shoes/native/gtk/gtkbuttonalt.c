#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/types/color.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkbuttonalt.h"

/* Private class member */
#define GTK_BUTTON_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_BUTTON_ALT, GtkButton_AltPrivate))

typedef struct _GtkButton_AltPrivate GtkButton_AltPrivate;

struct _GtkButton_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_button_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_button_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkButton_Alt type and inherit from GtkButton */
G_DEFINE_TYPE(GtkButton_Alt, gtk_button_alt, GTK_TYPE_BUTTON);

/* Initialize the GtkButton_Alt class */
static void gtk_button_alt_class_init(GtkButton_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_button_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_button_alt_get_preferred_height;

    /* Override GtkButton methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkButton_AltPrivate));
}

/* Initialize a new GtkButton_Alt instance */
static void gtk_button_alt_init(GtkButton_Alt *buttontAlt) {
    /* This means that GtkButton_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(buttontAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkButton_AltPrivate *priv = GTK_BUTTON_ALT_PRIVATE(buttontAlt);

}

/* Return a new GtkButton_Alt cast to a GtkWidget */
GtkWidget *gtk_button_alt_new() {
    return GTK_WIDGET(g_object_new(gtk_button_alt_get_type(), NULL));
}

GtkWidget *gtk_button_alt_new_with_label(const gchar *label) {
  
    return GTK_WIDGET(g_object_new (gtk_button_alt_get_type(), "label", label, NULL));
}

static void gtk_button_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void gtk_button_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}
// end gtk subclass fun

extern VALUE cImage;
extern VALUE cColor;
GtkWidget *shoes_gtk_button_icon_box(GtkWidget *glable, GtkWidget *gimage, char *icon_pos);

gboolean shoes_button_gtk_clicked(GtkButton *button, gpointer data) {
    VALUE self = (VALUE)data;
    shoes_control_send(self, s_click);
    return TRUE;
}

SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    char *fntstr = NULL;
    VALUE fgclr = NULL; // Could be hex color or name
    VALUE icon = NULL;
    //SHOES_CONTROL_REF ref = gtk_button_alt_new_with_label(_(msg));
    GtkWidget *glabel = NULL; 
    GtkWidget *gimage = NULL;
    char *icon_pos = NULL;
    SHOES_CONTROL_REF ref = gtk_button_alt_new();
    if (msg && (strlen(msg) > 0)) {
      if (!NIL_P(shoes_hash_get(attr, rb_intern("font")))) {
        fntstr = RSTRING_PTR(shoes_hash_get(attr, rb_intern("font")));
        //fprintf(stderr, "%s\n", fntstr);
      } else {
        fntstr = "Arial";
      } 
      if (!NIL_P(shoes_hash_get(attr, rb_intern("stroke")))) {
        fgclr = shoes_hash_get(attr, rb_intern("stroke"));
      }
  
      glabel = gtk_label_new(NULL);
      PangoAttribute *pattr = NULL;
      PangoAttrList *plist = pango_attr_list_new ();
      PangoFontDescription *fontdesc = NULL;
      fontdesc = pango_font_description_from_string(fntstr);
      pattr = pango_attr_font_desc_new(fontdesc);
      pango_attr_list_insert (plist, pattr);
      // deal with stroke attr here -- add to the plist
      if (! NIL_P(fgclr)) {
        PangoAttribute *pfgcolor = NULL;
        if (TYPE(fgclr) == T_STRING) 
          fgclr = shoes_color_parse(cColor, fgclr);  // convert string to cColor
        if (rb_obj_is_kind_of(fgclr, cColor)) 
        { 
          shoes_color *color; 
          Data_Get_Struct(fgclr, shoes_color, color); 
          guint16 red = color->r * 65535;
          guint16 green = color->g * 65535;
          guint16 blue = color->b * 65535;
          //gcolor.alpha = color->a * 255;
          pfgcolor = pango_attr_foreground_new (red, green, blue);
          pango_attr_list_insert (plist, pfgcolor);
        }
      }
      gtk_label_set_attributes((GtkLabel *)glabel, plist);
      gtk_label_set_text((GtkLabel *)glabel, msg);
    }
    
    if (! NIL_P(shoes_hash_get(attr, rb_intern("icon")))) {
      VALUE image = shoes_hash_get(attr, rb_intern("icon"));
      if (rb_obj_is_kind_of(image, cImage)) {
        //TODO: turn a Shoes cImage into a GtkImage
      } else if (TYPE(image) == T_STRING) {
        char *iconp = RSTRING_PTR(image);
        gimage = (GtkWidget *)gtk_image_new_from_file(iconp);
        //fprintf(stderr, "%s\n", iconp);
      } else  {
        //TODO: Raise an error is not advised 
      }
      // check for icon positioning args here
      VALUE iconpv =  shoes_hash_get(attr, rb_intern("icon_pos"));
      if (! NIL_P(iconpv)) {
        icon_pos = RSTRING_PTR(iconpv);
      }
    }
    // now we need to get around a bug/oversight/untested feature
    // in gtk3 if we have both a title and a image
    if (glabel && gimage ) {
      // call for special sauce
      GtkWidget *icon_box = shoes_gtk_button_icon_box(glabel, gimage, icon_pos);
      if (icon_box) gtk_container_add((GtkContainer *)ref, icon_box);
    } else if (glabel) {
      // Finally, we add the GtkLabel to the Gtk_Button 
      gtk_container_add ((GtkContainer *)ref, (GtkWidget *)glabel);
    } else if (gimage) {
      gtk_button_set_image((GtkButton *) ref, gimage);
      gtk_button_set_image_position((GtkButton *) ref, GTK_POS_RIGHT);
      gtk_button_set_always_show_image((GtkButton *) ref, TRUE);
    } else {
      // button without label or image
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);

    return ref;
}

GtkWidget *shoes_gtk_button_icon_box(GtkWidget *glabel, GtkWidget *gimage, char *icon_pos)
{
  GtkWidget *box;
  //printf("special sauce needed\n");
  int pos = 0;
  if (icon_pos == NULL)
    pos = 1;
  else if (strcmp(icon_pos, "left") == 0)
    pos = 1;
  else if (strcmp(icon_pos, "right") == 0)
    pos = 2;
  else if (strcmp(icon_pos, "top") == 0)
    pos = 3;
  else if (strcmp(icon_pos,"bottom") == 0)
    pos = 4;
  else 
    pos = 1;
  if (pos < 3) {
    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_container_set_border_width (GTK_CONTAINER (box), 2);
    if (pos == 1) {
      gtk_box_pack_start (GTK_BOX (box), gimage, FALSE, FALSE, 3);
      gtk_box_pack_start (GTK_BOX (box), glabel, FALSE, FALSE, 3);
    } else {
      gtk_box_pack_start (GTK_BOX (box), glabel, FALSE, FALSE, 3);
      gtk_box_pack_start (GTK_BOX (box), gimage, FALSE, FALSE, 3);
    }
  } else {
    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_set_border_width (GTK_CONTAINER (box), 2);
    if (pos == 3) {
      gtk_box_pack_start (GTK_BOX (box), gimage, FALSE, FALSE, 3);
      gtk_box_pack_start (GTK_BOX (box), glabel, FALSE, FALSE, 3);
    } else {
      gtk_box_pack_start (GTK_BOX (box), glabel, FALSE, FALSE, 3);
      gtk_box_pack_start (GTK_BOX (box), gimage, FALSE, FALSE, 3);
    }
  }
  return box;
}
