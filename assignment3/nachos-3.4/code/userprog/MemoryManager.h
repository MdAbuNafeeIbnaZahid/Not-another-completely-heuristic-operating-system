//
// Created by MUKTADIR on 5/1/2017.
//

#ifndef CODE_LEARNING_MEMORYMANAGER_H
#define CODE_LEARNING_MEMORYMANAGER_H
#include "bitmap.h"
#include "synch.h"
#include "translate.h"



class MemoryManager {
    int numPages;
    int numFreePages;

    BitMap *pageStatus; ///(pageStatus[i] = 0) => i-th page frame is free.
    //// (pageStatus[i] = 1) => i-th page frame is alocated.

    Lock* lock;//for thread safety

    int victimCnt;

    
public:
    /* Create a manager to track the allocation of numPages of physical memory.
    You will create one by calling the constructor with NumPhysPages as
    the parameter. All physical pages start as free, unallocated pages. */
    MemoryManager(int numPages);

    ~MemoryManager();

    /* Allocate a free page, returning its physical page number or -1
    if there are no free pages available. */
    int AllocPage();

    /* Free the physical page and make it available for future allocation. */
    void FreePage(int physPageNum);

    int getNumFreePages(){ return numFreePages;}

    /* True if the physical page is allocated, false otherwise. */
    bool PageIsAllocated(int physPageNum); ///haven't implemented this.

    int Alloc(int processNo, TranslationEntry *entry);

    int AllocByForce();

    int GetProcessIdFromPhysPageNum(int physPageNum);

    TranslationEntry *GetTranslationEntryFromPhysPageNum(int physPageNum);



    // Nafee : 
    int *processMap;

    TranslationEntry **entries; 
};


#endif //CODE_LEARNING_MEMORYMANAGER_H
