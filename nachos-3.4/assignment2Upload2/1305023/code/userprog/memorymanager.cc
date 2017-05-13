#include "memorymanager.h"

MemoryManager::MemoryManager(int numpages)
{
	this->numpages = numpages;
	lock = new Lock("Lock of memory manager");
	bitmap = new BitMap(numpages);
}


MemoryManager::~ MemoryManager()
{
	lock->Acquire();
	this->numpages = 0;
	delete bitmap;
	lock->Release();
	delete lock;
}

int MemoryManager::AllocPage()
{
	lock->Acquire();
	int ret = bitmap->Find();
	lock->Release();
	return ret;
}

void MemoryManager::FreePage(int physPageNum)
{
	lock->Acquire();
	bitmap->Clear(physPageNum);
	lock->Release();
}

bool MemoryManager::IsPageAllocated(int physPageNum)
{
	lock->Acquire();
	bool ret = bitmap->Test(physPageNum);
	lock->Release();
	return ret;
}

int MemoryManager::HowManyPageFree()
{
	return bitmap->NumClear();
}