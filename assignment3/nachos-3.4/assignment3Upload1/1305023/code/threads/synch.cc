// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
    	queue->Append((void *)currentThread);	// so go to sleep

    	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}


// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName){ //written by me
    name = debugName;
    blockedThreads = new List();
    lockerThread = 0;
}
Lock::~Lock(){ //written by me
    delete  blockedThreads;
}

void Lock::Acquire(){ ///written by me
    IntStatus prevLevel = interrupt->SetLevel(IntOff); ///disable interrupt so that no context switch occurs while executing this procedure

    if(lockerThread != 0 && lockerThread != currentThread){

        do{
            blockedThreads->Append(currentThread);
            currentThread->setStatus(BLOCKED);
            /*I should not enable the interrupt. Otherwise deadlock may occur. 
            How?
            Suppose it is a consumer thread. Suppose immediately after enabling the interrupt, 
            an interrupt occurs. So context switch will occur. Again suppose the new thread is the producer
            After gaining context, the producer produced (MAX_QUEUE_SIZE + 1) [+1, becuase MAX_QUEUE_SIZE
            will be inserted usual way. It will go to sleep while trying to insert another item] 
            number of items, and then went to sleep. 
            Now the consumer came and go to sleep. As a result, both producer and consumer will sleep, 
            and deadlock will occur.
            */
            //(void) interrupt->SetLevel(prevLevel); ///do i really need this? will test after writing codes. No, as semaphore code has not done this
            currentThread->Sleep();
                                                //prevLevel = interrupt->SetLevel(IntOff);
        }while (lockerThread != 0);
    }
    //else the thread that contains the lock again asked for lock. So just ignore.
    
    lockerThread = currentThread;

    (void) interrupt->SetLevel(prevLevel);
}

void Lock::Release(){
    IntStatus prevLevel = interrupt->SetLevel(IntOff); ///disable interrupt so that no context switch occurs while executing this procedure

    if(lockerThread == currentThread){
        
        lockerThread = 0;
        Thread *t = (Thread *) blockedThreads->Remove();
        if(t != NULL) scheduler->ReadyToRun(t);
    }
    else{
        //error printing
    }

    (void) interrupt->SetLevel(prevLevel);
}

bool Lock::isHeldByCurrentThread(){
    return currentThread == lockerThread;
}

Condition::Condition(char* debugName){
    blockedThreads = new List();
}

Condition::~Condition() {
    delete blockedThreads;
}

void Condition::Wait(Lock* conditionLock){
    //ASSERT(FALSE);

    ///my code
    IntStatus prevLevel = interrupt->SetLevel(IntOff); ///disable interrupt so that no context switch occurs while executing this procedure
    if(conditionLock->isHeldByCurrentThread()){
        blockedThreads->Append(currentThread);
        currentThread->setStatus(BLOCKED);

        conditionLock->Release();
        currentThread->Sleep();
        conditionLock->Acquire();

    }
    (void) interrupt->SetLevel(prevLevel);
}

void Condition::Signal(Lock* conditionLock){
    IntStatus prevLevel = interrupt->SetLevel(IntOff);
    
    if(conditionLock->isHeldByCurrentThread()){
        Thread * t = (Thread*) blockedThreads->Remove();

        if(t != NULL) scheduler->ReadyToRun(t);
    }
    else{
        //error
        printf("You don't have the lock\n");
    }

    (void) interrupt->SetLevel(prevLevel);
}
void Condition::Broadcast(Lock* conditionLock){
    IntStatus prevLevel = interrupt->SetLevel(IntOff);
    
    if(conditionLock->isHeldByCurrentThread()){
        while (1) {
            Thread *t = (Thread *) blockedThreads->Remove();

            if (t != NULL) scheduler->ReadyToRun(t);
            else break;
        }
    }
    else{
        printf("You don't have the lock\n");
    }

    (void) interrupt->SetLevel(prevLevel);
}