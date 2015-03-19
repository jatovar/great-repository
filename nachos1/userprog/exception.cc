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
extern int algoritmo;

int FIFO(int nf)
{
    return (nf) % NumPhysPages;
}

int Reloj()
{
    int index = -1;
    int frame = -1;
    bool band = TRUE;
    
    do{
        frame = FIFO(currentThread->space->nextFrame);
        index = currentThread->space->PageLookUp(frame);
        if(index != -1)
        {
            if(currentThread->space->pageTable[index].use == TRUE)
            {
                currentThread->space->pageTable[index].use = FALSE;
                currentThread->space->nextFrame = currentThread->space->nextFrame + 1;
                
            }else
            {
                band = FALSE;
            }
        }
                
    }while(band);
    return currentThread->space->pageTable[index].physicalPage;
}
//----------------------------------------------------------------------
// PageFaultHandler
// 	Método para manejar el fallo de página, del actual proceso en ejecución, se manda a cargar la 
//	página que se necesita desde la memoria de intercambio
//	
void PageFaultHandler()
{   
    int ipageFault = machine->ReadRegister(BadVAddrReg)/PageSize;
    printf("Van %d fallos de pagina\n",stats->numPageFaults);
    //Se divide la dirección virtual / tamaño de la página (Eso nos da como resultado el indice de la pagina que provoco el fallo)    
    int victima = -1;
    if(stats->numPageFaults <= NumPhysPages)//Esquema de paginacion por demanda pura
    {
          //Se declara una variable estática para guardar el marco siguiente
       // printf("Dirección virtual de la página que provocó el fallo = %d\n", machine->ReadRegister(BadVAddrReg));
       // printf("Indice de la página que provoco el fallo = %d\n", ipageFault);
       // printf("Indice del marco al que se va a cargar la página = %d\n", nextFrame);           
        victima = FIFO(currentThread->space->nextFrame);
        currentThread->space->LoadPage(ipageFault, victima);//Se carga en memoria principal el indice de la pagina y el marco
        printf("Se metio la pagina: %d en el marco: %d\n",ipageFault,victima);

    }else
    {
        switch(algoritmo)
        {
            case 1: 
                victima = Reloj();        
                printf("Victima reloj = %d\n",victima);
                break;
            case 2:
                break;
            case 3:
                break;
        }
        currentThread->space->SwapPage(ipageFault,victima);
        
    }
    currentThread->space->nextFrame++;
        
}


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
    int type = machine->ReadRegister(2);
    if(which == PageFaultException) //Se captura la excepcion ya que se ignoro la traduccion desde la primer página(Paginacion por demanda pura)
    {
        stats->numPageFaults++;// Se aumenta el numero de fallos de pagina
        PageFaultHandler();  
    }else
        {
            if ((which == SyscallException) && (type == SC_Halt)) 
            {
                DEBUG('a', "Shutdown, initiated by user program.\n");
                interrupt->Halt();

            }else 
                {
                    printf("Unexpected user mode exception %d %d\n", which, type);
                    ASSERT(FALSE);
                }
        }
}
