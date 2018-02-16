#include "Payload.h"
#include "PktParser.h"
#include "Error.h"
#include "SerIODriver.h"
#include <assert.h>

/*----------Structure Declaration-----------*/
#pragma pack(1)//Dont align on word boundaries
typedef struct
{
  CPU_INT08S payloadLen;/*Signed changed to include the error message*/ 
  CPU_INT08U dstAddr;
  CPU_INT08U srcAddr;
  CPU_INT08U msgType;
  union
  {
    CPU_INT08S temp;
    CPU_INT16U pres;
    struct
    {
      CPU_INT08S dewPt;
      CPU_INT08U hum;
    } hum;
    struct
    {
      CPU_INT08U speed[WINDCONST];//change the constant
      CPU_INT16U dir;
    } wind;
    CPU_INT16U rad;
    CPU_INT32U dateTime;
    CPU_INT08U depth[PRECIPCONST];//change the constant
    CPU_INT08U id[IDCONST];//change the constant
  } dataPart;
}Payload;


/*----------Payload Task Constants----------*/
#define SuspendTimeout 0	    // Timeout for semaphore wait
#define Payload_STK_SIZE 128  // Consumer task Priority
#define PayloadPrio 4         // Consumer task Priority


static  OS_TCB   payloadTCB;                     // Consumer task TCB
static  CPU_STK  payloadStk[Payload_STK_SIZE];  // Space for Consumer task stack

BfrPair payloadBfrPair;
static CPU_INT08U pBfr0Space[payloadBfrSize];
static CPU_INT08U pBfr1Space[payloadBfrSize];

extern OS_SEM	openPayloadBfrs;	  /* Binary Semaphore 1 */
extern OS_SEM	closedPayloadBfrs;	  /* Binary Semaphore 2 */

 
//Initializes the payload buffer pair and reply buffer pair
CPU_VOID PayloadInit(BfrPair**pBfrPair)
{
  *pBfrPair = &payloadBfrPair;//***puts the address of the static member
  BfrPairInit(&payloadBfrPair,pBfr0Space,pBfr1Space,payloadBfrSize);
  return;
}

//Creates a Payload Task
void CreatePayloadTask(void)
{
  /* O/S error code */
  OS_ERR  osErr;                            
  
  /* Create the consumer task */
  OSTaskCreate(&payloadTCB,            // Task Control Block                 
               "Payload Task",         // Task name
               PayloadTask,                // Task entry point 
               NULL,                    // Pointer to argument
               PayloadPrio,            // Task priority
               &payloadStk[0],         // Base address of task stack space
               Payload_STK_SIZE / 10,  // Stack water mark limit
               Payload_STK_SIZE,       // Task stack size
               0,                       // This task has no task queue
               0,                       // Number of clock ticks (defaults to 10)
               (void      *)0,          // Pointer to TCB extension
               0,                       // Task options
               &osErr);                 // Address to return O/S error code
    
  /* Verify successful task creation. */
  assert(osErr == OS_ERR_NONE);
}

