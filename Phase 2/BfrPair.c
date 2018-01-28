/*
Author: Swapnil Chaughule
*/
#include "BfrPair.h"

/*
typedef struct
{
  CPU_INT08U putBfrNum; 
  Buffer buffer[NumBfrs];
}BfrPair;
*/

/*Initializes the buffer pair*/
void BfrPairInit( BfrPair *bfrPair,CPU_INT08U *bfr0Space,CPU_INT08U *bfr1Space,CPU_INT16U size)
{
  bfrPair->putBfrNum=0;
  BfrInit(&bfrPair->buffer[bfrPair->putBfrNum],bfr0Space,size);
  BfrInit(&bfrPair->buffer[1-bfrPair->putBfrNum],bfr1Space,size);
  return;
}

/*Resets the put buffer in the buffer pair*/
void PutBfrReset(BfrPair *bfrPair)
{
  BfrReset(&bfrPair->buffer[bfrPair->putBfrNum]);
  return;
}

/*Returns the data space address of the put buffer*/
CPU_INT08U *PutBfrAddr(BfrPair *bfrPair)
{
  return (bfrPair->buffer[bfrPair->putBfrNum].buffer);
}

/*Returns the data space address of the get buffer*/
CPU_INT08U *GetBfrAddr(BfrPair *bfrPair)
{
  return (bfrPair->buffer[1-bfrPair->putBfrNum].buffer);
}

/*Returns True if put buffer is closed or else returns false*/
CPU_BOOLEAN PutBfrClosed(BfrPair *bfrPair)
{
  return BfrClosed(&bfrPair->buffer[bfrPair->putBfrNum]);
}

/*Return true if get buffer is closed or else return false*/
CPU_BOOLEAN GetBfrClosed(BfrPair *bfrPair)
{
  return BfrClosed(&bfrPair->buffer[1-bfrPair->putBfrNum]);
}

/*Marks put buffer as close*/
void ClosePutBfr(BfrPair *bfrPair)
{
  BfrClose(&bfrPair->buffer[bfrPair->putBfrNum]);
  return;
}

/*Marks get buffer as open*/
void OpenGetBfr (BfrPair *bfrPair)
{
  BfrOpen(&bfrPair->buffer[1-bfrPair->putBfrNum]);
  return;
}

/*Adds a byte to the put buffer and increments the putIndex*/
CPU_INT16S PutBfrAddByte(BfrPair *bfrPair,CPU_INT16S byte)
{
  return BfrAddByte(&bfrPair->buffer[bfrPair->putBfrNum],byte);
}

/*Returns a byte from get buffer without incrementing the getIndex*/
CPU_INT16S GetBfrNextByte(BfrPair *bfrPair)
{
  return BfrNextByte(&bfrPair->buffer[1-bfrPair->putBfrNum]);
}

/*Returns a byte from get buffer and increments the getIndex*/
CPU_INT16S GetBfrRemByte(BfrPair *bfrPair)
{
  return BfrRemoveByte(&bfrPair->buffer[1-bfrPair->putBfrNum]);
}

/*Checks whether the put buffer and get buffer are swappable i.e. put buffer is closed and get buffer is opened*/
CPU_BOOLEAN BfrPairSwappable(BfrPair *bfrPair)
{
  if(PutBfrClosed(bfrPair)==TRUE&&GetBfrClosed(bfrPair)==FALSE)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
/*Swap the put and get buffer and reset the put buffer*/
void BfrPairSwap(BfrPair *bfrPair)
{
  bfrPair->putBfrNum = 1-bfrPair->putBfrNum;
  PutBfrReset(bfrPair);
  return;
}
