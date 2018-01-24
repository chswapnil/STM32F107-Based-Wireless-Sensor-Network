/* 
Author: Swapnil Suresh Chaughule
Email: chswapnil@gmail.com
*/
#include "includes.h"
#include "PktParser.h"
#include "Error.h"

#define BAUDRATE    9600
#define WINDCONST   2
#define PRECIPCONST 2
#define IDCONST     10
#define DEVICEADDR  1
#define MASK        0xff
#define MSIGMASK    0xf0
#define LSIGMASK    0x0f
#define MSXBMASK    0xff00
#define LSXBMASK    0x00ff
#define TWYSEVBITS  27
#define TWYFOURBITS 24
#define TWYONEBITS  21
#define NINEBITS    9
#define EIGHTBITS   8
#define FIVEBITS    5
#define FOURBITS    4
#define ZEROS       0
#define ONES        1
#define TWOS        2
#define THREES      3
#define TIMEMASK1   0xff000000
#define TIMEMASK2   0xff0000
#define TIMEMASK3   0xff00
#define TIMEMASK4   0xff
#define TIMEMASK5   0x1e0
#define TIMEMASK6   0x1f
#define TIMEMASK7   0x1ffe00
#define TIMEMASK8   0x3f


#define TEMPERATURE   0x01
#define PRESSURE      0x02
#define HUMIDITY      0x03
#define WIND          0x04
#define SOLAR         0x05
#define TIME          0x06
#define PRECIPITATION 0x07
#define ID            0x08

//Function Declarations
CPU_INT32S AppMain();


//Structure Declaration
#pragma pack(1)//Dont align on word boundaries
typedef struct
{
  CPU_INT08U payloadLen;
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


CPU_INT32S main()
{
  CPU_INT32S exitCode;//return value

  BSP_IntDisAll();
  BSP_Init();
  BSP_Ser_Init(BAUDRATE);//Intializes RS232 port to operate at the indicated baud rate
  
  exitCode = AppMain();
  
  return exitCode;
}

CPU_INT32S AppMain()
{
  Payload payload;
  for(;;)
  {
    ParsePkt(&payload);
    if(payload.dstAddr==DEVICEADDR)
    {
      switch(payload.msgType)
      {
      case TEMPERATURE:
        BSP_Ser_Printf("\nSource Node %d: TEMPERATURE MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tTemperature = %d\n",payload.dataPart.temp);
        break;
      case PRESSURE:
        BSP_Ser_Printf("\nSource Node %d: BAROMETRIC PRESSURE MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tPressure = %d\n",((payload.dataPart.pres>>EIGHTBITS)&MASK)|((payload.dataPart.pres&MASK)<<EIGHTBITS));
        break;
      case HUMIDITY:
        BSP_Ser_Printf("\nSource Node %d: HUMIDITY MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tDew Point = %d Humidity = %d\n",payload.dataPart.hum.dewPt,payload.dataPart.hum.hum);
        break;
      case WIND:
        BSP_Ser_Printf("\nSource Node %d: WIND MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tSpeed = %d%d%d.%d  Wind Direction = %d\n",(payload.dataPart.wind.speed[ZEROS]&MSIGMASK)>>FOURBITS,(payload.dataPart.wind.speed[ZEROS]&LSIGMASK),(payload.dataPart.wind.speed[ONES]&MSIGMASK)>>FOURBITS,(payload.dataPart.wind.speed[ONES]&LSIGMASK),((payload.dataPart.wind.dir>>EIGHTBITS)&MASK)|((payload.dataPart.wind.dir&MASK)<<EIGHTBITS));
        break;
      case SOLAR:
        BSP_Ser_Printf("\nSource Node %d: SOLAR RADIATION MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tSolar Radiation Intensity = %d\n",((payload.dataPart.rad&MSXBMASK)>>EIGHTBITS)|((payload.dataPart.rad&LSXBMASK)<<EIGHTBITS));
        break;
      case TIME:
        payload.dataPart.dateTime = ((payload.dataPart.dateTime&TIMEMASK1)>>TWYFOURBITS)|((payload.dataPart.dateTime&TIMEMASK2)>>EIGHTBITS)|((payload.dataPart.dateTime&TIMEMASK3)<<EIGHTBITS)|((payload.dataPart.dateTime&TIMEMASK4)<<TWYFOURBITS);
        BSP_Ser_Printf("\nSource Node %d: DATE/TIME STAMP MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tTime Stamp = %d/%d/%d %d:%d\n",((payload.dataPart.dateTime&TIMEMASK5)>>FIVEBITS),(payload.dataPart.dateTime&TIMEMASK6),((payload.dataPart.dateTime&TIMEMASK7)>>NINEBITS),(payload.dataPart.dateTime>>TWYSEVBITS),(payload.dataPart.dateTime>>TWYONEBITS)&TIMEMASK8);
        break;
      case PRECIPITATION:
        BSP_Ser_Printf("\nSource Node %d: PRECIPITATION MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tPrecipitation Depth = %d%d.%d%d\n",(payload.dataPart.depth[ZEROS]&MSIGMASK)>>FOURBITS,(payload.dataPart.depth[ZEROS]&LSIGMASK),(payload.dataPart.depth[ONES]&MSIGMASK)>>FOURBITS,(payload.dataPart.depth[ONES]&LSIGMASK));
        break;
      case ID:
        BSP_Ser_Printf("\nSource Node %d: SENSOR ID MESSAGE\n",payload.srcAddr);
        BSP_Ser_Printf("\tNode ID = ");
        for(CPU_INT08U i = ZEROS;i<payload.payloadLen-THREES;i++)
        {
          BSP_Ser_Printf("%c",payload.dataPart.id[i]);
        }
        BSP_Ser_Printf("\n");
        break;
      default:
        PktError(UNKPKTERROR);
        break;
      }
    }
    else
    {
      PktError(DSTADRERROR);
    }
  }
    
}