CPU_VOID PayloadTask(CPU_VOID*data)
{
  OS_ERR osErr;
  CPU_INT08U msgBfr[msgBfrSize];//buffer to store the string to display
  CPU_INT16S c;
  Payload *payload;
  while(1)
  {
    if(!GetBfrClosed(&payloadBfrPair))
    {
      OSSemPend(&closedPayloadBfrs, SuspendTimeout, OS_OPT_PEND_BLOCKING, NULL, &osErr);
      assert(osErr==OS_ERR_NONE);
      if(BfrPairSwappable(&payloadBfrPair))
        BfrPairSwap(&payloadBfrPair);
      payload = (Payload*)GetBfrAddr(&payloadBfrPair);
    }
      while(GetBfrClosed(&payloadBfrPair))
      { 
        if(payload->payloadLen>=0)
        {
          if(payload->dstAddr==DEVICEADDR)
          {
            switch(payload->msgType)
            {
            case TEMPERATURE:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: TEMPERATURE MESSAGE\n\tTemperature = %d\n",payload->srcAddr,payload->dataPart.temp);
              break;
            case PRESSURE:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: BAROMETRIC PRESSURE MESSAGE\n\tPressure = %d\n",payload->srcAddr,((payload->dataPart.pres>>EIGHTBITS)&MASK)|((payload->dataPart.pres&MASK)<<EIGHTBITS));
              break;
            case HUMIDITY:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: HUMIDITY MESSAGE\n\tDew Point = %d Humidity = %d\n",payload->srcAddr,payload->dataPart.hum.dewPt,payload->dataPart.hum.hum);
              break;
            case WIND:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: WIND MESSAGE\n\tSpeed = %d%d%d.%d  Wind Direction = %d\n",payload->srcAddr,(payload->dataPart.wind.speed[ZEROS]&MSIGMASK)>>FOURBITS,(payload->dataPart.wind.speed[ZEROS]&LSIGMASK),(payload->dataPart.wind.speed[ONES]&MSIGMASK)>>FOURBITS,(payload->dataPart.wind.speed[ONES]&LSIGMASK),((payload->dataPart.wind.dir>>EIGHTBITS)&MASK)|((payload->dataPart.wind.dir&MASK)<<EIGHTBITS));
              break;
            case SOLAR:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: SOLAR RADIATION MESSAGE\n\tSolar Radiation Intensity = %d\n",payload->srcAddr,((payload->dataPart.rad&MSXBMASK)>>EIGHTBITS)|((payload->dataPart.rad&LSXBMASK)<<EIGHTBITS));
              break;
            case TIME:
              payload->dataPart.dateTime = ((payload->dataPart.dateTime&TIMEMASK1)>>TWYFOURBITS)|((payload->dataPart.dateTime&TIMEMASK2)>>EIGHTBITS)|((payload->dataPart.dateTime&TIMEMASK3)<<EIGHTBITS)|((payload->dataPart.dateTime&TIMEMASK4)<<TWYFOURBITS);
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: DATE/TIME STAMP MESSAGE\n\tTime Stamp = %d/%d/%d %d:%d\n",payload->srcAddr,((payload->dataPart.dateTime&TIMEMASK5)>>FIVEBITS),(payload->dataPart.dateTime&TIMEMASK6),((payload->dataPart.dateTime&TIMEMASK7)>>NINEBITS),(payload->dataPart.dateTime>>TWYSEVBITS),(payload->dataPart.dateTime>>TWYONEBITS)&TIMEMASK8);
              break;
            case PRECIPITATION:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: PRECIPITATION MESSAGE\n\tPrecipitation Depth = %d%d.%d%d\n",payload->srcAddr,(payload->dataPart.depth[ZEROS]&MSIGMASK)>>FOURBITS,(payload->dataPart.depth[ZEROS]&LSIGMASK),(payload->dataPart.depth[ONES]&MSIGMASK)>>FOURBITS,(payload->dataPart.depth[ONES]&LSIGMASK));
              break;
            case ID:
              sprintf((CPU_CHAR*)msgBfr,"\nSource Node %d: SENSOR ID MESSAGE\n\tNode ID = %s\n",payload->srcAddr,payload->dataPart.id);
              break;
            default:
              HandleError(UNKPKTERROR,msgBfr);
              break;
            }
          }
          else
          {
            HandleError(DSTADRERROR,msgBfr);
          }
        }
        else
        {
          HandleError(payload->payloadLen,msgBfr);
        }
        for(c=0;msgBfr[c]!='\0';c++)/*Print the message buffer*/
          PutByte(msgBfr[c]);
        OpenGetBfr(&payloadBfrPair);/*Open The Get Buffer to Get out of the Loop*/
        if(!GetBfrClosed(&payloadBfrPair))
        {
          OSSemPost(&openPayloadBfrs, OS_OPT_POST_1, &osErr);
          assert(osErr==OS_ERR_NONE);
        }
      }
  }
}