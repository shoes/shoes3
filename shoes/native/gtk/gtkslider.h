#ifndef SHOES_GTK_SLIDER_H
#define SHOES_GTK_SLIDER_H

SHOES_CONTROL_REF shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
double shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref);
void shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc);
#endif
