
#ifndef GTK_LABEL_ALT_H
#define	GTK_LABEL_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_BEGIN_DECLS

#define GTKLABEL_ALT_TYPE            (gtklabel_alt_get_type())
#define GTKLABEL_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTKLABEL_ALT_TYPE, GtKLabel_Alt))
#define GTKLABEL_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTKLABEL_ALT_TYPE, GtKLabel_AltClass))
#define IS_GTKLABEL_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTKLABEL_ALT_TYPE))
#define IS_GTKLABEL_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTKFIXED_ALT_TYPE))

typedef struct _GtKLabel_Alt {
    GtkLabel parent_instance;
} GtKLabel_Alt;

typedef struct _GtKLabel_AltClass {
    GtkLabelClass parent_class;
} GtKLabel_AltClass;

GType gtklabel_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtklabel_alt_new(void);
GtkWidget *gtklabel_alt_new_with_label(const gchar *label);

G_END_DECLS

#endif /* GtkLabel_Alt_H */
