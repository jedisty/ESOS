#ifndef __HOST_H
#define __HOST_H

#include "common.h"

typedef struct host_command {
	int type;		// CMD_READ, CMD_WRITE
	sector_t lba;
	ssize_t sectors;
	void * buf;

	atomic_t remain;	// remaining issued flash operation
	uint32_t error;

	// kernel
	struct list_head list;
	struct request * req;
	//struct bio * bio;
} host_command_t;

struct event_queue {
	struct list_head head;
	spinlock_t lock;
};

extern struct event_queue event_queue;
extern struct event_queue end_queue;

host_command_t * get_free_event(void);
void commit_event(struct event_queue * queue, host_command_t * cmd);
void cmd_dec_remain(host_command_t * cmd, uint32_t error);
int host_loop(void * unused);

#endif
