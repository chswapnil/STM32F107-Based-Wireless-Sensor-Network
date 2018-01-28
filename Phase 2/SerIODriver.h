#include "includes.h"
#include "BfrPair.h"

#ifndef SERIODRIVER_H_
#define SERIODRIVER_H_


/*----------Function Prototypes-----------*/

void InitSerIO(void);

CPU_INT16S PutByte(CPU_INT16S txChar);

void ServiceTx(void);

CPU_INT16S GetByte(void); 

void ServiceRx(void);


#endif
