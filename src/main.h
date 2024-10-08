/* global definitions */

#ifndef BZTED_MAIN_H
#define BZTED_MAIN_H

#define BZTVER "0.1"

#include <exec/types.h>
#include <exec/libraries.h>

#define MAKE_ID3(a, b, c) (((a) << 24) | ((b) << 16) | ((c) << 8))
#define MAKE_ID4(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define RESULTS_TO_STACK 0
#define RESULTS_TO_FRAME 1

static inline int max(int a, int b) { return ((b > a) ? b : a); }
static inline int min(int a, int b) { return ((b < a) ? b : a); }

extern Library *SysBase, *DOSBase;

class Compiler;

extern Compiler *Comp;

#endif  /* BZTED_MAIN_H */
