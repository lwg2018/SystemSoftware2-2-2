#include "Init.h"
#include "Thread.h"
#include "ThFunc.h"
#include "MsgQueue.h"
#include <stdio.h>

void Init(){
	ReadyQHead = NULL;
	ReadyQTail = NULL;

	WaitQHead = NULL;
	WaitQTail = NULL;

	curThread = NULL;

	for(int i=0; i<MAX_QCB_SIZE; i++){
		qcbTblEntry[i].key = -1;
		qcbTblEntry[i].pQcb = NULL;
	}
}

