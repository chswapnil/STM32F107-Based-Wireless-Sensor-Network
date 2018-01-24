#include "Error.h"


CPU_VOID PktError(CPU_INT08U errorType)
{
  switch(errorType)
  {
  case P1BYTEERROR:
    BSP_Ser_Printf("\a***ERROR: Bad Preamble byte 1\n");
    break;
  case P2BYTEERROR:
    BSP_Ser_Printf("\a***ERROR: Bad Preamble byte 2\n");
    break;
  case P3BYTEERROR:
    BSP_Ser_Printf("\a***ERROR: Bad Preamble byte 3\n");
    break;
  case CHKSUMERROR:
    BSP_Ser_Printf("\a***ERROR: Checksum error\n");
    break;
  case PKTLENERROR:
    BSP_Ser_Printf("\a***ERROR: Bad Packet Size\n");
    break;
  case DSTADRERROR:
    BSP_Ser_Printf("***INFO: Not My Address\n");
    break;
  case UNKPKTERROR:
    BSP_Ser_Printf("\a***ERROR: Unknown Message Type\n");
    break;
  default:
    BSP_Ser_Printf("\a***ERROR: Unknown Error\n");
    break;
  }
  return;
}