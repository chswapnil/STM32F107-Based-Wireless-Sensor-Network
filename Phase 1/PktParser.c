#include "PktParser.h"//Own header
#include "Error.h"//Defining Functions for Error


#define ARRAYCONST 1 //No of elements defined in an array
#define REDUNDANTCONST 5 //Excluding the length byte itself
#define MINPKTLEN 8
#define ZEROHEX 0x00

#define SYNCBYTE1 0x03
#define SYNCBYTE2 0xef
#define SYNCBYTE3 0xaf



//Structure defined to save the packet
typedef struct
{
  CPU_INT08U payloadLen;
  CPU_INT08U data[ARRAYCONST];
}PktBfr;

//P1:Preamble 1; P2:Preamble 2; P3:Preamble 3; L:Length State; D:Data State; CS:Checksum State; ER: Error State
typedef enum{P1,P2,P3,L,D,CS,ER} ParseState;

CPU_VOID ParsePkt(CPU_VOID *payloadBfr)
{
  PktBfr *pktBfr = payloadBfr;
  ParseState state = P1;
  CPU_INT08U c,checkSum = ZEROHEX;
  for(CPU_INT32U i=ZEROHEX;;)
  {
    c = GetByte();
    checkSum ^= c;
    switch(state)
    {
    case P1:
      if(c==SYNCBYTE1)
      {
        state = P2;
      }
      else
      {
        PktError(P1BYTEERROR);
        state = ER;
      }
      break;
    case P2:
      if(c==SYNCBYTE2)
      {
        state = P3;
      }
      else
      {
        PktError(P2BYTEERROR);
        state = ER;
      }
      break;
    case P3:
      if(c==SYNCBYTE3)
      {
        state = L;
      }
      else
      {
        PktError(P3BYTEERROR);
        state = ER;
      }
      break;
    case L:
      if(c>=MINPKTLEN)
      {
        pktBfr->payloadLen = c - REDUNDANTCONST;
        state = D;
      }
      else
      {
        PktError(PKTLENERROR);
        state = ER;
      }
      break;
    case D:
      pktBfr->data[i++] = c;
      if(i>=(pktBfr->payloadLen))
      {
        i = 0;
        state = CS;
      }
      break;
    case CS:
      if(!checkSum)
      {
        state = P1;
        return;
      }
      else
      {
        PktError(CHKSUMERROR);
        state = ER;
      }
      break;
    case ER:
      checkSum = ZEROHEX;
      if(c==SYNCBYTE1)
      {
        checkSum^=c;
        c = GetByte();
        if(c==SYNCBYTE2)
        {
          checkSum^=c;
          state = P3;
        }
      }
      break; 
    } 
  }
}