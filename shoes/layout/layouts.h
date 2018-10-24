#ifndef SHOES_LAYOUTS_H
#define SHOES_LAYOUTS_H

#include "shoes/layout/emeus-vfl-parser-private.h"
VALUE shoes_vfl_rules(shoes_layout *lay, shoes_canvas *canvas, VALUE args);
void shoes_vfl_add_ele(shoes_canvas *canvas, VALUE ele);
#endif
