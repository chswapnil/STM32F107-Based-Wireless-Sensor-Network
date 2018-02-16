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

/*Enable IRQ 38 interrupt in NVIC which is in Cortex M3*/
#define SETENA1 (*((CPU_INT32U*)0xE000E104))
#define USART2ENA 0x00000040 

/*Enabling TX and RX Interrupts in stm32f104*/
#define USARTINTENA 0x000020AC

/*Mask TX Interrutps*/
#define TXMASK 0xFFFFFF7F

/*Unmask TX Interrutps*/
#define TXUNMASK 0x00000080

/*Mask RX Interrutps*/
#define RXMASK 0xFFFFFFDF

/*Unmask RX Interrutps*/
#define RXUNMASK 0x00000020

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
  USART2->CR1 = USARTINTENA;//Unmasked TX and RX Interrupts
  SETENA1 = USART2ENA;//Enable Interrupts in NVIC; Enabled IRQ38
  BfrPairInit(&iBfrPair,iBfr0Space,iBfr1Space,BfrSize);
  BfrPairInit(&oBfrPair,oBfr0Space,oBfr1Space,BfrSize);
  return;
}

CPU_INT16S PutByte(CPU_INT16S txChar)
{
  CPU_INT16S retValue;
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
    retValue = PutBfrAddByte(&oBfrPair, txChar);
    USART2->CR1 |= TXUNMASK; /*Unmasking TX interrupt, Because there is a byte available to transmitt*/
    return retValue;
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
    else
    {
      USART2->CR1 &= TXMASK; /*Masking TX interrupts, Because the the Get buffer is not closed or full yet*/
    }
  }
  return;
}

CPU_INT16S GetByte(void)
{
  CPU_INT16S retByte;
  if(BfrPairSwappable(&iBfrPair)==TRUE)
  {
    BfrPairSwap(&iBfrPair);
  }
  retByte = GetBfrRemByte(&iBfrPair);
  USART2->CR1 |= RXUNMASK; /*Unmasking RX interrupt, Because there is a byte available to receive*/
  return retByte;
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
      /*More Byte to add*/
    }
    else
    {
      USART2->CR1 &= RXMASK; /*Masking RX Interrupt, because Put Buffer is closed or full*/
    }
  }
  return;
}

void SerialISR(void)
{
  ServiceRx();
  
  ServiceTx();
}
