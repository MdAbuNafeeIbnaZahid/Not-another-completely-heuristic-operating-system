#include "IDManager.h"


IDManager::IDManager(int size){
	this->items = new void*[size];
    this->size = size;
    for (int i = 0; i < size; ++i){
        items[i] = 0;
    }

    lock = new Lock("Process table lock");
    numItems  = 0;
}

IDManager::~IDManager(){
    delete items;
    delete lock;
}

/* Allocate a table slot for "object", returning the "index" of the
allocated entry; otherwise, return -1 if no free slots are available. */
int IDManager::Alloc(void *object){
    lock->Acquire();
    //myDebug('e', "Allocating\n");
    for (int i = 0; i < size; ++i){
        if(items[i] == 0){
            items[i] = object;
            numItems++;
            lock->Release();
            return (i + 1); 
            ///why I have return (i+1) not i? 
            ///In offline specification, we must return processId = 0 when the process is not 
            ///successfully created. So we must have valid process ids in range [1, size].That
            ///is we need 1-based indexing. But array is 0-based indexing. So we must convert 
            ///0-based to 1-based indexing.
        }
    }
    lock->Release();
    return -1;
}

/* Retrieve the object from table slot at "index", or NULL if that
slot has not been allocated. */
void * IDManager::Get(int index){
	index--; //why decrement? [see above]
    lock->Acquire();
    if(index < 0 || index >= size || items[index] == 0){
        
        lock->Release();
        return 0;
    }
    lock->Release();
    return items[index];

}

/* Free the table slot at index. */
void IDManager::Release(int index){
	index--; //why decrement? [see above]
    lock->Acquire();
    //myDebug('e', "releasing\n");
    if(index < 0 || index >= size) {
        lock->Release();
        return;
    }
    numItems--;
    lock->Release();
    items[index] = 0;
}