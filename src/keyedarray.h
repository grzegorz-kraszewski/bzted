/* KeyedArray is an array of KeyedPairs + binary search */

#ifndef BZTED_KEYEDARRAY_H
#define BZTED_KEYEDARRAY_H

#include "strutils.h"


template <class T>
struct KeyedPair
{
	const char* key;
	T value;
};


template <class T>
class KeyedArray
{
	KeyedPair<T> *data;
	int length;

	public:

	KeyedArray() { data = NULL; length = 0; }
	void fetch(KeyedPair<T> *idata, int ilength) { data = idata; length = ilength; }
	T* find(const char *name);
};


template <class T>
T* KeyedArray<T>::find(const char *name)
{
	int s = 0, e = length - 1, c, comp;

	while (s <= e)
	{
		c = (s + e) >> 1;
		comp = StrCmp(data[c].key, name);
		if (comp == 0) return &data[c].value;
		if (comp < 0) s = c + 1;
		if (comp > 0) e = c - 1;
	}

	return NULL;
}

#endif  /* BZTED_KEYEDARRAY_H */
