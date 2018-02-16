#include "includes.h"
#include "BfrPair.h"



#ifndef PAYLOAD_H_
#define PAYLOAD_H_

#define payloadBfrSize 16 //changed from 16 to 20 for debugging
#define replyBfrSize 80
#define WINDCONST   2
#define PRECIPCONST 2
#define IDCONST     10
#define msgBfrSize 160

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

#define DEVICEADDR  1 /*Address of the Device*/


/*------------Functions------------*/
//Intializes Payload Buffer Pair and Reply Buffer Pair
CPU_VOID PayloadInit(BfrPair**payloadBfrPair, BfrPair**replyBfrPair);


CPU_VOID PayloadTask();


#endif