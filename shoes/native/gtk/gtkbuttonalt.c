#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/types/color.h"
#include "shoes/internal.h"

#include "gtkbuttonalt.h"
#include "gtklabelalt.h"

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

extern VALUE cImage;
extern VALUE cColor;

gboolean shoes_button_gtk_clicked(GtkButton *button, gpointer data) {
    VALUE self = (VALUE)data;
    shoes_control_send(self, s_click);
    return TRUE;
}

SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    char *fntstr = NULL;
    VALUE fgclr = Qnil; // Could be hex color or name
    VALUE icon = Qnil;
    //SHOES_CONTROL_REF ref = gtk_button_alt_new_with_label(_(msg));
    GtkWidget *glabel = NULL; 
    GtkWidget *gimage = NULL;
    SHOES_CONTROL_REF ref = gtk_button_alt_new();
    if (!NIL_P(shoes_hash_get(attr, rb_intern("font")))) {
      fntstr = RSTRING_PTR(shoes_hash_get(attr, rb_intern("font")));
      //fprintf(stderr, "%s\n", fntstr);
    } else {
      fntstr = "Arial";
    } 
    if (!NIL_P(shoes_hash_get(attr, rb_intern("stroke")))) {
      fgclr = shoes_hash_get(attr, rb_intern("stroke"));
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
        //TODO: Raise an error
      }
    }

    //glabel = gtk_label_new(NULL);
    glabel = gtklabel_alt_new();
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
    
    // Finally, we add the GtkLabel to the Gtk_Button 
    gtk_container_add ((GtkContainer *)ref, (GtkWidget *)glabel);
    if (gimage) {
      gtk_button_set_image((GtkButton *)ref, gimage);
      gtk_button_set_image_position((GtkButton *)ref, GTK_POS_RIGHT);
      gtk_button_set_always_show_image((GtkButton *)ref, TRUE);
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);

    return ref;
}
