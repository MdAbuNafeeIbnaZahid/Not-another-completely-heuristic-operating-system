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
#include "MemoryManager.h"

void 
ConsoleTest (char *in, char *out);

extern void processThread(int execNameVirAddr); //Me: for the purpose of CreateProcess()
//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;


    if (executable == NULL) {
    	printf("Unable to open file %s\n", filename);
    	return;
    }

    space = new AddrSpace(executable);   

    currentThread->space = space;
    currentThread->processId = processManager->Alloc((void*)currentThread); ///me: todo: assign this process a process id


    // Nafee : Sid sir instructed us not to delete executable
    //delete executable;			// close file



    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    ///me:todo: when this process is context switched in after being switched off?
    ///the registers of kernel mode will have garbage value. That's why I have written 
    ///the following code
    /*currentThread->processId = processManager->Alloc((void*)t);
    currentThread->Fork(processThread, currentThread->processId);
    scheduler->FindNextToRun();
    scheduler->Run();
    */
    ///Correction: I actually don't have to do this. 
    ///Reason: suppose this thread switches context for the first time.
    ///then the current host machine register values and stack allocated 
    ///to nachos will be stored in stack* and machineState[]. 

    //ConsoleTest(NULL, NULL);
    //printf("before running mips\n");
    machine->Run();			// jump to the user progam


    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}



int 
CreateProcess(char *fileName){
    /**********Me: loads the executable into the memory of MIPS machine.********/

    OpenFile *program = fileSystem->Open(fileName);
    if (program == NULL) {
        printf("Unable to open file %s|\n", fileName);
        return 0;
    }
    else printf("opened file %s\n", fileName);

    AddrSpace *space = new AddrSpace(program);
    delete program; ///the program is loaded into the MIPS machine. We don't need this 
        ///file anymore. So close the file.
    if(!space->success){
        return 0;
    }
    /****************************************************************************/

    
    /**********Me: todo: create  a thread, and run the process in this thread.*********/
    Thread* t = new Thread("New Process");
    t->space = space;
    t->processId = processManager->Alloc((void*)t); ///return value in [1, max#process], 
    ///-1 in case of failure due to crossed the limit of max#process.

    myDebug('e', "new process id: %d\n", t->processId);
    if(t->processId == -1){
        printf("Cannot create the process. Becoz Crossed maximum process limit\n");
        delete t;
        return 0;
    }

    t->Fork(processThread, t->processId);///Me: we must fork. Reason: in JUST_CREATED state,  machineState[] is uninitialized, and stack*
       ///does not contain any stack. The only way to initialize machineState[] and stack* is 
    ///calling the Fork(). 
    ///Why is it necessary to initialize machineState and stack?
    ///Reason: a thread running a user program can run in two modes: user mode and kernel mode.
    ///in kernel mode, the registers are copied from machineState into host's processor by SWITCH
    ///ESP is set to point to stack*. 
    ///So if we don't initialize machineState, it will have gargbage value, and process NACHOS will crash 
    /// at the time when the thread is context switched in. 
    /***********************************************************************************/

    return t->processId;
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;
Lock* consoleLock;
//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { 
//    printf("inside read avail: %c\n",(char)arg );
   readAvail->V(); 
}
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
