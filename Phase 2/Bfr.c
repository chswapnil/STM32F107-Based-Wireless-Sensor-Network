#include "Bfr.h"


/*Intializes the Buffer*/
void BfrInit(Buffer *bfr, CPU_INT08U *bfrSpace, CPU_INT16U size)
{
  bfr->buffer  = bfrSpace;
  bfr->size = size;
  bfr->closed = FALSE;
  bfr->putIndex = 0;
  bfr->getIndex = 0;
  return;
}

/*Resets the Buffer*/
void BfrReset(Buffer *bfr)
{
  bfr->putIndex = 0;
  bfr->getIndex = 0;
  bfr->closed = FALSE;
  return;
}

/*Checks whether the buffers closed*/
CPU_BOOLEAN BfrClosed(Buffer *bfr)
{
  return bfr->closed;
}

/*Marks the buffer close*/
void BfrClose(Buffer *bfr)
{
  bfr->closed = TRUE;
  return;
}

/*Marks the buffer open*/
void BfrOpen(Buffer *bfr)
{
  bfr->closed = FALSE;
  return;
}

/*Checks whether the buffer is full*/
CPU_BOOLEAN BfrFull(Buffer *bfr)
{
  if(bfr->putIndex>=bfr->size)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*Checks whether the buffer is empty*/
CPU_BOOLEAN BfrEmpty(Buffer *bfr)
{
  if(bfr->getIndex>=bfr->putIndex)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/*Adds theByte to the buffer and increments the putIndex value*/
CPU_INT16S BfrAddByte(Buffer *bfr,CPU_INT16S theByte)
{
  if(BfrFull(bfr))
  {
    return -1;
  }
  else
  {
    *(bfr->buffer+bfr->putIndex) = theByte;
    bfr->putIndex++;
    if(BfrFull(bfr))
    {
      BfrClose(bfr);
    }
    return theByte;
  }
}

/*Gets the byte from the buffer without incrementing the getIndex value*/
CPU_INT16S BfrNextByte(Buffer *bfr)
{
  if(BfrEmpty(bfr))
  {
    return -1;
  }
  else
  {
    return *(bfr->buffer+bfr->getIndex);
  }
}

/*Gets the byte from the buffer and increments the getIndex value*/
CPU_INT16S BfrRemoveByte(Buffer *bfr)
{
  CPU_INT08U temp;
  if(BfrEmpty(bfr))
  {
    return -1;
  }
  else
  {
    temp = *(bfr->buffer+bfr->getIndex);
    bfr->getIndex++;
    if(BfrEmpty(bfr))
    {
      BfrOpen(bfr);
    }
    return temp;
    
  }
}