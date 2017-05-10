#include "synch.h"
#include "bitmap.h"

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

class MemoryManager
{
	public:
		 MemoryManager(int numpages);
		~ MemoryManager();
		int AllocPage();
		void FreePage(int physPageNum);
		bool IsPageAllocated(int physPageNum); 
	private:
		int numpages;
		Lock *lock;
		BitMap *bitmap;
};

#endif