#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkslider.h"

extern void shoes_widget_changed(GtkWidget *ref, gpointer data);

SHOES_CONTROL_REF shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0., 1., 0.01);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_scale_set_draw_value(GTK_SCALE(ref), FALSE);
    g_signal_connect(G_OBJECT(ref), "value-changed",
                     G_CALLBACK(shoes_widget_changed), (gpointer)self);
    return ref;
}

double shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref) {
    return gtk_range_get_value(GTK_RANGE(ref));
}

void shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc) {
    gtk_range_set_value(GTK_RANGE(ref), perc);
}
