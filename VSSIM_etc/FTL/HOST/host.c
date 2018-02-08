// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#include "common.h"

#ifdef HOST_QUEUE

event_queue* e_queue;

/* Condition Lock */
pthread_t th_id1;
pthread_cond_t t_ready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t t_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;

/* Debug Variable */
int enqueue_count=0;
int dequeue_count=0;
int queue_entry_nb=0;

void INIT_EVENT_QUEUE(event_queue** e_queue)
{
	(*e_queue) = (event_queue*)malloc(sizeof(event_queue));
	(*e_queue)->host_commands = (host_command*)malloc(sizeof(host_command)*(HOST_QUEUE_ENTRY_NB+1));

	(*e_queue)->size = HOST_QUEUE_ENTRY_NB;
	(*e_queue)->front = 0;
	(*e_queue)->rear = 0;

	pthread_create(&th_id1, NULL, HOST_LOOP, NULL);
}

void TERM_EVENT_QUEUE(event_queue* e_queue)
{
	free(e_queue->host_commands);
	free(e_queue);
}

void WRITE_TO_EVENT_QUEUE(int type, int32_t sector_nb, unsigned int length)
{
	host_command h_cmd;
	h_cmd.type = type;
	h_cmd.sector_nb = sector_nb;
	h_cmd.length = length;

//	pthread_mutex_lock(&t_lock);
	//printf("[%s] get thread lock!!\n",__FUNCTION__);
	while(IS_FULL(e_queue)==1){
//		printf("[%s] queue is full!!\n",__FUNCTION__);
		pthread_cond_signal(&t_ready);
	}

	ENQUEUE_HOST_COMMAND(e_queue, &h_cmd);
	//printf("[%s] queueing cmd complete\n",__FUNCTION__);
//	pthread_mutex_unlock(&t_lock);

	pthread_cond_signal(&t_ready);
}

void ENQUEUE_HOST_COMMAND(event_queue* e_queue, host_command* host_cmd)
{
	pthread_mutex_lock(&q_lock);
	/*
	if(e_queue->rear == e_queue->size + 1){
		e_queue->rear = 0;
		position = 0;
	}
	else{
		position = e_queue->rear++;
	}
	*/
	int position = e_queue->rear;

	e_queue->host_commands[position].type = host_cmd->type;
	e_queue->host_commands[position].sector_nb = host_cmd->sector_nb;
	e_queue->host_commands[position].length = host_cmd->length;

	e_queue->rear++;
	if(e_queue->rear == e_queue->size + 1){
		e_queue->rear = 0;
	}

	enqueue_count++;
	queue_entry_nb++;
	pthread_mutex_unlock(&q_lock);
}

host_command* DEQUEUE_HOST_COMMAND(event_queue* e_queue)
{
	pthread_mutex_lock(&q_lock);
	int position = e_queue->front;
	host_command* h_cmd;

//	if(e_queue->front == e_queue->size)
//		e_queue->front = 0;
//	else
//		e_queue->front++;

	h_cmd = &e_queue->host_commands[position];

	e_queue->front++;
	if(e_queue->front == e_queue->size + 1){
		e_queue->front = 0;
	}

	queue_entry_nb--;
	dequeue_count++;

	pthread_mutex_unlock(&q_lock);
	return h_cmd;
}

int IS_EMPTY(event_queue* e_queue)
{
	pthread_mutex_lock(&q_lock);
	int ret = (e_queue->front == e_queue->rear);
	pthread_mutex_unlock(&q_lock);
	return ret;
}

int IS_EVENT(event_queue* e_queue)
{
	int ret = !(IS_EMPTY(e_queue));
	return ret;
}

int IS_FULL(event_queue* e_queue)
{
	pthread_mutex_lock(&q_lock);
	int ret;
	if(e_queue->front < e_queue->rear)
		ret = ( e_queue->rear - e_queue->front ) == e_queue->size;
	else
		ret = ( e_queue->rear + 1 ) == e_queue->front;

	pthread_mutex_unlock(&q_lock);

	return ret;
}

void *HOST_LOOP(void *arg)
{
	while(1){
//		printf("[%s] enqueue %d / dequeue %d\n",__FUNCTION__, enqueue_count, dequeue_count);
//		pthread_mutex_lock(&t_lock);
		//printf("[%s] get lock! \n", __FUNCTION__);
	
		int busy = 0;
		if(IS_EMPTY(e_queue)==0){
			//printf("[%s] Get Event! \n", __FUNCTION__);

			host_command* h_cmd = DEQUEUE_HOST_COMMAND(e_queue);

			if(h_cmd->type == READ){
				FTL_READ(h_cmd->sector_nb, h_cmd->length);
			}
			else if(h_cmd->type == WRITE){
				FTL_WRITE(h_cmd->sector_nb, h_cmd->length);
			}
			else if(h_cmd->type == NOOP){
				printf("ERROR[%s] NOOP operation \n", __FUNCTION__);
			}
			else{
				printf("ERROR[%s] ?? operation \n", __FUNCTION__);
			}
			
			h_cmd->type = NOOP;
			busy = 1;
		}
#ifdef Polymorphic
		else if(free_page_cnt <= NR_PHY_PAGES - (NR_RESERVED_PHY_PAGES + PAGE_NB*FLASH_NB*2700)){
			printf("Soft TH\n");
			garbage_collection();
		}
#endif

//		if(!busy){
		while(IS_EMPTY(e_queue)==1 && busy == 0){
		//	printf("[%s] Empty queue! go sleep \n", __FUNCTION__);
			pthread_cond_wait(&t_ready, &t_lock);
//			printf("[%s] Get signal \n", __FUNCTION__);
		}
//		pthread_mutex_unlock(&t_lock);	
	//	printf("[%s]Host loop end \n", __FUNCTION__);
	}	
}

#endif
