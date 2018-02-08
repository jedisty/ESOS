#include "host.h"
#include "kernel.h"
#include "ftl.h"

struct event_queue event_queue;
struct event_queue end_queue;

extern int free_page_cnt; 
extern int garbage_collection(void);

int is_event(struct event_queue * queue)
{
	int res;
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);
	res = !list_empty(&queue->head);
	spin_unlock_irqrestore(&queue->lock, flags);
	return res;
}

host_command_t * get_event(struct event_queue * queue)
{
	host_command_t * res = NULL;
	struct list_head * ptr = NULL;
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);
	ASSERT(queue->head.next, !=, &queue->head);
	ptr = queue->head.next;
	res = (host_command_t *)list_entry(ptr, host_command_t, list);
	list_del(ptr);
	spin_unlock_irqrestore(&queue->lock, flags);

	return res;
}

void end_event(host_command_t * cmd, int error)
{
	cmd->error |= error;

	kernel_end_event(cmd, error);

	kfree(cmd);
}

host_command_t * get_free_event(void)
{
	host_command_t * res = kzalloc(sizeof(host_command_t), GFP_ATOMIC);
	atomic_set(&res->remain, 0xFFFF);
	return res;
}

void commit_event(struct event_queue * queue, host_command_t * cmd)
{
	unsigned long flags;

	spin_lock_irqsave(&queue->lock, flags);
	list_add_tail(&cmd->list, &queue->head);
	spin_unlock_irqrestore(&queue->lock, flags);

	wake_up_process(host_thread);
}

void cmd_set_remain(host_command_t * cmd, int remain)
{
	DEBUG_INFO("mlc: %s: cmd %p remain %d\n", __FUNCTION__, cmd, remain);

	if (atomic_add_return(remain - 0xFFFF, &cmd->remain) == 0) {
		DEBUG_INFO("     call end_event()\n");
		end_event(cmd, 0);
	}
	else DEBUG_INFO("     remain %d\n", atomic_read(&cmd->remain));
}

void cmd_dec_remain(host_command_t * cmd, uint32_t error)
{
	cmd->error |= error;

	DEBUG_INFO("mlc: %s: cmd %p\n", __FUNCTION__, cmd);

	if (atomic_dec_and_test(&cmd->remain)) {
		DEBUG_INFO("     insert to end_queue.\n");
		commit_event(&end_queue, cmd);
	}
}

int host_loop(void * unused)
{
	while (!kthread_should_stop()) {
		int busy = 0;

		if (is_event(&event_queue)) {
			int res;
			host_command_t * cmd = get_event(&event_queue);

			if (cmd->type == CMD_READ) {
				kernel_prep_read(cmd);
				res = ftl_read(cmd->lba, cmd->sectors, cmd->buf, cmd);
			}
			else {
				kernel_prep_write(cmd);
//				DEBUG_FTL_RW("host_loop: ftl_write(%10u, %6u, %p)\n", cmd->lba, cmd->sectors, cmd->buf);
				res = ftl_write(cmd->lba, cmd->sectors, cmd->buf, cmd);
			}

//			printk("host_loop: cmd(%d) remain(%d)\n", cmd->remain, res);

			cmd_set_remain(cmd, res);
//			printk("[%s] end event\n", __FUNCTION__);

			busy = 1;
		}
		else if(free_page_cnt <=  NR_PHY_PAGES - (NR_RESERVED_PHY_PAGES + PAGES_PER_BLOCK*NR_PHY_BANKS*1500)){
				printk("Soft TH \n");
				garbage_collection();	
		}

		if (is_event(&end_queue)) {
			host_command_t * cmd = get_event(&end_queue);
			end_event(cmd, cmd->error);

			busy = 1;
		}

		if (!busy) {
//			printk("[%s] not busy\n");
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(10);
		}

//		printk("[%s] loop end\n", __FUNCTION__);
	}

	return 0;
}
