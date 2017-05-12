#ifndef PROCESS_TABLE_H
#define PROCESS_TABLE_H



#include "thread.h"

class ProcessTable
{
private:
	int size;
	Thread **tablePtr;
public:
	ProcessTable(int size);

	~ProcessTable();

	int Alloc(Thread *thread);

	Thread *Get( int idx );

	void Release(int idx);
};


#endif