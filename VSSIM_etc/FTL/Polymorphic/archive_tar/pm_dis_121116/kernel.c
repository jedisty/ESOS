#include "common.h"
#include "kernel.h"
#include "flash.h"
#include "ftl.h"
#include "cache.h"
#include "host.h"

static int kernel_getgeo(struct block_device * block_device, struct hd_geometry * geo);
static struct block_device_operations kernel_fops = {
	.owner = THIS_MODULE,
	.getgeo = kernel_getgeo,
};

static struct kernel_device {
	spinlock_t lock;
	struct gendisk * gd;
} kernel_device;

static struct proc_dir_entry * kernel_proc_entry;
static int kernel_dev_major;

struct task_struct * host_thread;

static struct request_queue * kernel_queue;
static spinlock_t kernel_queue_lock;

static void kernel_request(struct request_queue *q)
{
	host_command_t * cmd = NULL;
	struct request *req;

	req = blk_fetch_request(q);
	while (req != NULL) {
		if (!blk_fs_request(req)) {
			DEBUG_INFO("Skip non-CMD request\n");
			__blk_end_request_all(req, -EIO);
			req = blk_fetch_request(q);
			continue;
		}
		else if (blk_rq_pos(req) + blk_rq_cur_sectors(req) > get_capacity(kernel_device.gd))
		{
			printk(KERN_ERR "mlc: Skip beyond-end write, pos %lu nsects %u cap %lu\n",
					blk_rq_pos(req), blk_rq_cur_sectors(req), get_capacity(kernel_device.gd));
			__blk_end_request_all(req, -EIO);
			req = blk_fetch_request(q);
			continue;
		}

		ASSERT(blk_rq_sectors(req), <=, 32 * 4096 / 512);

		cmd = get_free_event();

		if (!cmd) {
			DEBUG_WARN("mlc: event_queue is full (bio: %p,%ld)\n", bio, (long)bio->bi_sector);
			ASSERT(1, ==, 0); // TODO
		}

		INIT_LIST_HEAD(&cmd->list);
		cmd->type = rq_data_dir(req);
		cmd->lba = blk_rq_pos(req);
		cmd->sectors = blk_rq_sectors(req);
		cmd->buf = NULL; // alloc in kernel_prep_XXX()
		cmd->req = req;

		commit_event(&event_queue, cmd);

		req = blk_fetch_request(q);
	}
}

int kernel_prep_read(host_command_t * cmd)
{
	cmd->buf = kmalloc(cmd->sectors * 512, GFP_KERNEL | GFP_ATOMIC);
	ASSERT(cmd->buf, !=, NULL);

	return 0;
}

int kernel_prep_write(host_command_t * cmd)
{
	struct bio_vec * bvec;
	struct req_iterator iter;
	int ptr = 0;

	cmd->buf = kmalloc(cmd->sectors * 512, GFP_KERNEL | GFP_ATOMIC);
	ASSERT(cmd->buf, !=, NULL);

	rq_for_each_segment(bvec, cmd->req, iter) {
		void * kernel_buf = kmap(bvec->bv_page) + bvec->bv_offset;
		memcpy(cmd->buf + ptr, kernel_buf, bvec->bv_len);
		kunmap(bvec->bv_page);
		ptr += bvec->bv_len;
	}

	ASSERT(ptr, ==, cmd->sectors * 512);

	return !(ptr == cmd->sectors * 512);
}

int kernel_end_event(host_command_t * cmd, int error)
{
	unsigned long flags;

	if (cmd->type == READ)
	{
		struct bio_vec * bvec;
		struct req_iterator iter;
		int ptr = 0;

		rq_for_each_segment(bvec, cmd->req, iter) {
			void * kernel_buf = kmap(bvec->bv_page) + bvec->bv_offset;
			memcpy(kernel_buf, cmd->buf + ptr, bvec->bv_len);
			kunmap(bvec->bv_page);
			ptr += bvec->bv_len;
		}

		ASSERT(ptr, ==, cmd->sectors * 512);
	}

	kfree(cmd->buf);

	// TODO: end seperately
	spin_lock_irqsave(kernel_queue->queue_lock, flags);
	__blk_end_request_all(cmd->req, 0);
	spin_unlock_irqrestore(kernel_queue->queue_lock, flags);

	return 0;
}

