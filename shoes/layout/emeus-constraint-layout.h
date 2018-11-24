/* emeus-constraint-layout.h: The constraint layout manager
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

//#include <gtk/gtk.h>
#include "shoes/layout/emeus-types.h"
#include "shoes/layout/emeus-constraint.h"
#include "shoes/layout/gshoes-ele.h"
#include "shoes/layout/shoes-vfl.h"
G_BEGIN_DECLS

#define EMEUS_TYPE_CONSTRAINT_LAYOUT (emeus_constraint_layout_get_type())

/**
 * EmeusConstraintLayout:
 *
 * A container that uses constraints to determine the layout policy.
 *
 * The contents of the `EmeusConstraintLayout` are private and should never be
 * accessed directly.
 *
 * Since: 1.0
 */
EMEUS_AVAILABLE_IN_1_0
G_DECLARE_FINAL_TYPE (EmeusConstraintLayout, emeus_constraint_layout, EMEUS, CONSTRAINT_LAYOUT, GObject)

EMEUS_AVAILABLE_IN_1_0
EmeusConstraintLayout *     emeus_constraint_layout_new  (shoes_layout *lay);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_pack   (EmeusConstraintLayout *layout,
                                                GshoesEle            *child,
                                                const char            *name,
                                                EmeusConstraint       *first_constraint,
                                                ...);
extern void
emeus_constraint_layout_size_allocate (EmeusConstraintLayout *self, 
                                          int width,
                                          int height);
extern Variable *
get_layout_attribute (EmeusConstraintLayout   *layout,
                      EmeusConstraintAttribute attr);


EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_add_constraint          (EmeusConstraintLayout *layout,
                                                                 EmeusConstraint       *constraint);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_add_constraints         (EmeusConstraintLayout *layout,
                                                                 EmeusConstraint       *first_constraint,
                                                                 ...) G_GNUC_NULL_TERMINATED;

EMEUS_AVAILABLE_IN_1_0
GList *         emeus_constraint_layout_get_constraints         (EmeusConstraintLayout *layout);

EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_clear_constraints       (EmeusConstraintLayout *layout);

#define EMEUS_TYPE_CONSTRAINT_LAYOUT_CHILD (emeus_constraint_layout_child_get_type())

/**
 * EmeusConstraintLayoutChild:
 *
 * A child in a #EmeusConstraintLayout.
 *
 * The contents of the `EmeusConstraintLayoutChild` structure are private and
 * should never be accessed directly.
 *
 * Since: 1.0
 */
EMEUS_AVAILABLE_IN_1_0
G_DECLARE_FINAL_TYPE (EmeusConstraintLayoutChild, emeus_constraint_layout_child, EMEUS, CONSTRAINT_LAYOUT_CHILD, GObject)

EMEUS_AVAILABLE_IN_1_0
EmeusConstraintLayoutChild *     emeus_constraint_layout_child_new  (const char *name,
        GshoesEle *ele);
        
extern Variable *
get_child_attribute (EmeusConstraintLayoutChild *child,
                     EmeusConstraintAttribute    attr);
                     
EMEUS_AVAILABLE_IN_1_0
const char *    emeus_constraint_layout_child_get_name                  (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_top                   (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_right                 (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_bottom                (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_left                  (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_width                 (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_height                (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_center_x              (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
int             emeus_constraint_layout_child_get_center_y              (EmeusConstraintLayoutChild *child);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_child_set_intrinsic_width       (EmeusConstraintLayoutChild *child,
                                                                         int                         width);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_child_set_intrinsic_height      (EmeusConstraintLayoutChild *child,
                                                                         int                         height);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_child_add_constraint            (EmeusConstraintLayoutChild *child,
                                                                         EmeusConstraint            *constraint);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_child_remove_constraint         (EmeusConstraintLayoutChild *child,
                                                                         EmeusConstraint            *constraint);
EMEUS_AVAILABLE_IN_1_0
void            emeus_constraint_layout_child_clear_constraints         (EmeusConstraintLayoutChild *child);

G_END_DECLS
