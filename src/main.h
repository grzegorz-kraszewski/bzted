/* global definitions */

#include <exec/types.h>
#include <exec/libraries.h>

#define MAKE_ID3(a, b, c) (((a) << 24) | ((b) << 16) | ((c) << 8))
#define MAKE_ID4(a, b, c, d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

extern Library *SysBase, *DOSBase;
