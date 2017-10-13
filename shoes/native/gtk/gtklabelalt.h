
#ifndef GTK_LABEL_ALT_H
#define	GTK_LABEL_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_LABEL_ALT           (gtk_label_alt_get_type())
#define GTK_LABEL_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_GTK_LABEL_ALT, GtkLabel_Alt))
#define GTK_LABEL_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_GTK_LABEL_ALT, GtkLabel_AltClass))
#define IS_GTK_LABEL_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_GTK_LABEL_ALT))
#define IS_GTK_LABEL_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_GTK_LABEL_ALT))
#define GTK_GTK_LABEL_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GTK_LABEL_ALT, GtkLabel_AltClass))

typedef struct _GtkLabel_AltPrivate GtkLabel_AltPrivate;

typedef struct _GtkLabel_Alt {
    GtkLabel parent_instance;
} GtkLabel_Alt;

typedef struct _GtkLabel_AltClass {
    GtkLabelClass parent_class;
} GtkLabel_AltClass;

GType gtk_label_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtk_label_alt_new(void);
GtkWidget *gtk_label_alt_new_with_label(const gchar *label);

G_END_DECLS

#endif /* GtkLabel_Alt_H */
