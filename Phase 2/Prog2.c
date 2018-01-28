/*--------------- P r o g 3 . c ---------------

by: George Cheney
    ECE Dept.
    UMASS Lowell

PURPOSE
Receive wireless sensor network packets from the RS232 port,
interpret and display the messages.

DEMONSTRATES
Cooperative multitasking
Concurrent, polled I/O

CHANGES
01-29-2013 gpc -  Created
*/

#include "includes.h"//Defined
#include "BfrPair.h"//Defined
#include "Reply.h"//Defined
#include "Payload.h"//Not Defined
#include "Error.h"//Not Defined
#include "PktParser.h"//Not Defined
#include "SerIODriver.h"//Defined

/*----- c o n s t a n t    d e f i n i t i o n s -----*/

// Define RS232 baud rate.
#define BaudRate 9600

/*----- f u n c t i o n    p r o t o t y p e s -----*/

void AppMain(void);

/*--------------- m a i n ( ) -----------------*/

CPU_INT32S main()
{
//  Initialize the STM32F107 eval. board.
    BSP_IntDisAll();            /* Disable all interrupts. */

    BSP_Init();                 /* Initialize BSP functions */

    BSP_Ser_Init(BaudRate);     /* Initialize the RS232 interface. */

//  Run the application.    
    AppMain();
    
    return 0;
}

/*--------------- A p p M a i n ( ) ---------------

PURPOSE
This is the application main program.

*/


void AppMain(void)
{
  BfrPair *payloadBfrPair;  // Address of the Payload Buffer Pair
  BfrPair *replyBfrPair;    // Address of the Reply Buffer Pair

  // Create and Initialize iBfrPair and oBfrPair.
  InitSerIO();

  // Create and initialize the Payload Buffer Pair and the Reply Buffer
  // Pair and get their addresses.
  PayloadInit(&payloadBfrPair, &replyBfrPair);
  
  // Multitasking Executive Loop: Tasks are executed round robin.
  
  /*Program Runs Correctly till the IO driver i.e. the IO drivers are tested after that everything written at once*/
  for (;;)
    {
    // Service the RS232 receiver.
    ServiceRx();
    //Functions communicates using GetByte call
    // Execute the ParsePkt task.
    ParsePkt(payloadBfrPair); /************Warning at ParsePkt Header If Program Doesnt work correct them and then try*/
 
    // Execute the Payload task.
    PayloadTask();
    //Functions communicate using PutByte call
    // Execute the Reply Task.
    Reply(replyBfrPair);

    // Service the RS232 transmitter.
    ServiceTx();
    }
}
