#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/internal.h"
#include "shoes/native/native.h"
#include "shoes/native/gtk/gtkvideo.h"
#include "shoes/native/gtk/gtkcss.h"
#include "shoes/types/color.h"
#include "shoes/types/video.h"

void surface_on_realize(SHOES_CONTROL_REF ref, gpointer data) {
    VALUE rbvideo = (VALUE)data;
    shoes_video *video;
    Data_Get_Struct(rbvideo, shoes_video, video);
    video->realized = 1;
}

// SHOES_SURFACE_REF and SHOES_CONTROL_REF expands the same : GtkWidget *
// ref in shoes_video struct was a SHOES_CONTROL_REF anyway
SHOES_CONTROL_REF shoes_native_surface_new(VALUE attr, VALUE video) {
    SHOES_CONTROL_REF da = gtk_drawing_area_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(da), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_widget_set_size_request(da, NUM2INT(ATTR(attr, width)), NUM2INT(ATTR(attr, height)));

    VALUE uc = Qnil;
    if (!NIL_P(attr)) uc = ATTR(attr, bg_color);

    // DONE: (better with GtkStyleProvider)
#if 1
    if (! NIL_P(uc)) {
#ifdef NEW_MACRO_COLOR
        Get_TypedStruct2(uc, shoes_color, col);
#else
        shoes_color *col;
        Data_Get_Struct(uc, shoes_color, col);
#endif
        GtkCssProvider *provider;
        GtkStyleContext *context;
        char new_css[100]; 
        sprintf(new_css, "GtkDrawingArea {\n background-color: rgb(%d,%d,%d);\n}\n", 
            col->r, col->g, col->b);
        provider = gtk_css_provider_new ();
        g_signal_connect(G_OBJECT(provider), "parsing-error",
                       G_CALLBACK(shoes_css_parse_error),
                       (gpointer)NULL);
        gtk_css_provider_load_from_data(provider, new_css, -1, NULL);
        context = gtk_widget_get_style_context (da);
        gtk_style_context_add_provider (context,
              GTK_STYLE_PROVIDER (provider),
              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
        // check what's really in provider ?
        //printf("provider has: %s\n", gtk_css_provider_to_string(provider));
    }
#else
    GdkRGBA color = {.0, .0, .0, 1.0};
    if (!NIL_P(uc)) {
#ifdef NEW_MACRO_COLOR
        Get_TypedStruct2(uc, shoes_color, col);
#else
        shoes_color *col;
        Data_Get_Struct(uc, shoes_color, col);
#endif
        color.red = col->r/255.0;
        color.green = col->g/255.0;
        color.blue = col->b/255.0;
    }
    gtk_widget_override_background_color(GTK_WIDGET(da), 0, &color);
#endif 
    g_signal_connect(G_OBJECT(da), "realize",
                     G_CALLBACK(surface_on_realize),
                     (gpointer)video);

    return da;
}

void shoes_native_surface_remove(SHOES_CONTROL_REF ref) {
    gtk_widget_destroy(ref);
}

/* doing this directly on control now
 *
void
shoes_native_surface_position(SHOES_SURFACE_REF ref, shoes_place *p1,
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  shoes_native_control_position((SHOES_CONTROL_REF)ref, p1, self, canvas, p2);
}

void
//shoes_native_surface_hide(SHOES_SURFACE_REF ref)
shoes_native_surface_hide(SHOES_CONTROL_REF ref)
{
  shoes_native_control_hide(ref);
}

void
//shoes_native_surface_show(SHOES_SURFACE_REF ref)
shoes_native_surface_show(SHOES_CONTROL_REF ref)
{
  shoes_native_control_show(ref);
}
*/
