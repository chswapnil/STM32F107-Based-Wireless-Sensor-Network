#include "includes.h"
#ifndef BFR_H_
#define BFR_H_

/*------Buffer Structers------*/
typedef struct
{
  volatile CPU_BOOLEAN closed;  /* -- True if buffer has data ready to be process, volatile because can be changed rapidly*/
  CPU_INT16U size;     /* -- The capacity of the buffer in bytes*/
  CPU_INT16U putIndex; /* -- The position where the next byte will be added*/
  CPU_INT16U getIndex; /* -- The position where the next byte will be removed*/
  CPU_INT08U *buffer;  /* -- The address of the buffer space*/
}Buffer;


/*------Buffer Functions-------*/

void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size);

void BfrReset(Buffer *bfr);

CPU_BOOLEAN BfrClosed(Buffer *bfr);

void BfrClose(Buffer *bfr);

void BfrOpen(Buffer *bfr);

CPU_BOOLEAN BfrFull(Buffer *bfr);

CPU_BOOLEAN BfrEmpty(Buffer *bfr);

CPU_INT16S BfrAddByte(Buffer *bfr,CPU_INT16S theByte);

CPU_INT16S BfrNextByte(Buffer *bfr);

CPU_INT16S BfrRemoveByte(Buffer *bfr);

#endif