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
///me: prototypes of  auxiliary functions
int copyStringFromMem(int virtualAddr, char* buff, int size);
int copyStringFromMem(int virtualAddr, char* buff);
char* getPhysicalMemPointer(int virtualAddr,TranslationEntry* pageTable); //from addrspace.cc

///me:proto types of handler
int sysExecHandler(unsigned int execNameVirAddr);
int sysExitHandler( int returnValue);
int sysReadHandler(int virAddrBuff, int size, int fileId);
void sysWriteHandler(int virAddrBuff, int size, int fileId);


void
ExceptionHandler(ExceptionType which)
{

    ///Me: should it be atomic?
    //no.
    //IntStatus oldLevel;

    ///Me: Why register 2?
    ///Reason: in MIPS processor, a procedure puts its return value into register 2 or 3.
    ///Again Why R4-R7 is used for arguements?
    ///Reason: in MIPS processor, the arguements are placed in R4-R7.
    ///[ref: computer organization and design by patterson, 5th edn, p105]
    int type = machine->ReadRegister(2);

    if(which == SyscallException){
        int ret = 0;
        switch (type) {
            default:
                printf("Invalid system call\n");
                break;
            case SC_Halt:
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();
                break;
            case SC_Exit:
                //oldLevel = interrupt->SetLevel(IntOff);
                ///no need to be atomic.
                myDebug('e', "\n\nexiting : %d\n", currentThread->processId);
                sysExitHandler(machine->ReadRegister(4));
                //(void) interrupt->SetLevel(oldLevel);
                break;
            case SC_Exec:
                DEBUG('a', "EXEC system call.\n");
                myDebug('e', "entering exec\n");
                ret = sysExecHandler(machine->ReadRegister(4));
                machine->WriteRegister(2,ret); ///Me: return the process id to the caller process.
                break;
            case SC_Join:
                break;
            case SC_Create:
                break;
            case SC_Open:
                break;
            case SC_Read:
                ret = sysReadHandler(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                machine->WriteRegister(2,ret);///Me: return the number of bytes read to the caller process.
                break;
            case SC_Write:
                sysWriteHandler(machine->ReadRegister(4), machine->ReadRegister(5), machine->ReadRegister(6));
                break;
            case SC_Close:
                break;
            case SC_Fork:
                break;
            case SC_Yield:
                break;
        }
        ///Me:todo: updates PC. Because if execution returns to the same instruction,
        ///the trap call will happen again. there are 3 registers related to PC
        machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
        machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
        machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+8);
        

    }
    else if(which == PageFaultException){

        

        // printf("\nPage Fault...\n");
        int faultingAddress = machine->ReadRegister(39);
        // printf("\nfaultingAddress = %d\n", faultingAddress);
        int vpn = faultingAddress / PageSize;
        // printf("\nvpn = %d\n", vpn);
        int physicalPageNo = -1;
        if ( memoryManager->getNumFreePages() > 0 )
        {
            printf("\n Free page is available \n");
            //physicalPageNo = memoryManager->AllocPage();
            physicalPageNo = memoryManager->Alloc((int)currentThread->processId, &machine->pageTable[vpn]);
        }
        else
        {
            // will force to free a page
            printf("\n forcing a page out \n");
            physicalPageNo = memoryManager->AllocByForce();

            

            // handle swap file cases here

        }

        printf("\n faultingAddress = %d \n", faultingAddress);
        printf("\n physicalPageNo = %d \n", physicalPageNo);

        // printf("\n before calling loadIntoFreePage \n");
        currentThread->space->loadIntoFreePage( faultingAddress, physicalPageNo );


        // Nafee : We will not exit process anymore 
        //sysExitHandler(-1);
    }
    else if(which == ReadOnlyException){
        printf("ReadOnlyException\n");
        sysExitHandler(-1);
    }else if(which == BusErrorException){
        printf("BusErrorException\n");
        sysExitHandler(-1);
    }
    else if(which == AddressErrorException){
        printf("AddressErrorException\n");
        sysExitHandler(-1);
    }
    else if(which == OverflowException){
        printf("OverflowException\n");
        sysExitHandler(-1);
    }
    else if(which == IllegalInstrException){
        printf("IllegalInstrException\n");
        sysExitHandler(-1);
    }
    else if(which == NumExceptionTypes){
        printf("NumExceptionTypes\n");
        sysExitHandler(-1);
    }
    
    
    /*
    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    } else {
		printf("Unexpected user mode exception %d %d\n", which, type);
		ASSERT(FALSE);
    }
     */
}
/********************************EXEC system call*******************************/
int copyStringFromMem(int virtualAddr, char* buff){
    /*this algo is inefficient. But simple
    int len = 0;
    do{        
        int x;
        if(!machine->ReadMem(virtualAddr++, 1, &x)){
            printf("Wrong Address\n");
            return 0;
        } 
        len ++;
        *buff = (char) x;

   }while(*buff++ != 0);
   return len;
*/

    TranslationEntry* pageTable = machine->pageTable;
    unsigned int numPages = machine->pageTableSize;
    int maxVirAddr = numPages * (PageSize + 1) - 1;

    int len;
    
    for (len = 0; ; virtualAddr++, len++, buff++)
    {
        if(virtualAddr > maxVirAddr) {
            printf("Invalid address of the buffer\n");
            return -1;
        }
        
        *buff = *getPhysicalMemPointer(virtualAddr, pageTable);
        if(!*buff) break;
    }

    return (len - 1); //minus one because null pointer.
    
}

