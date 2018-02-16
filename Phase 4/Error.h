#include "includes.h"

//TYPES OF ERRORS
#define P1BYTEERROR   -1
#define P2BYTEERROR   -2
#define P3BYTEERROR   -3
#define CHKSUMERROR   -4
#define PKTLENERROR   -5
#define DSTADRERROR   -6
#define UNKPKTERROR   -7


#ifndef ERROR_H_
#define ERROR_H_

CPU_VOID HandleError(CPU_INT08S errorType,CPU_INT08U* msgBfr);

#endif