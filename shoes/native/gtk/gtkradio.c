#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkradio.h"

/* 
 * This is called at 'draw' time (actual 
 * group is an Array of SHOES_CONTROL_REF's
 */
SHOES_CONTROL_REF shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group) {
    SHOES_CONTROL_REF ref;
    GSList *list = NULL;
    
    if (!NIL_P(group)) {
        shoes_control *lctrl;
        VALUE leader = rb_ary_entry(group, 0);
        if (NIL_P(leader))
          fprintf(stderr,"No control in group hash");
        Data_Get_Struct(leader, shoes_control, lctrl);
#if 1
        GtkWidget *first = (GtkWidget *)lctrl->ref;
        ref = gtk_radio_button_new(NULL);
        gtk_radio_button_join_group(ref, first);
#else
        ref = gtk_radio_button_new(NULL);
        
        list = gtk_radio_button_get_group(GTK_RADIO_BUTTON(lctrl->ref));
        if (list == NULL) 
          printf("group len: %d\n", g_slist_length(list));
#endif
    } else {
      // no group from shoes - not likely - see types/radio.c
      printf("no group specified\n");
      ref = gtk_radio_button_new(list);
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);
    return ref;
}