int copyStringFromMem(int virtualAddr, char* buff, int size){
    TranslationEntry* pageTable = machine->pageTable;
    unsigned int numPages = machine->pageTableSize;
    int maxVirAddr = numPages * (PageSize + 1) - 1;

    int len;
    
    for (len = 0; len < size ; virtualAddr++, len++, buff++)
    {
        if(virtualAddr > maxVirAddr) {
            printf("Invalid address of the buffer\n");
            return -1;
        }
        
        *buff = *getPhysicalMemPointer(virtualAddr, pageTable);
    }
    return len;
}

void processThread(int execNameVirAddr){
    currentThread->space->InitRegisters();///Me: initializes the MIPS's registers before starting execution
    currentThread -> space -> RestoreState();   
    machine->Run();
}

int sysExecHandler(unsigned int execNameVirAddr){
    ///Me: todo: retrieve the file name
    int maxFileNameSize = (machine->pageTableSize * (PageSize + 1)) - execNameVirAddr;
    char *fileName = new char[maxFileNameSize + 10];
    if(copyStringFromMem(execNameVirAddr, fileName) < 1){
        printf("Invalid file name\n");
        return 0;///as mentioned in the offline, zero indicates the process is not created.
    }

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
    delete fileName;
    if(!space->success){ ///check whether memory allocation for this program was successfull.
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

/*********************************\EXEC system call***********************************/


int sysExitHandler(int returnValue){
    //frees the allocated pages. 
    int ptSize = machine->pageTableSize;
    TranslationEntry* pageTable = machine->pageTable;
    
    int pid = currentThread->processId;
    printf("%d: Return value: %d\n", pid, returnValue); 

    processManager->Release(pid);

    ///Me: todo: free the allocated page frames
    for (int i = 0; i < machine->pageTableSize; ++i) {
        if ( machine->pageTable[i].valid ) // Nafee : only freeing the valid pages as 
            // sid sir instructed
        {
            // printf("\n Got a valid page \n");
            memoryManager->FreePage(machine->pageTable[i].physicalPage);
        }
    }
    myDebug('e',"total processes: %d\n", processManager->currentItems());

    if(processManager->currentItems() == 0) interrupt->Halt(); ///why should I halt? 
    ///console is scheduled in interrupt. So, console will still be running. and system
    ///will never halt.
    ///race condition will never arise. As there is no process to create race condition
    currentThread->Finish();
}

/********************************Write/Read system call******************************/
/* Write "size" bytes from "buffer" to the open file. */
void sysWriteHandler(int virAddrBuff, int size, int fileId){
    myDebug('e', "sysWriteHandler called\n");
    if(fileId != ConsoleOutput){
        printf("Only console I/O is implemented in this offline.\n");
        return;
    }
    
    char* buff = new char[size + 10];
    copyStringFromMem(virAddrBuff, buff, size);

    myConsole.lock->Acquire();
    for (int i = 0; i < size; ++i)
    {
        myConsole.device->PutChar(buff[i]);
        myConsole.writeDone->P();
    }
    myConsole.lock->Release();
}

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int sysReadHandler(int virAddrBuff, int size, int fileId){
    if(fileId != ConsoleInput){
        printf("Only console I/O is implemented in this offline.\n");
        return -1;
    }

    int readBytes = 0;
    char ch = 'a';
    myConsole.lock->Acquire();
    while(readBytes < size && ch != '\n' ){
        myConsole.readAvail->P();
        ch = myConsole.device->GetChar();
        if(ch == '\n' || ch == '\r'){
            machine->WriteMem(virAddrBuff, 1, 0);
            break;
        }
        machine->WriteMem(virAddrBuff, 1, ch);
        virAddrBuff++;
        readBytes++;
    }
    myConsole.lock->Release();
    return readBytes;
}
/******************************************************************************/