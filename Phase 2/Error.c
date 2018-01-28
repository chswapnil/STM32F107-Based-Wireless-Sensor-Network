/*
Author: Swapnil Chaughule
*/
#include "Error.h"

/*
//TYPES OF ERRORS
#define P1BYTEERROR   -1
#define P2BYTEERROR   -2
#define P3BYTEERROR   -3
#define CHKSUMERROR   -4
#define PKTLENERROR   -5
#define DSTADRERROR   -6
#define UNKPKTERROR   -7
*/

CPU_VOID HandleError(CPU_INT08S errorType,CPU_INT08U* msgBfr)
{
  switch(errorType)
  {
  case P1BYTEERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Bad Preamble byte 1\n");
    break;
  case P2BYTEERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Bad Preamble byte 2\n");
    break;
  case P3BYTEERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Bad Preamble byte 3\n");
    break;
  case CHKSUMERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Checksum error\n");
    break;
  case PKTLENERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Bad Packet Size\n");
    break;
  case DSTADRERROR:
    sprintf((CPU_CHAR*)msgBfr,"***INFO: Not My Address\n");
    break;
  case UNKPKTERROR:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Unknown Message Type\n");
    break;
  default:
    sprintf((CPU_CHAR*)msgBfr,"\a***ERROR: Unknown Error\n");
    break;
  }
  return;
}
