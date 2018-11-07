#pragma once

#include <glib-object.h>

G_BEGIN_DECLS
/*
 * Type declaration.
 */

#define GSHOES_TYPE_ELE (gshoes_ele_get_type())

G_DECLARE_FINAL_TYPE (GshoesEle, gshoes_ele, GSHOES, ELE, GObject)

/*
 * Method definitions.
 */
extern GshoesEle *gshoes_ele_new(GString *str, gpointer *ele);
extern gpointer gshoes_ele_get_element(GshoesEle *gs);
G_END_DECLS
