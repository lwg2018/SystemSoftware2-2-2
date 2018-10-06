#include "Thread.h"
#include "Init.h"
#include "Scheduler.h"
#include "ThFunc.h"
#include "MsgQueue.h"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

Thread *new_th = NULL;
Thread *waitch = NULL;

void* __wrapperFunc(void* arg) {
	void* ret;
	WrapperArg* pArg = (WrapperArg*)arg;

	new_th = make_Thread();
	new_th->pExitCode = pArg->funcArg;
	push_Thread(&ReadyQHead, &ReadyQTail, new_th);

	__thread_wait_handler(0);
	void* (*funcPtr)(void *) = pArg->funcPtr;
	void* funcArg = pArg->funcArg;

	ret = (funcPtr)(funcArg);
	return ret;
}

int thread_create(thread_t *thread, thread_attr_t *attr, void* (*start_routine) (void *), void *arg){
	WrapperArg* rap = (WrapperArg*)malloc(sizeof(WrapperArg));
	rap->funcPtr = start_routine;
	rap->funcArg = arg;

	pthread_create(thread, attr, __wrapperFunc, rap);

	while(new_th == NULL)usleep(1);
	new_th->parentTid = thread_self();
	new_th = NULL;
}

int thread_join(thread_t thread, void **retval){
	Thread* jo = curThread;
	waitch = __getThread(thread);

	if(waitch == NULL)return -1;

	if(waitch->status != THREAD_STATUS_ZOMBIE){
		push_Thread(&WaitQHead,&WaitQTail,jo);
		jo->status = THREAD_STATUS_BLOCKED;

		jo->bRunnable = 0;
		__thread_wait_handler(0);
	}

	remove_Thread(&ReadyQHead, &ReadyQTail,waitch);
	(*retval) = waitch->pExitCode;
	free(waitch);

	return 0;
}

int thread_suspend(thread_t tid){
	Thread *sus;

	sus = __getThread(tid);
	if(sus == NULL){
		sus = curThread;
		sus->status = THREAD_STATUS_BLOCKED;
		return 0;
	}

	sus->status = THREAD_STATUS_BLOCKED;
	remove_Thread(&ReadyQHead, &ReadyQTail,sus);
	push_Thread(&WaitQHead,&WaitQTail,sus);
	return 0;
}


int thread_resume(thread_t tid){
	Thread *res;

	res = __getThreadW(tid);
	if(res == NULL)
		return -1;
	res->status = THREAD_STATUS_READY;
	remove_Thread(&WaitQHead, &WaitQTail,res);
	push_Thread(&ReadyQHead,&ReadyQTail,res);
	return 0;
}


thread_t thread_self(){
	return pthread_self();
}

int thread_exit(void* retval){
	Thread* me = curThread;
	me->status = THREAD_STATUS_ZOMBIE;
	me->pExitCode = retval;

	Thread* pa = __getThreadW(me->parentTid);
	if(pa != NULL && waitch == me){
		pa->status = THREAD_STATUS_READY;
		remove_Thread(&WaitQHead, &WaitQTail,pa);
		push_Thread(&ReadyQHead,&ReadyQTail,pa);
	}
	return 0;
}
