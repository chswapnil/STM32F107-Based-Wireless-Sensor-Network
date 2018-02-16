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

/*Timeout for semaphore wait*/
#define SuspendTimeout 0	    

// Allocate the input buffer pair.
static BfrPair iBfrPair;
static CPU_INT08U iBfr0Space[BfrSize];
static CPU_INT08U iBfr1Space[BfrSize];
 
// Allocate the output buffer pair.
static BfrPair oBfrPair;
static CPU_INT08U oBfr0Space[BfrSize];
static CPU_INT08U oBfr1Space[BfrSize];


static OS_SEM openOBfrs;  /*Semaphores to count number of output buffers open*/
static OS_SEM closedIBfrs;/*Semaphores to count number of input buffers closed*/

/*-------Function Definition--------*/

void InitSerIO(void)
{
  /*Interrupts Initialized*/
  USART2->CR1 = USARTINTENA;//Unmasked TX and RX Interrupts
  SETENA1 = USART2ENA;//Enable Interrupts in NVIC; Enabled IRQ38
  /*IO Buffers Initialized*/
  BfrPairInit(&iBfrPair,iBfr0Space,iBfr1Space,BfrSize);
  BfrPairInit(&oBfrPair,oBfr0Space,oBfr1Space,BfrSize);
  /*Semaphores Initialized*/
  OS_ERR  osErr;// O/S Error Code
  OSSemCreate(&openOBfrs, "openOBfrs", 2, &osErr);/*Number of Open output buffers*/
  assert(osErr == OS_ERR_NONE);/*Check for any Errors*/
  OSSemCreate(&closedIBfrs, "closedIBfrs", 0, &osErr);/*Number of Closed input buffers*/
  assert(osErr == OS_ERR_NONE);/*Check for any Errors*/
  return;
}

CPU_INT16S PutByte(CPU_INT16S txChar)
{
  OS_ERR osErr;/* -- Semaphore error code */
  
  if(PutBfrClosed(&oBfrPair))
  {
    OSSemPend(&openOBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);/*Decrease the semaphore by 1 if put buffer is closed*/		
    assert(osErr==OS_ERR_NONE);/*Check for any error*/
    if(BfrPairSwappable(&oBfrPair))
      BfrPairSwap(&oBfrPair);/*Swap Put and Get Buffers*/
  }
  
  PutBfrAddByte(&oBfrPair, txChar);
  USART2->CR1 |= TXUNMASK; /*Unmasking TX interrupt, Because there is a byte available to transmitt*/
  return txChar;
}

void ServiceTx(void)
{
  CPU_INT16S  c;
  OS_ERR osErr;/* -- Semaphore error code */
  
  if(USART2->SR&USART_TXE)/*Check Whether TX bit is one*/
  {
    if(!GetBfrClosed(&oBfrPair))/*check wheter Get Buffer is not closed*/
    {
      USART2->CR1 &= TXMASK; /*Masking TX interrupts, Because the the Get buffer is not closed or full yet*/
    }
    else
    {
      c = GetBfrRemByte(&oBfrPair);/*Get the byte from the get buffer*/
      USART2->DR = c;/*Put the byte over the register*/
      if(!GetBfrClosed(&oBfrPair))/*check whether get buffer is still closed or not*/
      {
        OSSemPost(&openOBfrs, OS_OPT_POST_1, &osErr);/*Increase the value of semaphore if get buffer is not closed*/
        assert(osErr==OS_ERR_NONE);
      }
    }
  }
  return;
}

CPU_INT16S GetByte(void)
{
  OS_ERR osErr;/* -- Semaphore error code */
  CPU_INT32S retByte;
  
  if(!GetBfrClosed(&iBfrPair))
  {
    OSSemPend(&closedIBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);/*Decrease the semaphore by 1 if put buffer is closed*/		
    assert(osErr==OS_ERR_NONE);/*Check for any error*/
    if(BfrPairSwappable(&iBfrPair))
      BfrPairSwap(&iBfrPair);/*Swap Put and Get Buffers*/
  }
  
  retByte = GetBfrRemByte(&iBfrPair);
  USART2->CR1 |= RXUNMASK; /*Unmasking RX interrupt, Because there is a byte available to receive*/
  return retByte;
  
}

void ServiceRx(void)
{
  OS_ERR osErr;/* -- Semaphore error code */
  CPU_INT08U   rx_byte;
  if(USART2->SR & USART_RXNE)
  {
    if(PutBfrClosed(&iBfrPair))
    {
      USART2->CR1 &= RXMASK; /*Masking RX Interrupt, because Put Buffer is closed or full*/
    }
    else
    {
      rx_byte = USART2->DR;/* Read the data form the temporal register          */
      PutBfrAddByte(&iBfrPair,rx_byte);
      if(PutBfrClosed(&iBfrPair))/*check whether get buffer is still closed or not*/
      {
        OSSemPost(&closedIBfrs, OS_OPT_POST_1, &osErr);/*Increase the value of semaphore if get buffer is not closed*/
        assert(osErr==OS_ERR_NONE);
      }
    }
  }
  return;
}

void SerialISR(void)
{
  /*Boiler Plate Code*/
  /* Disable interrupts. */
  CPU_SR_ALLOC();
  OS_CRITICAL_ENTER();  
  /* Tell kernel we're in an ISR. */
  OSIntEnter();
  /* Enable interrupts. */
  OS_CRITICAL_EXIT();
  /*Boiler Plate Code*/
  
  ServiceRx();
  
  ServiceTx();
  
  /*Boiler Plate Code*/
  /* Give the O/S a chance to swap tasks. */
  OSIntExit ();
  /*Boiler Plate Code*/
}
