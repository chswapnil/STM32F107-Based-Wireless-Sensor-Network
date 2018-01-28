#include "Payload.h"
#include "Reply.h"
#include "Error.h"


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

static BfrPair payloadBfrPair;
static CPU_INT08U pBfr0Space[payloadBfrSize];
static CPU_INT08U pBfr1Space[payloadBfrSize];

static BfrPair replyBfrPair;
static CPU_INT08U rBfr0Space[replyBfrSize];
static CPU_INT08U rBfr1Space[replyBfrSize];

 
//Initializes the payload buffer pair and reply buffer pair
CPU_VOID PayloadInit(BfrPair**pBfrPair, BfrPair**rBfrPair)
{
  *pBfrPair = &payloadBfrPair;//***puts the address of the static member
  *rBfrPair = &replyBfrPair;//***puts the address of the static member
  BfrPairInit(&payloadBfrPair,pBfr0Space,pBfr1Space,payloadBfrSize);
  BfrPairInit(&replyBfrPair,rBfr0Space,rBfr1Space,replyBfrSize);
  return;
}

CPU_VOID PayloadTask()
{
  
  CPU_INT08U msgBfr[msgBfrSize];//buffer to store the string to display
  Payload *payload;
  
  if(BfrPairSwappable(&payloadBfrPair))//swap if put buffer is closed and get buffer is open
    BfrPairSwap(&payloadBfrPair);
  
  if(!GetBfrClosed(&payloadBfrPair)||PutBfrClosed(&replyBfrPair))//check if payloadbfrpair get buffer is not closed and replybfrpair put buffer is closed, if so return
    return;
  
  payload = (Payload*)GetBfrAddr(&payloadBfrPair);
  
  PutBfrReset(&replyBfrPair);
  
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
  
  PutReplyMsg(&replyBfrPair,msgBfr);
  PutReplyMsg(&replyBfrPair,"\n");
  
  ClosePutBfr(&replyBfrPair);
  OpenGetBfr(&payloadBfrPair);
  
}
