/* System compatible bidirectional intrusive list. */

#ifndef BZTED_SYSLIST_H
#define BZTED_SYSLIST_H

#include <proto/dos.h>

template <class T> class SysListIter;

template <class T> class SysList
{
	friend class SysListIter<T>;

	T *head;
	T *tail;
	T *tailpred;

	public:

	SysList()
	{
		if (this)
		{
			head = (T*)&tail;
			tail = NULL;
			tailpred = (T*)&head;
		}
	}

	bool isempty()
	{
		return (head->succ ? FALSE : TRUE);
	}

	void insert(T *node, T *after)
	{
		node->succ = after->succ;
		node->pred = after;
		after->succ->pred = node;
		after->succ = node;
	}

	void remove(T *node)
	{
		node->succ->pred = node->pred;
		node->pred->succ = node->succ;
	}

	void addhead(T *node)
	{
		node->succ = head;
		node->pred = (T*)&head;
		head->pred = node;
		head = node;	
	}

	void addtail(T *node)
	{
		node->succ = (T*)&tail;
		node->pred = tailpred;
		tailpred->succ = node;
		tailpred = node;
	}

	T* remhead()
	{
		T* node = head;

		if (node->succ)
		{
			head = node->succ;
			head->pred = (T*)&head;
			return node;
		}
		else return NULL;
	}

	T* remtail()
	{
		T* node = tailpred;

		if (node->pred)
		{
			tailpred = node->pred;
			tailpred->succ = (T*)&tail;
			return node;
		}
		else return NULL;
	}

	T* first() { return (head == (T*)&tail) ? NULL : head; }

	T* last() { return (tailpred == (T*)&head) ? NULL : tailpred; }

	T* next(T *current)
	{
		current = current->succ;
		return (current == (T*)&tail) ? NULL : current;
	}
};

/*-------------------------------------------------------------------------------------------*/

template <class T> class SysListIter
{
	T* ptr;

	public:

	SysListIter(SysList<T> &list) { ptr = list.head; }
	T* operator++(int) { T* p = ptr; ptr = ptr->succ; return (ptr ? p : NULL); }
};

/*-------------------------------------------------------------------------------------------*/

extern int StrCmp(const char *a, const char *b);   /* nasty */

/* System list with find by name. */


template <class T> class KeyedSysList : public SysList<T>
{
	public:

	T* find(const char *key)
	{
		T* obj;

		for (obj = first(); obj; obj = next(obj))
		{
			if (StrCmp(obj->name, key) == 0) return obj;
		}

		return NULL; 
	}
};

#endif  /* BZTED_SYSLIST_H */

