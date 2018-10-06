#include "Init.h"
#include "Thread.h"
#include "Scheduler.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "ThFunc.h"

int RunScheduler( void ){
	while(1){
		if(empty_Queue(ReadyQHead)){}
		else
			__ContextSwitch(curThread, pop_Thread(&ReadyQHead,&ReadyQTail));
		sleep(TIMESLICE);
	}
}

void __ContextSwitch(Thread* pCurThread, Thread* pNewThread){
	if(pCurThread!=NULL && pCurThread->status != THREAD_STATUS_BLOCKED){
		curThread->bRunnable = 0;
		if(curThread->status != THREAD_STATUS_ZOMBIE)
			curThread->status = THREAD_STATUS_READY;
		push_Thread(&ReadyQHead,&ReadyQTail,curThread);
		pthread_kill(curThread->tid, SIGUSR1);
		usleep(10);
	}

	curThread = pNewThread;
	__thread_wakeup(curThread);
}