static int kernel_proc_read(char * page, char ** start, off_t off,
		int count, int * eof, void * data)
{
	char *buf = page;

	buf += sprintf(buf, "mlc: proc_read()\n");

	return (buf - page < count) ? buf - page : count;
}

static int kernel_proc_write(struct file * filp, const char __user * buff,
		unsigned long len, void* data )
{
	char cmd[20];

	sscanf(buff, "%s", cmd);

	if(strcmp(cmd, "stat") == 0) {
		printk("mlc: proc_write(): test\n");
	}
	else {
		printk("mlc: proc_write(): unknown command\n");
	}

	return len;
}

static int kernel_getgeo(struct block_device * block_device,
		struct hd_geometry * geo)
{
	geo->heads = 255;
	geo->sectors = 63;
	geo->cylinders = (NR_PHY_SECTORS + 255 * 63 - 1) / 255 / 63;
	geo->start = 0;

	return 0;
}

static int kernel_device_init(void)
{
	// device & queue init
	spin_lock_init(&kernel_device.lock);
	spin_lock_init(&kernel_queue_lock);
	kernel_queue = blk_init_queue(kernel_request, &kernel_queue_lock);
	//kernel_queue = blk_init_queue(kernel_request, &kernel_queue->__queue_lock);
	if (!kernel_queue) goto out;
	//kernel_queue->queue_lock = &kernel_queue->__queue_lock;
	blk_queue_io_min(kernel_queue, PHY_PAGE_SIZE);
	blk_queue_io_opt(kernel_queue, PHY_PAGE_SIZE * NR_PHY_WAYS * NR_PHY_BANKS);

	// device register
	kernel_dev_major = register_blkdev(kernel_dev_major, "mlc");
	if (kernel_dev_major <= 0) goto out;

	// gendisk init & register
	kernel_device.gd = alloc_disk(16);
	if (!kernel_device.gd) goto out_unregister;
	kernel_device.gd->major = kernel_dev_major;
	kernel_device.gd->first_minor = 0;
	kernel_device.gd->fops = &kernel_fops;
	kernel_device.gd->private_data = &kernel_device;
	strcpy(kernel_device.gd->disk_name, "mlc0");
	set_capacity(kernel_device.gd, NR_PHY_SECTORS * 9 / 10);
	kernel_device.gd->queue = kernel_queue;
	add_disk(kernel_device.gd);

	// proc fs register
	kernel_proc_entry = create_proc_entry("mlc", S_IRUGO | S_IFREG, NULL);
	kernel_proc_entry->write_proc = kernel_proc_write;
	kernel_proc_entry->read_proc = kernel_proc_read;
	kernel_proc_entry->data = NULL;

	return 0;

out_unregister:
	unregister_blkdev(kernel_dev_major, "mlc");
out:
	return -ENOMEM;
}

static void kernel_device_exit(void)
{
	remove_proc_entry("mlc", NULL);
	del_gendisk(kernel_device.gd);
	put_disk(kernel_device.gd);
	unregister_blkdev(kernel_dev_major, "mlc");
	blk_cleanup_queue(kernel_queue);
}

static int __init kernel_init(void)
{
	printk("mlc: init()\n");
	spin_lock_init(&event_queue.lock);
	INIT_LIST_HEAD(&event_queue.head);
	spin_lock_init(&end_queue.lock);
	INIT_LIST_HEAD(&end_queue.head);

	flash_open();
	ftl_open();

	host_thread = kthread_run(host_loop, NULL, "%s", "host_loop");
	mdelay(1);
	kernel_device_init();
	printk("mlc: init() done.\n");

	return 0;
}

static void __exit kernel_exit(void)
{
	printk("mlc: exit()\n");
	kernel_device_exit();
	kthread_stop(host_thread);

	ftl_close();
	flash_close();
}

module_init(kernel_init);
module_exit(kernel_exit);
