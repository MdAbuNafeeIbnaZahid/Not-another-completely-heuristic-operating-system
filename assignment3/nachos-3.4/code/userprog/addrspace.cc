// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.


#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

/*
Me:

What does this constructor do?

It loads the executable program into memory. 

Before loading, it must allocate pages in memory from the memory manager.

Before allocating memory, we must know how many pages we are needed to allocate

To know how many pages is needed, we must know how much memory is taken by the process.

To know the memory size required, we need to know the size of the code segment, data segment,
stack segment. stack size is predefined in "UserStackSize" variable. the others' size
 is known from the header of the executable file. 

 So we must first load the header.


[more precise steps about loading: Computer Organization & Design, By patterson, chapt 2, section 2.12, page129]
*/
AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;
    success = true;

    /**********************Me:Load Header**********************************/
    /*Me: Loads the header of the executable file into noffH.
    Q: Why not loaded into the MIPS memory?
    Ans: The code is of kernel space, and kernel space codes are executed in host's
    machine. So we must not load the header into MIPS machine's memory.
    */
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

    //Me: the noff executable program may be little endian. But
    //we are on a big endian machine(intel microprocessor).
    //So checks whether the program is little endian.
    //If yes, then convert to big endian.
    if ((noffH.noffMagic != NOFFMAGIC) &&
        (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

/*******************************************************************/

/*********************Me:calc of total required #pages**********************/

// how big is address space?
    /**Me: reads the header loaded right now to determine how many
        pages are needed for the process.
    */
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize; // we need to increase the size
    // to leave room for the stack
    numPages = divRoundUp(size, PageSize); //Me: numPages = ceil(size of the process / size of each page)
    size = numPages * PageSize; //Me: the memory needed for the pages.

    //ASSERT(numPages <= NumPhysPages);       // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

/*******************************************************************/

/******************Me: create & init page table**************************/

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);

    ///Me:Why do we need to disable interrupt?
    ///to avoid race condition. Suppose I have checked that I have enough free pages for the process.
    ///then context switch occured, and another process took all the pages. But when I will allocate
    ///memory for the pages, it will fail.
    ///but for nachos, it will works fine If we don't disable interrupts. Because no interrupt will
    ///generated here. In nachos, interrupts are generated only 3 cases [mentioned in the machine/interrupt.h]
    IntStatus oldLevel = interrupt->SetLevel(IntOff);   // disable interrupts

    ///Me: Todo: checks whether the required number of pages are free
    int freePages = memoryManager->getNumFreePages();


    // Nafee : Commenting out this if block as instructed by Sid sir 
    // if(freePages < numPages){ //physical memory does not have enough space for the process
    //     printf("Error:AddrSpace:: not enougth memory for the process\n");
    //     ///Me: Todo: probably I have to raise exception to terminate the process.
    //     success =  false;
    //     return ;
    // }
    ///Me: is it possible to



    // first, set up the translation
    pageTable = new TranslationEntry[numPages]; //Me: Creates page table. Pagetable resides in host's machine!!!
    //Me: initialize the newly created page table
    for (i = 0; i < numPages; i++) {

        pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
        

        // pageTable[i].physicalPage = memoryManager->AllocPage(); //Me:Todo: allocate a page using memory manager
        // Nafee : as instructed by Sid sir
        pageTable[i].physicalPage = -1;
        

        ///Me: I am full confident that the allocation will be successful. Because I have checked
        ///whether I have enough free pages above. And no context switch will occur, as interrupt disabled
        ///
        ///Default code pageTable[i].physicalPage = i;. Why have we changed it?
        ///because i-th physical page may not be available. Which page available?
        ///We need to consult memoryManager.



        
        // pageTable[i].valid = TRUE; ///Me: will be FALSE if memory allocation fails.
        // Nafee : As instructed by Sid sir
        pageTable[i].valid = FALSE;



        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
    }


    (void) interrupt->SetLevel(oldLevel);   // re-enable interrupts
/*******************************************************************/
/********************Me:loads code & data segment***************************/
    
    // Nafee : Sid sir said us to comment zeroing entire machine->mainMemory
    // Nafee : So I am commenting out this for loop block
    ///Me: Todo: zero out only the allocated pages, to zero out the uninitialized data segment.
    // for (int i = 0; i < numPages; ++i){
    //     char* locationToClean = &machine->mainMemory[ PageSize * pageTable[i].physicalPage ];
    //     int numBytesToClean = PageSize;

    //     bzero(locationToClean, numBytesToClean);
    // }





    // Nafee : Sid sir said us to comment loading code segment, data segment
    
    //printf("before allocating code seg\n");
    // then, copy in the code and data segments into memory
    // if (noffH.code.size > 0) {
    //     DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
    //           noffH.code.virtualAddr, noffH.code.size);

    //     ///Me: Todo: copy the code segment into mips's memory
    //     char* buff = new char[noffH.code.size  + 10];
    //     executable->ReadAt(buff, noffH.code.size, noffH.code.inFileAddr);
    //     loadIntoMemory(buff, noffH.code.size, noffH.code.virtualAddr );
    //     delete buff;

    //     //executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),    noffH.code.size, noffH.code.inFileAddr);
    // }
    // if (noffH.initData.size > 0) {
    //     DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
    //           noffH.initData.virtualAddr, noffH.initData.size);

    //     ///Me: Todo: copy the data segment into mips's memory
    //     char* buff = new char[noffH.initData.size  + 10];
    //     executable->ReadAt(buff, noffH.initData.size, noffH.initData.inFileAddr);
    //     loadIntoMemory(buff, noffH.initData.size, noffH.initData.virtualAddr );
    //     delete buff;

    //     //executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]), noffH.initData.size, noffH.initData.inFileAddr);
    // }
    /*******************************************************************/

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    

   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

//Me: copy() & getPhysicalMemPointer()
void copy(char* dest, char* source, int size){
    while (size--){
        *dest = * source;
        dest++;
        source++;
    }
}

char* getPhysicalMemPointer(int virtualAddr, TranslationEntry* pageTable){
    int vpn = (unsigned) virtualAddr / PageSize;
    int offset = (unsigned) virtualAddr % PageSize;
    int physicalAddr = pageTable[vpn].physicalPage  * PageSize + offset;
    return &machine->mainMemory[physicalAddr];
}


///Me: the following code is written by me.
void AddrSpace::loadIntoMemory(char* buff, int size, int startVirAddr){
    /*TranslationEntry * t = machine->pageTable;
    int s =  machine->pageTableSize;
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

    for (int i = 0; i < size; ++i)
    {
        //    bool WriteMem(int addr, int size, int value);
        machine->WriteMem(startVirAddr++, 1, *buff++);
    }
        machine->pageTable= t;
        machine->pageTableSize = s;
        */
    
    int vpn = (unsigned) startVirAddr / PageSize;
    int offset = (unsigned) startVirAddr % PageSize;

    int upperFraction = (PageSize - offset);
    if(size <= upperFraction){
        copy(getPhysicalMemPointer(startVirAddr, pageTable), buff, size);
    }
    else{
        copy(getPhysicalMemPointer(startVirAddr, pageTable), buff, upperFraction);

        buff += upperFraction;
        size -= upperFraction;
        vpn ++;
        while (1){

            if(PageSize > size){
                copy(getPhysicalMemPointer(vpn * PageSize, pageTable), buff, size);
                break;
            }
            else{
                copy(getPhysicalMemPointer(vpn * PageSize, pageTable), buff, PageSize);
                buff += PageSize;
                size -= PageSize;
                vpn++;
            }

        }
    }

}


///may contain buss. not tested
int copyFromVirMem(int virAddr, int size, char* buff){ //much like loadIntoMemory() from addrspace
    int vpn = (unsigned) virAddr / PageSize;
    int offset = (unsigned) virAddr % PageSize;
    TranslationEntry* pageTable = machine->pageTable;

    int upperFraction = (PageSize - offset);


    int lastPageNum = vpn + ( size <= upperFraction? 0 : (size - upperFraction)/PageSize + (((size - upperFraction)%PageSize) != 0) );
    if(vpn < 0 || lastPageNum >= machine->pageTableSize) {
        printf("copyFromVirMem: Invalid virtual address\n");
        return -1;
    }
    


    
    if(size <= upperFraction){
        copy(buff, getPhysicalMemPointer(virAddr, pageTable), size);
    }
    else{
        copy(buff, getPhysicalMemPointer(virAddr, pageTable),  upperFraction);

        buff += upperFraction;
        size -= upperFraction;
        vpn ++;
        while (1){

            if(PageSize > size){
                copy( buff, getPhysicalMemPointer(vpn * PageSize, pageTable), size);
                break;
            }
            else{
                copy(buff, getPhysicalMemPointer(vpn * PageSize, pageTable),  PageSize);
                buff += PageSize;
                size -= PageSize;
                vpn++;
            }

        }
    }
}


