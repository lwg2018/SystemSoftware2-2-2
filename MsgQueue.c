#include "Thread.h"
#include "MsgQueue.h"
#include "Init.h"
#include "ThFunc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

void _InitMsgQueue(void){
	Init(); // qcbTblEntry init
}

int 	mymsgget(int key, int msgflg){
	int i;

	for(i=0; i<MAX_QCB_SIZE; i++){
		if(qcbTblEntry[i].key==key)
			return i;
	}
	
	if(i==MAX_QCB_SIZE){
		for(i=0; i<MAX_QCB_SIZE; i++)
			if(qcbTblEntry[i].key == -1)
				break;
	}

	qcbTblEntry[i].key = key;
	qcbTblEntry[i].pQcb = (Qcb*)malloc(sizeof(Qcb));

	qcbTblEntry[i].pQcb->msgCount = 0;
	qcbTblEntry[i].pQcb->pMsgHead = NULL;
	qcbTblEntry[i].pQcb->pMsgTail = NULL;
	qcbTblEntry[i].pQcb->waitThreadCount = 0;
	qcbTblEntry[i].pQcb->pThreadHead = NULL;
	qcbTblEntry[i].pQcb->pThreadTail = NULL;

	return i;
}

int 	mymsgsnd(int msqid, const void *msgp, int msgsz, int msgflg){
	//printf("snd msqid:%d\n", msqid);
	if(qcbTblEntry[msqid].key == -1)
		return -1;

	Message* newmsg = (Message*)malloc(sizeof(Message)); // message create
	newmsg->type = ((Message*)msgp)->type;
	for(int i=0; i<msgsz; i++)
		newmsg->data[i] = ((Message*)msgp)->data[i];
	newmsg->size = msgsz;
	newmsg->pPrev = NULL;
	newmsg->pNext = NULL;

	push_Message(&(qcbTblEntry[msqid].pQcb->pMsgHead), &(qcbTblEntry[msqid].pQcb->pMsgTail), newmsg);
	(qcbTblEntry[msqid].pQcb->msgCount)++;

	Thread* waketh; // wait thread search
	for(waketh=qcbTblEntry[msqid].pQcb->pThreadHead; waketh!=NULL; waketh=waketh->pNext){
		if(waketh->type == newmsg->type){
			remove_Thread(&(qcbTblEntry[msqid].pQcb->pThreadHead), &(qcbTblEntry[msqid].pQcb->pThreadTail), waketh);
			push_Thread(&ReadyQHead,&ReadyQTail,waketh);

			waketh->status = THREAD_STATUS_READY;
			//printf("wake msqid:%d\n", msqid);
			(qcbTblEntry[msqid].pQcb->waitThreadCount)--;
			break;
		}
	}
	return 0;
}

int	mymsgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg){
	Message* gmsg;
	//printf("rcv msqid:%d\n", msqid);
	GET:
	if(qcbTblEntry[msqid].key == -1)
		return -1;

	gmsg = remove_Message(&(qcbTblEntry[msqid].pQcb->pMsgHead), &(qcbTblEntry[msqid].pQcb->pMsgTail), msgtyp);

	if(gmsg == NULL){ // thread wait
		thread_suspend(thread_self());
		push_Thread(&(qcbTblEntry[msqid].pQcb->pThreadHead), &(qcbTblEntry[msqid].pQcb->pThreadTail), curThread);
		
		curThread->type = msgtyp;
		curThread->bRunnable = 0;
		(qcbTblEntry[msqid].pQcb->waitThreadCount)++;
		//printf("wait msqid:%d\n", msqid);
		__thread_wait_handler(0);
		goto GET;
	}

	((Message*)msgp)->type = gmsg->type; // get message
	for(int i=0; i<msgsz; i++)
		((Message*)msgp)->data[i] = gmsg->data[i];

	(qcbTblEntry[msqid].pQcb->msgCount)--;

	free(gmsg);

	return strlen(((Message*)msgp)->data);
}

int 	mymsgctl(int msqid, int cmd, void* buf){
	if(cmd != MY_IPC_RMID)
		return 0;

	if(qcbTblEntry[msqid].key == -1 || qcbTblEntry[msqid].pQcb == NULL) // no messageQ
		return -1;
	if((qcbTblEntry[msqid].pQcb->msgCount)>0 || (qcbTblEntry[msqid].pQcb->waitThreadCount)>0) // not empty (message or waiting thread)
		return -1;
	if(!(empty_MQueue(qcbTblEntry[msqid].pQcb->pMsgHead)) || !(empty_Queue(qcbTblEntry[msqid].pQcb->pThreadHead))) // not empty (message or waiting thread)
		return -1;

	//printf("msqid:%d mcount:%d tcount:%d\n",msqid,qcbTblEntry[msqid].pQcb->msgCount,qcbTblEntry[msqid].pQcb->waitThreadCount);
	qcbTblEntry[msqid].key = -1; // remove messageQ
	Message* fmsg = pop_Message(&(qcbTblEntry[msqid].pQcb->pMsgHead), &(qcbTblEntry[msqid].pQcb->pMsgTail));
	while(fmsg != NULL){
		free(fmsg);
		fmsg = pop_Message(&(qcbTblEntry[msqid].pQcb->pMsgHead), &(qcbTblEntry[msqid].pQcb->pMsgTail));
	}

	free(qcbTblEntry[msqid].pQcb);
	qcbTblEntry[msqid].pQcb = NULL;
	return 0;
}
