#ifndef __KERNEL_H
#define __KERNEL_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

#include <linux/kthread.h>

struct host_command;
int kernel_prep_read(struct host_command * cmd);
int kernel_prep_write(struct host_command * cmd);
int kernel_end_event(struct host_command * cmd, int error);

extern struct task_struct * host_thread;

MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

#endif
