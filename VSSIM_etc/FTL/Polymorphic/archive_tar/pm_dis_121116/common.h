#ifndef __COMMON_H
#define __COMMON_H

// macro for debugging

#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/mman.h>
#include <linux/slab.h>

// CONFIGURABLE constant
#define MAX_EVENTS 64
//#define NR_SECTORS 262144*4

#define NR_PHY_BANKS	4
#define NR_PHY_WAYS		2
#define BLOCKS_PER_BANK	1024  // 64
#define PAGES_PER_BLOCK	128 // 128
#define PHY_PAGE_SIZE	8192
#define PHY_SPARE_SIZE	436

#define READ_LATENCY	 400
#define WRITE_LATENCY	1600
#define ERASE_LATENCY	1500

// calculated constant value
#define SECTORS_PER_PAGE (PHY_PAGE_SIZE / 512)			// 8k

#define NR_PHY_BLOCKS	(NR_PHY_BANKS  * NR_PHY_WAYS * BLOCKS_PER_BANK)
#define NR_PHY_PAGES	(NR_PHY_BLOCKS * PAGES_PER_BLOCK)
#define NR_PHY_SECTORS	(NR_PHY_PAGES * SECTORS_PER_PAGE)

#define NR_RESERVED_PHY_SUPER_BLOCKS	4
#define NR_RESERVED_PHY_BLOCKS	(NR_PHY_BANKS * NR_PHY_WAYS * NR_RESERVED_PHY_SUPER_BLOCKS)
#define NR_RESERVED_PHY_PAGES	(NR_RESERVED_PHY_BLOCKS * PAGES_PER_BLOCK)

// map macro
#define MAP_ENTRIES_PER_PAGE	2048		// 8k / 4byte

// debug macro
#define PRINT_LINE() do { printk("mlc: %s:%4d\n", __FUNCTION__, __LINE__); mdelay(200); } while (0)
//#define ASSERT(x, op, y)
#define ASSERT(x, op, y)	\
		if (!((x) op (y))) {	\
			printk("<0>mlc: ASSERTION '%s(%ld) %s %s(%ld)' fail!" \
					"%s:%d (%s)\n", #x, (long)x, #op, #y, (long)y, __FUNCTION__, __LINE__, __FILE__);	\
			mdelay(100);	\
		}

// define blk_fs_request
#define blk_fs_request(rq)	((rq)->cmd_type == REQ_TYPE_FS)

#define DEBUG_ERR(...)		printk(__VA_ARGS__)
#define DEBUG_WARN(...)		//printk(__VA_ARGS__)
#define DEBUG_INFO(...)		//printk(__VA_ARGS__)

#define DEBUG_KREQUEST(...)	printk(__VA_ARGS__)
#define DEBUG_FTL_RW(...)	printk(__VA_ARGS__)
#define DEBUG_FTL_RW_V(...)	//printk(__VA_ARGS__)
#define DEBUG_QUEUE(...)	//printk(__VA_ARGS__)
#define DEBUG_FLASH(...)	//printk(__VA_ARGS__)
#define DEBUG_FLASH_V(...)	//printk(__VA_ARGS__)

// bit operation
#define BIT_SET(data, bit)		((data) |= (0x01 << (bit)))
#define BIT_GET(data, bit)		((data) & (0x01 << (bit)))
//#define BIT_CLEAR(data, bit) 	((data) &= ~(0x01 << (bit)))
//#define BIT_INVERT(data, bit)	((data) ^= (0x01 << (bit)))

// other
#define CMD_READ 0
#define CMD_WRITE 1
#define CMD_ERASE 2

#include "flash.h"
#include "ftl.h"
#include "cache.h"
#include "pm.h"

#endif
