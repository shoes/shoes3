/* emeus-utils.h: Utility functions
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

#include "emeus-types.h"
#include "emeus-constraint.h"

G_BEGIN_DECLS

EMEUS_AVAILABLE_IN_1_0
GList * emeus_create_constraints_from_description       (const char * const  lines[],
                                                         guint               n_lines,
                                                         int                 hspacing,
                                                         int                 vspacing,
                                                         GHashTable         *views,
                                                         GHashTable         *metrics);

G_END_DECLS
