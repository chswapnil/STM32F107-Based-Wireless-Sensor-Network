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


CPU_VOID ParsePkt(BfrPair *payloadBfrPair)
{
  static ParseState parseState = P1;//stores the state
  static CPU_INT08U checkSum = 0;//caculates the checksum
  static CPU_INT32U i = 0;;//
  PktBfr *pktBfr = (PktBfr*) PutBfrAddr(payloadBfrPair);
  CPU_INT16S nextByte;//read the byte
  if(PutBfrClosed(payloadBfrPair))
    return;
  
  for(;;)
  {
    nextByte = GetByte();
    if(nextByte<0)
      return;
    checkSum ^= nextByte;
    switch(parseState)
    {
    case P1:
      if(nextByte == SYNCBYTE1)
      {
        parseState = P2;
      }
      else
      {
        pktBfr->payloadLen = P1BYTEERROR;//Putting lenght of pkt as negative
        ClosePutBfr(payloadBfrPair);//closing the put buffer of payload buffer
        parseState = ER;//putting the state to error
      }
      break;
    case P2:
      if(nextByte == SYNCBYTE2)
      {
        parseState = P3;
      }
      else
      {
        pktBfr->payloadLen = P2BYTEERROR;
        ClosePutBfr(payloadBfrPair);
        parseState = ER;
      }
      break;
    case P3:
      if(nextByte == SYNCBYTE3)
      {
        parseState = L;
      }
      else
      {
        pktBfr->payloadLen = P3BYTEERROR;
        ClosePutBfr(payloadBfrPair);
        parseState = ER;
      }
      break;
    case L:
      if(nextByte>=MINPKTLEN)
      {
        pktBfr->payloadLen = nextByte - REDUNDANTCONST;
        parseState = D;
      }
      else
      {
        pktBfr->payloadLen = PKTLENERROR;
        ClosePutBfr(payloadBfrPair);
        parseState = ER;
      }
      break;
    case D:
      pktBfr->data[i++] = nextByte;
      if(i>=(pktBfr->payloadLen))
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
        pktBfr->payloadLen = CHKSUMERROR;
        parseState = ER;
      }
      ClosePutBfr(payloadBfrPair);//Close the put buffer 
      return;//and then return even if there is an error so that the payload processes the error first
    case ER:
      i = 0;
      checkSum = 0;
      if(nextByte == SYNCBYTE1)
      {
        checkSum^=nextByte;
        nextByte = GetByte();
        if(nextByte<0)
        {
          checkSum = 0;
          return;
        }
        if(nextByte == SYNCBYTE2)
        {
          checkSum^=nextByte;
          parseState = P3;
        }
      }
      break;
    }
    
  }
}
