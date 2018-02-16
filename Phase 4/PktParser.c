#include "PktParser.h"
#include "Error.h"
#include "SerIODriver.h"

#define ARRAYCONST 1
#define REDUNDANTCONST 5
#define MINPKTLEN 8

#define SYNCBYTE1 0x03
#define SYNCBYTE2 0xef
#define SYNCBYTE3 0xaf

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
//Structure defined to save the packet
typedef struct
{
  CPU_INT08S payloadLen;
  CPU_INT08U data[ARRAYCONST];
}PktBfr;

//P1:Preamble 1; P2:Preamble 2; P3:Preamble 3; L:Length State; D:Data State; CS:Checksum State; ER: Error State
typedef enum{P1,P2,P3,L,D,CS,ER} ParseState;

/*--------Parser Task Constants--------*/
#define SuspendTimeout 0	    // Timeout for semaphore wait
#define Parser_STK_SIZE 128  // Parser stack size
#define ParserPrio 3          // Parser task Priority

#define OPENBFRS 2
#define CLOSEDBFRS 0

static  OS_TCB   parserTCB;                     // Parser task TCB 
static  CPU_STK  ParserStk[Parser_STK_SIZE];  // Space for Parser task stack

OS_SEM	openPayloadBfrs;	  /* Binary Semaphore 1 */
OS_SEM	closedPayloadBfrs;	  /* Binary Semaphore 2 */

extern BfrPair payloadBfrPair;

/*-------Function to create the Parse Packet Task-------*/
CPU_VOID CreateParserTask(CPU_VOID)
{
  /* O/S error code */
  OS_ERR  osErr;
  
  /* Create the producer task */
  OSTaskCreate(&parserTCB,            // Task Control Block                 
               "Parser Task",         // Task name
               ParsePkt,                // Task entry point 
               NULL,                    // Pointer to argument
               ParserPrio,            // Task priority
               &ParserStk[0],         // Base address of task stack space
               Parser_STK_SIZE / 10,  // Stack water mark limit
               Parser_STK_SIZE,       // Task stack size
               0,                       // This task has no task queue
               0,                       // Number of clock ticks (defaults to 10)
               (CPU_VOID*)0,                    // Pointer to TCB extension
               (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),   // Task options
               &osErr);                 // Address to return O/S error code
  
  /* Verify successful task creation. */
  assert(osErr == OS_ERR_NONE);

  /* Create and initialize semaphores. */
  OSSemCreate(&openPayloadBfrs, "Open Sem 1", OPENBFRS, &osErr);			/* -- Empty at start. */
  OSSemCreate(&closedPayloadBfrs, "Closed Sem 2", CLOSEDBFRS, &osErr);			/* -- Empty at start. */
  assert(osErr == OS_ERR_NONE);
}


/*--------Function To Parse The Packets--------*/
CPU_VOID ParsePkt(CPU_VOID*data)
{
  OS_ERR osErr;
  CPU_INT16S nextByte;
  ParseState parseState = P1;
  PktBfr *pktBfr;
  CPU_INT32U i = 0;
  CPU_INT08U len = 0;
  CPU_INT08U checkSum = 0;
  while(1)
  {
    if(PutBfrClosed(&payloadBfrPair))
    {
      OSSemPend(&openPayloadBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);
      assert(osErr==OS_ERR_NONE);
      if(BfrPairSwappable(&payloadBfrPair))
        BfrPairSwap(&payloadBfrPair);
    }
    while(!PutBfrClosed(&payloadBfrPair))
    {
      pktBfr =  (PktBfr*)PutBfrAddr(&payloadBfrPair);
      nextByte = GetByte();
      if(nextByte>=0)
      {
        checkSum^=nextByte;
        switch(parseState)
        {
        case P1:
          if(nextByte == SYNCBYTE1)
          {
            parseState = P2;
          }
          else
          {
            parseState = ER;
            pktBfr->payloadLen = P1BYTEERROR;
            ClosePutBfr(&payloadBfrPair);
          }
          break;
        case P2:
          if(nextByte == SYNCBYTE2)
          {
            parseState = P3;
          }
          else
          {
            parseState = ER;
            pktBfr->payloadLen = P2BYTEERROR;
            ClosePutBfr(&payloadBfrPair);
          }
          break;
        case P3:
          if(nextByte == SYNCBYTE3)
          {
            parseState = L;
          }
          else
          {
            parseState = ER;
            pktBfr->payloadLen = P3BYTEERROR;
            ClosePutBfr(&payloadBfrPair);
          }
          break;
        case L:
          if(nextByte >= MINPKTLEN)
          {
            len = nextByte - REDUNDANTCONST;
            pktBfr->payloadLen = len;
            parseState = D;
          }
          else
          {
            parseState = ER;
            pktBfr->payloadLen = PKTLENERROR;
            ClosePutBfr(&payloadBfrPair);
          }
          break;
        case D:
          pktBfr->data[i] = nextByte;
          i++;
          if(i>=len)
          {
            i = 0;
            parseState = CS;
          }
          break;
        case CS:
          if(!checkSum)
          {
            parseState = P1;
          }
          else
          {
            parseState = ER;
            pktBfr->payloadLen = CHKSUMERROR;
          }
          ClosePutBfr(&payloadBfrPair);
          break;
        case ER:
          i=0;
          checkSum = 0;
          if(nextByte == SYNCBYTE1)
          {
            checkSum^=nextByte;
            nextByte = GetByte();
            if(nextByte == SYNCBYTE2)
            {
              checkSum^=nextByte;
              parseState = P3;
            }
          }
          break;
        }
        if(PutBfrClosed(&payloadBfrPair))
        {
          OSSemPost(&closedPayloadBfrs, OS_OPT_POST_1, &osErr);
          assert(osErr==OS_ERR_NONE);
        }
      }
      
    }
  }
}
