#ifndef __COMMON_H
#define __COMMON_H

// macro for debugging

#define VSSIM_PM
//#define PM_DEBUG

#ifdef VSSIM_PM
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <malloc.h>
	#include <stdint.h>
	#include "ftl_config_manager.h"
	#include "ftl_vssim_cache.h"
	#include "ftl_perf_manager.h"
#else
	#include <linux/types.h>
	#include <linux/vmalloc.h>
	#include <linux/delay.h>
	#include <linux/mman.h>
	#include <linux/slab.h>
#endif

#ifdef VSSIM_PM
	#define NOOP 		10000
	#define READ 		10001
	#define WRITE 		10002
	#define ERASE 		10003
	#define GC_READ		10004
	#define GC_WRITE	10005
	#define SEQ_WRITE	10006
	#define RAN_WRITE	10007
	#define SEQ_MERGE_READ	10008
	#define RAN_MERGE_READ	10009
	#define SEQ_MERGE_WRITE	10010
	#define RAN_MERGE_WRITE	10011
	#define MAP_READ	10012
	#define MAP_WRITE	10013

	#define CH_OP		10100
	#define REG_OP		10101
	#define LATENCY_OP	10102

	#define UPDATE_START_TIME	10200
	#define UPDATE_END_TIME		10201
	#define UPDATE_GC_START_TIME	10202
	#define UPDATE_GC_END_TIME	10203

	#define SUCCESS		10300
	#define FAIL		10301

	#define MONITOR_ON
	#define FTL_DEBUG
#endif

// CONFIGURABLE constant
#define MAX_EVENTS 64
//#define NR_SECTORS 262144*4

#ifndef VSSIM_PM
	#define NR_PHY_BANKS	4
	#define NR_PHY_WAYS	2
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
#endif

// map macro
#define MAP_ENTRIES_PER_PAGE	2048		// 8k / 4byte

// debug macro

#ifdef VSSIM_PM
	#define PRINT_LINE() do { printf("mlc: %s:%4d\n", __FUNCTION__, __LINE__); } while (0)
#else
	#define PRINT_LINE() do { printk("mlc: %s:%4d\n", __FUNCTION__, __LINE__); mdelay(200); } while (0)
#endif

//#define ASSERT(x, op, y)

#ifdef VSSIM_PM
	#define ASSERT(x, op, y)	\
		if (!((x) op (y))) {	\
			printf("<0>mlc: ASSERTION '%s(%ld) %s %s(%ld)' fail!" \
					"%s:%d (%s)\n", #x, (long)x, #op, #y, (long)y, __FUNCTION__, __LINE__, __FILE__);	\
		}
#else
	#define ASSERT(x, op, y)	\
		if (!((x) op (y))) {	\
			printk("<0>mlc: ASSERTION '%s(%ld) %s %s(%ld)' fail!" \
					"%s:%d (%s)\n", #x, (long)x, #op, #y, (long)y, __FUNCTION__, __LINE__, __FILE__);	\
			mdelay(100);	\
		}
#endif


// define blk_fs_request
#define blk_fs_request(rq)	((rq)->cmd_type == REQ_TYPE_FS)

#ifdef VSSIM_PM
	#define DEBUG_ERR(...)		printf(__VA_ARGS__)
	#define DEBUG_WARN(...)		//printf(__VA_ARGS__)
	#define DEBUG_INFO(...)		//printf(__VA_ARGS__)
	#define DEBUG_KREQUEST(...)	printf(__VA_ARGS__)
	#define DEBUG_FTL_RW(...)	printf(__VA_ARGS__)
	#define DEBUG_FTL_RW_V(...)	//printf(__VA_ARGS__)
	#define DEBUG_QUEUE(...)	//printf(__VA_ARGS__)
	#define DEBUG_FLASH(...)	//printf(__VA_ARGS__)
	#define DEBUG_FLASH_V(...)	//printf(__VA_ARGS__)
#else
	#define DEBUG_ERR(...)		printk(__VA_ARGS__)
	#define DEBUG_WARN(...)		//printk(__VA_ARGS__)
	#define DEBUG_INFO(...)		//printk(__VA_ARGS__)
	#define DEBUG_KREQUEST(...)	printk(__VA_ARGS__)
	#define DEBUG_FTL_RW(...)	printk(__VA_ARGS__)
	#define DEBUG_FTL_RW_V(...)	//printk(__VA_ARGS__)
	#define DEBUG_QUEUE(...)	//printk(__VA_ARGS__)
	#define DEBUG_FLASH(...)	//printk(__VA_ARGS__)
	#define DEBUG_FLASH_V(...)	//printk(__VA_ARGS__)
#endif

// other
#define CMD_READ 0
#define CMD_WRITE 1
#define CMD_ERASE 2

// bit operation
#define BIT_SET(data, bit)              ((data) |= (0x01 << (bit)))
#define BIT_GET(data, bit)              ((data) & (0x01 << (bit)))
//#define BIT_CLEAR(data, bit)  ((data) &= ~(0x01 << (bit)))
//#define BIT_INVERT(data, bit) ((data) ^= (0x01 << (bit)))

#ifndef VSSIM_PM
	#include "flash.h"
#endif

#include "ftl.h"
#include "cache.h"
#include "pm.h"

#endif
