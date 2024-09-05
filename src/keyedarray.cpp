/* Array keyed by strings, with binary search */

#include "keyedarray.h"


int StrCmp(const char *a, const char *b)
{
	while(*a && (*a == *b)) { a++; b++; }
	return (*(const unsigned char*)a - *(const unsigned char*)b);
}

/*-------------------------------------------------------------------------------------------*/
