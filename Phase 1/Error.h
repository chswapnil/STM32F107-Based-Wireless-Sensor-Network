#include "includes.h"

//TYPES OF ERRORS
#define P1BYTEERROR   0
#define P2BYTEERROR   1
#define P3BYTEERROR   2
#define CHKSUMERROR   3
#define PKTLENERROR   4
#define DSTADRERROR   5
#define UNKPKTERROR   6

#ifndef ERROR_H_
#define ERROR_H_

CPU_VOID PktError(CPU_INT08U state);

#endif