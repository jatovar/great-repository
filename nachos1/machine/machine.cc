// machine.cc 
//	Routines for simulating the execution of user programs.
//
//  DO NOT CHANGE -- part of the machine emulation
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "machine.h"
#include "system.h"
static int fallo = 0;
// Textual names of the exceptions that can be generated by user program
// execution, for debugging.
static char* exceptionNames[] = { "no exception", "syscall", 
				"page fault/no TLB entry", "page read only",
				"bus error", "address error", "overflow",
				"illegal instruction" };

//----------------------------------------------------------------------
// CheckEndian
// 	Check to be sure that the host really uses the format it says it 
//	does, for storing the bytes of an integer.  Stop on error.
//----------------------------------------------------------------------

static
void CheckEndian()
{
    union checkit {
        char charword[4];
        unsigned int intword;
    } check;

    check.charword[0] = 1;
    check.charword[1] = 2;
    check.charword[2] = 3;
    check.charword[3] = 4;

#ifdef HOST_IS_BIG_ENDIAN
    ASSERT (check.intword == 0x01020304);
#else
    ASSERT (check.intword == 0x04030201);
#endif
}

//----------------------------------------------------------------------
// Machine::Machine
// 	Initialize the simulation of user program execution.
//
//	"debug" -- if TRUE, drop into the debugger after each user instruction
//		is executed.
//----------------------------------------------------------------------

Machine::Machine(bool debug)
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        registers[i] = 0;
    mainMemory = new char[MemorySize];
    for (i = 0; i < MemorySize; i++)
      	mainMemory[i] = 0;
#ifdef USE_TLB
    tlb = new TranslationEntry[TLBSize];
    for (i = 0; i < TLBSize; i++)
	tlb[i].valid = FALSE;
    pageTable = NULL;
#else	// use linear page table
    tlb = NULL;
    pageTable = NULL;
#endif
    numeroDePaginas = 0;
    singleStep = debug;
    CheckEndian();
}

//----------------------------------------------------------------------
// Machine::~Machine
// 	De-allocate the data structures used to simulate user program execution.
//----------------------------------------------------------------------

Machine::~Machine()
{
    delete [] mainMemory;
    if (tlb != NULL)
        delete [] tlb;
}

//----------------------------------------------------------------------
// Machine::RaiseException
// 	Transfer control to the Nachos kernel from user mode, because
//	the user program either invoked a system call, or some exception
//	occured (such as the address translation failed).
//
//	"which" -- the cause of the kernel trap
//	"badVaddr" -- the virtual address causing the trap, if appropriate
//----------------------------------------------------------------------

void
Machine::RaiseException(ExceptionType which, int badVAddr)
{
    DEBUG('m', "Exception: %s\n", exceptionNames[which]);
    
//  ASSERT(interrupt->getStatus() == UserMode);
    registers[BadVAddrReg] = badVAddr;
    DelayedLoad(0, 0);			// finish anything in progress
    interrupt->setStatus(SystemMode);
    ExceptionHandler(which);		// interrupts are enabled at this point
    interrupt->setStatus(UserMode);
}

//----------------------------------------------------------------------
// Machine::Debugger
// 	Primitive debugger for user programs.  Note that we can't use
//	gdb to debug user programs, since gdb doesn't run on top of Nachos.
//	It could, but you'd have to implement *a lot* more system calls
//	to get it to work!
//
//	So just allow single-stepping, and printing the contents of memory.
//----------------------------------------------------------------------

void Machine::Debugger()
{
    char *buf = new char[80];
    int num;

    interrupt->DumpState();
    DumpState();
    printf("%d> ", stats->totalTicks);
    fflush(stdout);
    fgets(buf, 80, stdin);
    if (sscanf(buf, "%d", &num) == 1)
	runUntilTime = num;
    else {
	runUntilTime = 0;
	switch (*buf) {
	  case '\n':
	    break;
	    
	  case 'c':
	    singleStep = FALSE;
	    break;
	    
	  case '?':
	    printf("Machine commands:\n");
	    printf("    <return>  execute one instruction\n");
	    printf("    <number>  run until the given timer tick\n");
	    printf("    c         run until completion\n");
	    printf("    ?         print help message\n");
	    break;
	}
    }
    delete [] buf;
}
 
//----------------------------------------------------------------------
// Machine::DumpState
// 	Print the user program's CPU state.  We might print the contents
//	of memory, but that seemed like overkill.
//----------------------------------------------------------------------

