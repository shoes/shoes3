#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkprogressbaralt.h"

/* Private class member */
#define GTK_PROGRESS_BAR_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_PROGRESS_BAR_ALT, GtkProgressBar_AltPrivate))

typedef struct _GtkProgressBar_AltPrivate GtkProgressBar_AltPrivate;

struct _GtkProgressBar_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_progress_bar_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_progress_bar_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkProgressBar_Alt type and inherit from GtkProgressBar */
G_DEFINE_TYPE(GtkProgressBar_Alt, gtk_progress_bar_alt, GTK_TYPE_PROGRESS_BAR);

/* Initialize the GtkProgressBar_Alt class */
static void gtk_progress_bar_alt_class_init(GtkProgressBar_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_progress_bar_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_progress_bar_alt_get_preferred_height;

    /* Override GtkProgressBar methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkProgressBar_AltPrivate));
}

/* Initialize a new GtkProgressBar_Alt instance */
static void gtk_progress_bar_alt_init(GtkProgressBar_Alt *progressbarAlt) {
    /* This means that GtkProgressBar_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(progressbarAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkProgressBar_AltPrivate *priv = GTK_PROGRESS_BAR_ALT_PRIVATE(progressbarAlt);

}

/* Return a new GtkProgressBar_Alt cast to a GtkWidget */
GtkWidget *gtk_progress_bar_alt_new() {
    return GTK_WIDGET(g_object_new(gtk_progress_bar_alt_get_type(), NULL));
}

static void gtk_progress_bar_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void gtk_progress_bar_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}
// end of subclass fun

SHOES_CONTROL_REF shoes_native_progress(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_progress_bar_alt_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ref), _(msg));
    return ref;
}

double shoes_native_progress_get_fraction(SHOES_CONTROL_REF ref) {
    return gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(ref));
}

void shoes_native_progress_set_fraction(SHOES_CONTROL_REF ref, double perc) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ref), perc);
}

