#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include "chip8.h"

#define UNSET_PIXEL '-'
#define SET_PIXEL '#'

void print_screen(u_int8_t *screen);
void print_state(state *state);

#endif