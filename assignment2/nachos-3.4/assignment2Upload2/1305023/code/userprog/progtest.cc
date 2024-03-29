// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "memorymanager.h"
#include "thread.h"
#include "processtable.h"
//#include "threadSafeSynchronizedConsole.h"

#define MAX_PROCESS_NUM 99




// Nafee : 
Lock *consoleLock;
Semaphore *consoleReadSemaphore;
Semaphore *consoleWriteSemaphore;
Console *myConsole;
ProcessTable *processTable;
// Nafee 
static void MyReadAvail(int arg) { consoleReadSemaphore->V(); }
static void MyWriteDone(int arg) { consoleWriteSemaphore->V(); }

MemoryManager *memorymanager;

//ThreadSafeSynchronizedConsole *threadSafeSynchronizedConsole;

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    memorymanager = new MemoryManager(NumPhysPages);
    processTable = new ProcessTable(MAX_PROCESS_NUM);

    consoleLock = new Lock("ThreadSafeSynchronizedConsole Lock");
    consoleReadSemaphore = new Semaphore("consoleReadSemaphore", 0);
    consoleWriteSemaphore = new Semaphore("consoleWriteSemaphore", 0);
    myConsole = new Console(NULL, NULL, MyReadAvail, MyWriteDone, 0);
    
    //memorymanager = new MemoryManager(NumPhysPages);
    
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);    
    currentThread->space = space;



    delete executable;			// close file



    space->InitRegisters();		// set the initial register values

    printf("Before \" space->RestoreState(); \" in progtest.cc \n");
    space->RestoreState();		// load page table register
    printf("After \" space->RestoreState(); \" in progtest.cc \n");

    // Nafee : Creating an instance of ThreadSafeSynchronizedConsole
    //threadSafeSynchronizedConsole = new ThreadSafeSynchronizedConsole();

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}
