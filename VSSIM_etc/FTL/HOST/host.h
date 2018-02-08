// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#ifndef _HOST_QUEUE_H_
#define _HOST_QUEUE_H_

extern int queue_entry_nb;

typedef struct host_command
{
	int type;				// NOOP, READ, WRITE
	int32_t sector_nb;
	unsigned int length;
}host_command;

typedef struct event_queue
{
	int size;
	int front;
	int rear;

	host_command* host_commands;
}event_queue;

extern event_queue* e_queue;

void INIT_EVENT_QUEUE(event_queue** e_queue);
void TERM_EVENT_QUEUE(event_queue* e_queue);

void WRITE_TO_EVENT_QUEUE(int type, int32_t sector_nb, unsigned int length);
void ENQUEUE_HOST_COMMAND(event_queue* e_queue, host_command* host_cmd);
host_command* DEQUEUE_HOST_COMMAND(event_queue* e_queue);

int IS_EMPTY(event_queue* e_queue);
int IS_EVENT(event_queue* e_queue);
int IS_FULL(event_queue* e_queue);

void *HOST_LOOP(void *arg);
#endif
