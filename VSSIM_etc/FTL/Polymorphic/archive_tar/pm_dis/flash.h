#ifndef __FLASH_H
#define __FLASH_H

#include "common.h"
#include <linux/hrtimer.h>

struct bank_hw {
	// addr_reg;
	// command_reg;

	void * read_buf;
	void * write_buf;

	// dram only
	volatile int status; // 0 = idle, 1 = setting, 2 = working or done
	struct hrtimer hrtimer;

	//void * page_mem[NR_PHY_BLOCKS];
	//void * spare_mem[NR_PHY_BLOCKS];
	unsigned char (* page_mem)[PAGES_PER_BLOCK][PHY_PAGE_SIZE];
	unsigned char (* spare_mem)[PAGES_PER_BLOCK][PHY_PAGE_SIZE];
};

struct bank_cmd {
	struct host_command * host_cmd;

	int type;
	uint32_t block;
	uint32_t page;
	uint32_t left_skip;
	uint32_t right_skip;
	void * buf;
};

int flash_open(void);
void flash_close(void);
int flash_read(void * callback, uint32_t bank, uint32_t block, uint32_t page, void * buf, uint32_t left_skip, uint32_t right_skip);
int flash_write(void * callback, uint32_t bank, uint32_t block, uint32_t page, void * buf);
int flash_partial_write(void * callback, uint32_t bank, uint32_t block, uint32_t page, void * buf,
		uint32_t old_bank, uint32_t old_block, uint32_t old_page,
		uint32_t left_old, uint32_t right_old);
int flash_erase(uint32_t bank, uint32_t block);
enum hrtimer_restart flash_isr(struct hrtimer * ptr);

void *get_flash_page(uint32_t ppn);			// sjh

#endif
