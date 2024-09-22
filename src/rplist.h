/* System compatible bidirectional intrusive list. */

#ifndef BZTED_RPLIST_H
#define BZTED_RPLIST_H

#include "strutils.h"

#include <proto/dos.h>


template <class T> class RpNode
{
	RpNode<T> *succ;
	RpNode<T> *pred;

	public:

	T* next() { return (T*)(succ->succ ? succ : NULL); }
	T* prev() { return (T*)(pred->pred ? pred : NULL); }

	void insertAfter(RpNode<T> *node) 
	{
		succ = node->succ;
		pred = node;
		node->succ->pred = this;
		node->succ = this;
	}

	void insertBefore(RpNode<T> *node)
	{
		pred = node->pred;
		succ = node;
		node->pred->succ = this;
		node->pred = this;
	}

	void remove()
	{
		pred->succ = succ;
		succ->pred = pred;
	}
};


template <class T> class RpNamedNode : public RpNode<T>
{
	const char *nodeName;

	public:
	
	RpNamedNode<T>(const char *n) { nodeName = n; }
	const char* name() { return nodeName; }
};


template <class T> class RpList
{
	T *head;
	T *tail;
	T *tailpred;

	public:

	RpList() { head = (T*)&tail; tail = NULL; tailpred = (T*)&head; }
	bool isEmpty() { return (first() ? FALSE : TRUE); }
	void addHead(T *node) { node->insertAfter((T*)&head); }
	void addTail(T *node) { node->insertBefore((T*)&tail); }
	T* first() { return (head == (T*)&tail) ? NULL : head; }
	T* last() { return (tailpred == (T*)&head) ? NULL : tailpred; }
	T* remHead() { T *node = first(); if (node) node->remove(); return node; }
	T* remTail() { T *node = last(); if (node) node->remove(); return node; }
};


template <class T> class RpNamedList : public RpList<T>
{
	public:
	
	T* find(const char *key);
};


template <class T>
T* RpNamedList<T>::find(const char *key)
{
	T* obj;

	for (obj = first(); obj; obj = obj->next())
	{
		if (StrCmp(obj->name(), key) == 0) return obj;
	}

	return NULL; 
}


//---------------------------------------------------------------------------------------------
// System list with find by name.

template <class T> class RpKeyedList : public RpList<T>
{
	public:

	T* find(const char *key);
};


template <class T>
T* RpKeyedList<T>::find(const char *key)
{
	T* obj;

	for (obj = first(); obj; obj = obj->next())
	{
		if (StrCmp(obj->name, key) == 0) return obj;
	}

	return NULL; 
}

#endif  /* BZTED_SYSLIST_H */

