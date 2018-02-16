#include "includes.h"
#include "BfrPair.h"

#ifndef PKTPARSER_H_
#define PKTPARSER_H_

/*
The keyword extern specifies that the semaphores are declared in the other file here which is PktParser.c.
Whichever file includes this header will indirectly include the extern declaration.
Hence extern declaration is not required in payload.c because we have included the pktparser.h file in it.
*/

extern OS_SEM	openPayloadBfrs;	  /* Binary Semaphore 1 */
extern OS_SEM	closedPayloadBfrs;	  /* Binary Semaphore 2 */


CPU_VOID CreateParserTask(CPU_VOID);

CPU_VOID ParsePkt(CPU_VOID*data);

#endif