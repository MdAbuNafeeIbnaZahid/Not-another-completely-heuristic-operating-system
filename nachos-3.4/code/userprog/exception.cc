// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "console.h"

extern Lock *consoleLock;
extern Semaphore *consoleReadSemaphore;
extern Semaphore *consoleWriteSemaphore;
extern Console *myConsole;


// extern MemoryManager *memorymanager;


//extern ThreadSafeSynchronizedConsole *threadSafeSynchronizedConsole;

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
	//printf(" memory manager has free pages %d \n", memorymanager->HowManyPageFree() );

    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
		DEBUG('a', "Shutdown, initiated by user program.\n");
	   	interrupt->Halt();
    }
    else if ( (which == SyscallException) && (type == SC_Write) )
    {
    	consoleLock->Acquire();
    	//printf("Write called \n" );
    	char *buf = (char *) machine->ReadRegister(4);
    	int size = machine->ReadRegister(5);
    	OpenFileId id = machine->ReadRegister(6);

    	int currentPC = machine->ReadRegister(PCReg);
    	if ( id != 0 )
    	{
    		printf(" file I/O not implemented  \n");
    		ASSERT(FALSE);
    	}
    	//printf("size = %d \n", size );
    	int i, j, k;
    	for (i = 0; i < size; i++)
    	{
    		int val;
    		machine->ReadMem( (int)(buf+i), 1, &val);
    		myConsole->PutChar( (char)val );
    		consoleWriteSemaphore->P();
    	}
    	consoleLock->Release();
    	
    	currentPC++;
    	machine->WriteRegister(PCReg, currentPC);

    	return;
    }
    else if ( (which == SyscallException) && (type == SC_Read) )
    {
    	// printf("Read called \n" );
    	// char *buf = (char *) machine->ReadRegister(4);
    	// int size = machine->ReadRegister(5);
    	// OpenFileId id = machine->ReadRegister(6);
    	// if ( id != 0 )
    	// {
    	// 	printf(" file I/O not implemented  \n");
    	// 	ASSERT(FALSE);
    	// }
    	// int i, j, k;
    	// for (i = 0; i < size; i++)
    	// {
    	// 	char ch = threadSafeSynchronizedConsole->GetChar();
    	// 	machine->WriteMem( (int)(buf+i), 1, ch );	
    	// }
    }
     else 
     {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
    }
}