void
Machine::DumpState()
{
    int i;
    
    printf("Machine registers:\n");
    for (i = 0; i < NumGPRegs; i++)
	switch (i) {
	  case StackReg:
	    printf("\tSP(%d):\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;
	    
	  case RetAddrReg:
	    printf("\tRA(%d):\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;
	  
	  default:
	    printf("\t%d:\t0x%x%s", i, registers[i],
		   ((i % 4) == 3) ? "\n" : "");
	    break;
	}
    
    printf("\tHi:\t0x%x", registers[HiReg]);
    printf("\tLo:\t0x%x\n", registers[LoReg]);
    printf("\tPC:\t0x%x", registers[PCReg]);
    printf("\tNextPC:\t0x%x", registers[NextPCReg]);
    printf("\tPrevPC:\t0x%x\n", registers[PrevPCReg]);
    printf("\tLoad:\t0x%x", registers[LoadReg]);
    printf("\tLoadV:\t0x%x\n", registers[LoadValueReg]);
    printf("\n");
}

//----------------------------------------------------------------------
// Machine::ReadRegister/WriteRegister
//   	Fetch or write the contents of a user program register.
//----------------------------------------------------------------------

int Machine::ReadRegister(int num)
    {
	ASSERT((num >= 0) && (num < NumTotalRegs));
	return registers[num];
    }

void Machine::WriteRegister(int num, int value)
    {
	ASSERT((num >= 0) && (num < NumTotalRegs));
	// DEBUG('m', "WriteRegister %d, value %d\n", num, value);
	registers[num] = value;
    }

//Algoritmos de reemplazo
void Machine::fifo(int pagErronea){	//Algoritmo FIFO
	int *vict = 0;	
	int fisica, logica;

	if (marcos < NumPhysPages){ 		//si hay marcos disponibles
		logica = pagErronea*PageSize;		//Se calcula la dir logica
		fisica = marcos*PageSize;		//Se calcula la dir fisica
		pageTable[pagErronea].physicalPage = marcos;	//Se ubica la pagina que genero el fallo y se actualiza con el marco nuevo
		marcos++;
	} else {		//Si ya no hay marcos disponibles
		vict = (int*)cadReferencia->Remove();			//Se saca de la cadena de referencia la victima
		fisica = pageTable[*vict].physicalPage*PageSize;	//Se obtiene la direccion fisica
		logica = pageTable[*vict].virtualPage*PageSize;		//Se obtiene la direccion logica
		if(pageTable[*vict].use){				//Si la pagina donde se ubica la victima se modifico
			stats->numDiskWrites++;				//Se actualizan las estadisticas
			swap->WriteAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se escribe en memoria la pagina nueva
		}
		logica = pagErronea*PageSize;
		pageTable[*vict].valid = false;	//Se libera este marco
		pageTable[pagErronea].physicalPage = pageTable[*vict].physicalPage;
	}
	stats->numPageFaults++;	//Se actualizan las estadisticas
	stats->numDiskReads++;
	swap->ReadAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se lee en memoria una pagina
	pageTable[pagErronea].valid = true;	//Se marca como valida
	vict = new int(pagErronea);		//Se indica a victima quien genero el fallo
	printf("Pagina Erronea: %d\n", *vict);
	cadReferencia->Append(vict);		//Se inserta al final la victima en la cadena de referencia
}

void Machine::lru(int pagErronea){	//Algoritmo LRU
	int* vict = new int(0);	
	int fisica, logica;
	int i, max = 0, j;
		
	fallo++;
	if (marcos < NumPhysPages){ 		//si hay marcos disponibles
		logica = pagErronea*PageSize;		//Se calcula la dir logica
		fisica = marcos*PageSize;		//Se calcula la dir fisica
		pageTable[pagErronea].physicalPage = marcos;	//Se ubica la pagina que genero el fallo y se actualiza con el marco nuevo
		contadores[marcos] = 0;
		pagina[marcos] = pagErronea;
		marcos++;
	} else {		//Si ya no hay marcos disponibles
		for(i = 0; i < NumPhysPages; i++){ //busca la pagina menos usada
			if(contadores[i] > max){
				j = i;
				max = contadores[i];
				*vict = pagina[i];
			}
		}
		fisica = pageTable[*vict].physicalPage*PageSize;	//Se obtiene la direccion fisica
		logica = pageTable[*vict].virtualPage*PageSize;		//Se obtiene la direccion logica
		if(pageTable[*vict].use){				//Si la pagina donde se ubica la victima se modifico
			stats->numDiskWrites++;				//Se actualizan las estadisticas
			swap->WriteAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se escribe en memoria la pagina nueva
		}
		logica = pagErronea*PageSize;
		pageTable[*vict].valid = false;	//Se libera este marco
		pageTable[pagErronea].physicalPage = pageTable[*vict].physicalPage;
		pagina[j] = pagErronea;
	}
	stats->numPageFaults++;	//Se actualizan las estadisticas
	stats->numDiskReads++;
	swap->ReadAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se lee en memoria una pagina
	pageTable[pagErronea].valid = true;	//Se marca como valida
	printf("Pagina Erronea: %d\n", pagErronea);
}

void Machine::optimo(int pagErronea){	//Algoritmo Optimo
	int *vict = 0;	
	int fisica, logica;

	if (marcos < NumPhysPages){ 		//si hay marcos disponibles
		logica = pagErronea*PageSize;		//Se calcula la dir logica
		fisica = marcos*PageSize;		//Se calcula la dir fisica
		pageTable[pagErronea].physicalPage = marcos;	//Se ubica la pagina que genero el fallo y se actualiza con el marco nuevo
		marcos++;
	} else {		//Si ya no hay marcos disponibles
		vict = (int*)cadReferencia->Remove();			//Se saca de la cadena de referencia la victima
		fisica = pageTable[*vict].physicalPage*PageSize;	//Se obtiene la direccion fisica
		logica = pageTable[*vict].virtualPage*PageSize;		//Se obtiene la direccion logica
		if(pageTable[*vict].use){				//Si la pagina donde se ubica la victima se modifico
			stats->numDiskWrites++;				//Se actualizan las estadisticas
			swap->WriteAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se escribe en memoria la pagina nueva
		}
		logica = pagErronea*PageSize;
		pageTable[*vict].valid = false;	//Se libera este marco
		pageTable[pagErronea].physicalPage = pageTable[*vict].physicalPage;
	}
	stats->numPageFaults++;	//Se actualizan las estadisticas
	stats->numDiskReads++;
	swap->ReadAt(&(machine->mainMemory[fisica]), PageSize, logica);	//Se lee en memoria una pagina
	pageTable[pagErronea].valid = true;	//Se marca como valida
	vict = new int(pagErronea);		//Se indica a victima quien genero el fallo
	printf("Pagina Erronea: %d\n", *vict);
	cadReferencia->Append(vict);		//Se inserta al final la victima en la cadena de referencia
}
