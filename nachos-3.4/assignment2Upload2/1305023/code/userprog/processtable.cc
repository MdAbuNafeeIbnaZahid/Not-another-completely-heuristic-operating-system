#include "processtable.h"

ProcessTable::ProcessTable(int size)
{
	this->size = size;
	tablePtr = new Thread*[size];
	int i;
	for (i = 0; i < size; i++)
	{
		tablePtr[i] = NULL;
	}
}

ProcessTable::~ProcessTable()
{
	int i;
	for (i = 0; i  < size ;i++)
	{
		if ( tablePtr[i] )
		{
			delete tablePtr[i];
		}
		tablePtr[i] = NULL;
	}
	delete tablePtr;
	tablePtr = NULL;
}

int ProcessTable::Alloc(Thread *thread)
{
	int i;
	for (i = 0; i < size; i++)
	{
		if ( tablePtr[i] == NULL )
		{
			tablePtr[i] =  thread;
			return i;
		}
	}
	return -1;
}

Thread* ProcessTable::Get( int idx )
{
	if (idx < 0 || idx >= size)
	{
		return NULL;
	}
	return tablePtr[ idx ];
}

void ProcessTable::Release(int idx)
{
	if (idx < 0 || idx >= size)
	{
		return;
	}
	if ( tablePtr[idx] )
	{
		delete tablePtr[idx];
	}
	tablePtr[ idx ] = NULL;
}

int ProcessTable::numProcess()
{
	int cnt = 0, i = 0;
	for (i = 0; i < size; i++)
	{
		cnt += ( tablePtr[i]==NULL );
	}
	return cnt;
}