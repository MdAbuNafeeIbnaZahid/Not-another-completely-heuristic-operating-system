#ifndef TABLE_H
#define TABLE_H

#include "synch.h"


/**Keep track of process/file ids */
class IDManager{
	void** items;
	int size;
	int numItems;

	Lock* lock;
public:
	IDManager(int size);
	~IDManager();

	/* Allocate a table slot for "object", returning the "index" of the
	allocated entry; otherwise, return -1 if no free slots are available. */
	int Alloc(void *object);

	/* Retrieve the object from table slot at "index", or NULL if that
	slot has not been allocated. */
	void *Get(int index);

	/* Free the table slot at index. */
	void Release(int index);

	int currentItems(){ return numItems;}
};

#endif