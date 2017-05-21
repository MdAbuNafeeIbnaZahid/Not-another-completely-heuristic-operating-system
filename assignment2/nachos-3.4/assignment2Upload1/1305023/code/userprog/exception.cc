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
#include "processtable.h"
#include "memorymanager.h"

#define MAX_SIZE 99

void incrementPC()
{
	int prevPC = machine->ReadRegister(PrevPCReg);
	int PC = machine->ReadRegister(PCReg);
	int nextPC = machine->ReadRegister(NextPCReg);

	prevPC = PC;
	PC = nextPC;
	nextPC += 4;

	machine->WriteRegister(PrevPCReg, prevPC);
	machine->WriteRegister(PCReg, PC);
	machine->WriteRegister(NextPCReg, nextPC);
	
}


extern Lock *consoleLock;
extern Semaphore *consoleReadSemaphore;
extern Semaphore *consoleWriteSemaphore;
extern Console *myConsole;
extern ProcessTable *processTable;
extern MemoryManager *memorymanager;

void regInThreadFun(int i)
{
	currentThread->space->RestoreState();
	currentThread->space->InitRegisters();
	machine->Run();
}

int myExec(int path)
{
	printf("\n path = %d \n", path);
	char buffer[MAX_SIZE];
	int i = 0;
	do
	{
		printf("i = %d \n", i);
		int x;
		if ( ! machine->ReadMem(path, 1, &x) )
		{
			return 0;
		}
		buffer[i] = (char) x;
		i++;
		path++;
		if ( i >= MAX_SIZE )
		{
			printf("\n File path too big  \n" );
			return 0;
		}
	} while( buffer[i] );

	printf("\n buffer = %s \n", buffer);

	OpenFile *fd = fileSystem->Open( buffer );
	if ( fd == 0 )
	{
		printf("\n Invalid filypath \n");
		return 0;
	}
	AddrSpace *addrSpace = new AddrSpace(fd);
	Thread *thread = new Thread(buffer);

	int pid = processTable->Alloc( thread );

	thread->space = addrSpace;
	addrSpace->processId = pid;

	thread->Fork(regInThreadFun, NULL);
}


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

    	//int currentPC = machine->ReadRegister(PCReg);
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
    	
    	//currentPC++;
    	//machine->WriteRegister(PCReg, currentPC);

    	incrementPC();

    	//return;
    }
    else if ( (which == SyscallException) && (type == SC_Read) )
    {
    	//printf("Read called \n" );
    	consoleLock->Acquire();

    	char *buf = (char *) machine->ReadRegister(4);
    	int size = machine->ReadRegister(5);
    	OpenFileId id = machine->ReadRegister(6);
    	if ( id != 0 )
    	{
    		printf(" file I/O not implemented  \n");
    		ASSERT(FALSE);
    	}
    	int i, j, k;
    	for (i = 0; i < size; i++)
    	{
    		consoleReadSemaphore->P();
			char ch = myConsole->GetChar();
    		machine->WriteMem( (int)(buf+i), 1, ch );	
    	}
    	consoleLock->Release();
    	incrementPC();
    }
    else if ( (which == SyscallException) && (type == SC_Exec) )
    {
    	int path = machine->ReadRegister(4);
    	myExec(path);
    	incrementPC();
    }
    else if ( (which == SyscallException) && (type == SC_Exit) )
    {
    	int status = machine->ReadRegister(4);
    	int numPages = currentThread->space->numPages;
    	int processId = currentThread->space->processId;
    	int i;
    	for (i = 0; i < numPages; i++)
    	{
    		int physPageNum = machine->pageTable[i].physicalPage;
    		memorymanager->FreePage( physPageNum );
    	}
    	processTable->Release( processId );

    	// Nafee : not sure where the incrementPC should be placed
    	incrementPC();
    	if ( processTable->numProcess() == 0 )
    	{
    		interrupt->Halt();
    	}
    	currentThread->Finish();
    	printf("\n At the end of exit routine \n" );
    }
    else if (which == PageFaultException)
    {
    	printf("\n PageFaultException detected \n");
    }
    else if (which == ReadOnlyException)
    {
    	printf("\n ReadOnlyException detected \n");
    }
    else if (which == BusErrorException)
    {
    	printf("\n BusErrorException detected \n");
    }
    else if (which == AddressErrorException)
    {
    	printf("\n AddressErrorException detected \n");
    }
    else if (which == OverflowException)
    {
    	printf("\n OverflowException detected \n");
    }
    else if (which == IllegalInstrException)
    {
    	printf("\n IllegalInstrException detected \n");
    }
    else if (which == NumExceptionTypes)
    {
    	printf("\n NumExceptionTypes detected \n");
    }
    
     else 
     {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
    }
}
