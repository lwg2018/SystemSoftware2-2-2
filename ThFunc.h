#ifndef __THFUNC_H__
#define __THFUNC_H__

#include "Thread.h"
#include "MsgQueue.h"
#include <signal.h>

Thread* curThread;

typedef struct message{
	long type;
	char text[20];
} mymsg;

Thread* make_Thread();
void __thread_wait_handler(int signo);
void __thread_wakeup(Thread* wake);
Thread* __getThread(thread_t tid);
Thread* __getThreadW(thread_t tid);

/* Double linked list functions */
void push_Thread(Thread **head, Thread **tail, Thread *new);
Thread *pop_Thread(Thread **head, Thread **tail);
void remove_Thread(Thread **head, Thread **tail, Thread* target);
BOOL empty_Queue(Thread *head);

void push_Message(Message **head, Message **tail, Message *new);
Message *remove_Message(Message **head, Message **tail, long mtype);
Message *pop_Message(Message **head, Message **tail);
BOOL empty_MQueue(Message *head);

#endif /* __THFUNC_H__ */
