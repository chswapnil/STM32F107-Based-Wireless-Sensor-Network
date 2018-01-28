#include "SerIODriver.h"


#ifndef BfrSize
#define BfrSize 4
#endif

#ifndef USART_RXNE
#define USART_RXNE  0x20  // Rx not Empty Status Bit
#endif

#ifndef USART_TXE
#define USART_TXE   0x80  // Tx Empty Status Bit
#endif

// Allocate the input buffer pair.
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];
 
// Allocate the output buffer pair.
static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];


/*-------Function Definition--------*/

void InitSerIO(void)
{
  BfrPairInit(&iBfrPair,iBfr0Space,iBfr1Space,BfrSize);
  BfrPairInit(&oBfrPair,oBfr0Space,oBfr1Space,BfrSize);
  return;
}

CPU_INT16S PutByte(CPU_INT16S txChar)
{
  if(BfrPairSwappable(&oBfrPair)==TRUE)
  {
    BfrPairSwap(&oBfrPair);
  }
  if(PutBfrClosed(&oBfrPair)==TRUE)
  {
    return -1;
  }
  else
  {
    return PutBfrAddByte(&oBfrPair, txChar);
  }
}

void ServiceTx(void)
{
  CPU_INT16S  c;
  if(USART2->SR&USART_TXE)
  {
    if(GetBfrClosed(&oBfrPair)==TRUE)
    {
      c = GetBfrRemByte(&oBfrPair);
      USART2->DR = c;
    }
  }
  return;
}

CPU_INT16S GetByte(void)
{
  CPU_INT16S c;
  if(BfrPairSwappable(&iBfrPair)==TRUE)
  {
    BfrPairSwap(&iBfrPair);
  }
  c = GetBfrRemByte(&iBfrPair);
  return c;
}

void ServiceRx(void)
{
  CPU_INT08U   rx_byte;
  if(USART2->SR & USART_RXNE)
  {
    if(PutBfrClosed(&iBfrPair)==FALSE)
    {
      rx_byte = USART2->DR;/* Read the data form the temporal register          */
      PutBfrAddByte(&iBfrPair,rx_byte);
    }
  }
  return;
}

