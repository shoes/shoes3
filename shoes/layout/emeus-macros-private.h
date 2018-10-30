/* emeus-macros-private.h: Private macros
 *
 * Copyright 2016  Endless
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <glib-object.h>

#define EMEUS_ENUM_VALUE(EnumValue,EnumNick) { EnumValue, #EnumValue, EnumNick },

#define EMEUS_DEFINE_ENUM_TYPE(TypeName,type_name,values) \
GType \
type_name ## _get_type (void) \
{ \
  static volatile gsize emeus_define_id__volatile = 0; \
  if (g_once_init_enter (&emeus_define_id__volatile)) \
    { \
      static const GEnumValue v[] = { \
        values \
        { 0, NULL, NULL }, \
      }; \
      GType emeus_define_id = \
        g_enum_register_static (g_intern_static_string (#TypeName), v); \
      g_once_init_leave (&emeus_define_id__volatile, emeus_define_id); \
    } \
  return emeus_define_id__volatile; \
}
