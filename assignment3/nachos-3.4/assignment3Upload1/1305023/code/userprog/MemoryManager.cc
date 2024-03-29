//
// Created by MUKTADIR on 5/1/2017.
//

#include "MemoryManager.h"

MemoryManager::MemoryManager(int numPages){
    this->numPages = numPages;
    this->numFreePages = numPages;
    this->victimCnt = 0;
    pageStatus = new BitMap(this->numPages);
    lock = new Lock("ofk");
    processMap = new int[numPages];
    entries = new TranslationEntry*[numPages];
    int i;
    for ( i = 0; i < numPages; i++)
    {
        processMap[i] = -1;
        entries[i] = NULL;
    }
}

MemoryManager::~MemoryManager(){
    delete pageStatus;
    delete lock;
}

///allocates a page
int MemoryManager::AllocPage(){
    lock->Acquire();
    printf("\nMemoryManager::AllocPage(). numFreePages = %d\n", numFreePages);
    if(numFreePages == 0){ //no free page available.
        lock->Release();
        return -1;
    }

    int i;
    for (i = 0; (i < numPages) && (pageStatus->Test(i)); ++i);

    numFreePages--;
    printf("\nmarking: %d\n",i );
    pageStatus->Mark(i);

    lock->Release();
    return i;
}

///allocates a page
int MemoryManager::Alloc(int processNo, TranslationEntry *entry){
    lock->Acquire();
    printf("\nMemoryManager::AllocPage(). numFreePages = %d\n", numFreePages);
    if(numFreePages == 0){ //no free page available.
        lock->Release();
        return -1;
    }

    int i;
    for (i = 0; (i < numPages) && (pageStatus->Test(i)); ++i);

    numFreePages--;
    printf("\nmarking: %d\n",i );
    pageStatus->Mark(i);

    processMap[i] = processNo;
    entries[i] = entry;

    lock->Release();
    return i;
}



void MemoryManager::FreePage(int physPageNum){
    lock->Acquire();
    printf("\nMemoryManager::FreePage\n");
    printf("\nphysPageNum = %d\n", physPageNum);
    printf("\nnumFreePages = %d\n", numFreePages);
    // printf("memMana: fee. #free: %d\n", numFreePages);

    pageStatus->Clear(physPageNum);
    numFreePages++;



    lock->Release();
}

bool MemoryManager::PageIsAllocated(int physPageNum){
    ///I think I don't need this.
}

int MemoryManager::AllocByForce()
{
    return (victimCnt++)%numPages;
}

int MemoryManager::GetProcessIdFromPhysPageNum(int physPageNum)
{
    return processMap[physPageNum];
}

TranslationEntry* MemoryManager::GetTranslationEntryFromPhysPageNum(int physPageNum)
{
    return entries[physPageNum];
}