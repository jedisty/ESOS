#include <linux/kthread.h>

#include "flash.h"
#include "ftl.h"
#include "host.h"

struct bank_hw bank_hw[NR_PHY_BANKS * NR_PHY_WAYS];
struct bank_cmd bank_cmd[NR_PHY_BANKS * NR_PHY_WAYS];

static const ktime_t ktime_read =  { .tv64 =  READ_LATENCY * 1000 };
static const ktime_t ktime_write = { .tv64 = WRITE_LATENCY * 1000 };
static const ktime_t ktime_erase = { .tv64 = ERASE_LATENCY * 1000 };

int flash_open(void)
{
	int i;
	
	for (i=0; i<NR_PHY_BANKS*NR_PHY_WAYS; i++) {
		bank_hw[i].page_mem = vmalloc(BLOCKS_PER_BANK * PHY_PAGE_SIZE * PAGES_PER_BLOCK);
		bank_hw[i].spare_mem = vmalloc(BLOCKS_PER_BANK * PHY_SPARE_SIZE * PAGES_PER_BLOCK);
		if (!bank_hw[i].page_mem || !bank_hw[i].spare_mem) {
			DEBUG_ERR("mlc: flash memory page allocation fail! bank %d %p %p\n",
					i, bank_hw[i].spare_mem, bank_hw[i].page_mem);
			return -1;
		}
		
//		printk("mlc: flash_open() for[%d] page,spare \n", i);

		bank_hw[i].read_buf = vmalloc(PHY_PAGE_SIZE);
		bank_hw[i].write_buf = vmalloc(PHY_PAGE_SIZE);
		if (!bank_hw[i].read_buf || !bank_hw[i].write_buf) {
			DEBUG_ERR("mlc: flash memory buffer allocation fail! bank %d %p %p\n",
					i, bank_hw[i].write_buf, bank_hw[i].read_buf);
			return -1;
		}
//		printk("mlc: flash_open() for[%d] read, write buf \n", i);

		bank_hw[i].status = 0;

		hrtimer_init(&bank_hw[i].hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		bank_hw[i].hrtimer.function = &flash_isr;
		
//		printk("mlc: flash_open() for[%d] \n", i);
	}
	
	return 0;
}

void flash_close(void)
{
	int i;

	for (i=0; i<NR_PHY_BANKS*NR_PHY_WAYS; i++) {

		// TODO: wait for all hrtimer events

		vfree(bank_hw[i].page_mem);
		vfree(bank_hw[i].spare_mem);

		vfree(bank_hw[i].read_buf);
		vfree(bank_hw[i].write_buf);
	}
}

int flash_read(void * callback, uint32_t bank, uint32_t block, uint32_t page,
		void * buf, uint32_t left_skip, uint32_t right_skip)
{
	void * ptr;

	ASSERT(callback, !=, NULL);
	ASSERT(bank,  <, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(block, <, BLOCKS_PER_BANK);
	ASSERT(page,  <, PAGES_PER_BLOCK);
	ASSERT(buf,  !=, NULL);

	DEBUG_FLASH("mlc: %s: bank %u block %u page %u\n", __FUNCTION__, bank, block, page);

	while (cmpxchg(&bank_hw[bank].status, 0, 1) != 0) cpu_relax();
	hrtimer_cancel(&bank_hw[bank].hrtimer); // wait 용도로 사용, 최적화 필요

	DEBUG_FLASH_V("mlc: %s:     fill bank_cmd\n", __FUNCTION__);

	bank_cmd[bank].type = CMD_READ;
	bank_cmd[bank].host_cmd = callback;
	bank_cmd[bank].block = block;
	bank_cmd[bank].page = page;
	bank_cmd[bank].left_skip = left_skip;
	bank_cmd[bank].right_skip = right_skip;
	bank_cmd[bank].buf = buf;

	// NAND operation
	ptr = bank_hw[bank].page_mem[block][page];
	memcpy(bank_hw[bank].read_buf, ptr, PHY_PAGE_SIZE);
	bank_hw[bank].status = 2;
	hrtimer_start(&bank_hw[bank].hrtimer, ktime_read, HRTIMER_MODE_REL);
	// NAND operation

	DEBUG_FLASH_V("mlc: %s:     issued\n", __FUNCTION__);

	return 0;
}

int flash_write(void * callback, uint32_t bank, uint32_t block, uint32_t page, void * buf)
{
	void * ptr;

	ASSERT(callback, !=, NULL);
	ASSERT(bank,  <, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(block, <, BLOCKS_PER_BANK);
	ASSERT(page,  <, PAGES_PER_BLOCK);
	ASSERT(buf,  !=, NULL);

	DEBUG_FLASH("mlc: %s: bank %u block %u page %u\n", __FUNCTION__, bank, block, page);

	while (cmpxchg(&bank_hw[bank].status, 0, 1) != 0) cpu_relax();
	hrtimer_cancel(&bank_hw[bank].hrtimer);

	DEBUG_FLASH_V("mlc: %s:     fill bank_cmd\n", __FUNCTION__);

	bank_cmd[bank].type = CMD_WRITE;
	bank_cmd[bank].host_cmd = callback;
	bank_cmd[bank].block = block;
	bank_cmd[bank].page = page;
	bank_cmd[bank].left_skip = bank_cmd[bank].right_skip = 0;
	bank_cmd[bank].buf = buf;

	memcpy(bank_hw[bank].write_buf, buf, PHY_PAGE_SIZE);

	// NAND operation
	ptr = bank_hw[bank].page_mem[block][page];
	memcpy(ptr, bank_hw[bank].write_buf, PHY_PAGE_SIZE);
	bank_hw[bank].status = 2;
	hrtimer_start(&bank_hw[bank].hrtimer, ktime_write, HRTIMER_MODE_REL);
	// NAND operation

	DEBUG_FLASH_V("mlc: %s:     issued\n", __FUNCTION__);

	return 0;
}

int flash_partial_write(void * callback, uint32_t bank, uint32_t block, uint32_t page,
		void * buf, uint32_t old_bank, uint32_t old_block, uint32_t old_page,
		uint32_t left_old, uint32_t right_old)
{
	void * ptr;

	ASSERT(callback, !=, NULL);
	ASSERT(bank,  <, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(block, <, BLOCKS_PER_BANK);
	ASSERT(page,  <, PAGES_PER_BLOCK);
	ASSERT(buf,  !=, NULL);
	ASSERT(old_bank,  <, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(old_block, <, BLOCKS_PER_BANK);
	ASSERT(old_page,  <, PAGES_PER_BLOCK);

	DEBUG_FLASH("mlc: %s: bank %u block %u page %u\n", __FUNCTION__, bank, block, page);

	while (cmpxchg(&bank_hw[bank].status, 0, 1) != 0) cpu_relax();
	hrtimer_cancel(&bank_hw[bank].hrtimer);

	DEBUG_FLASH_V("mlc: %s:     fill bank_cmd\n", __FUNCTION__);

	// read old data to write buffer
	if (old_bank == old_block && old_block == old_page && old_page == 0) {
		memset(bank_hw[bank].write_buf, 0xFF, PHY_PAGE_SIZE);
	}
	else {
		// TODO : fill bank_cmd
		ptr = bank_hw[old_bank].page_mem[old_block][old_page];
		memcpy(bank_hw[bank].write_buf, ptr, PHY_PAGE_SIZE);
//		udelay(READ_LATENCY);
	}

	// fill new data to write buffer
	memcpy(bank_hw[bank].write_buf + left_old, buf, PHY_PAGE_SIZE - left_old - right_old);

	// write old + new data
	bank_cmd[bank].type = CMD_WRITE;
	bank_cmd[bank].host_cmd = callback;
	bank_cmd[bank].block = block;
	bank_cmd[bank].page = page;
	bank_cmd[bank].left_skip = left_old;
	bank_cmd[bank].right_skip = right_old;
	bank_cmd[bank].buf = buf;

	// NAND operation
	ptr = bank_hw[bank].page_mem[block][page];
	memcpy(ptr, bank_hw[bank].write_buf, PHY_PAGE_SIZE);
	bank_hw[bank].status = 2;
	hrtimer_start(&bank_hw[bank].hrtimer, ktime_write, HRTIMER_MODE_REL);
	// NAND operation

	DEBUG_FLASH_V("mlc: %s:     issued\n", __FUNCTION__);

	return 0;
}

int flash_erase(uint32_t bank, uint32_t block)
{
	ASSERT(bank,  <, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(block, <, BLOCKS_PER_BANK);

	DEBUG_FLASH("mlc: %s: bank %u block %u\n", __FUNCTION__, bank, block);

	while (cmpxchg(&bank_hw[bank].status, 0, 1) != 0) cpu_relax();
	hrtimer_cancel(&bank_hw[bank].hrtimer);

	DEBUG_FLASH_V("mlc: %s:     fill bank_cmd\n", __FUNCTION__);

	bank_cmd[bank].type = CMD_ERASE;
	bank_cmd[bank].host_cmd = (void *)0xE5BEDDED;
	bank_cmd[bank].block = block;
	bank_cmd[bank].page = bank_cmd[bank].left_skip = bank_cmd[bank].right_skip = 0;

	DEBUG_FLASH("mlc: %s: bank %u block %u\n", __FUNCTION__, bank, block);

	// NAND operation
	memset(bank_hw[bank].page_mem[block], 0xFF, PHY_PAGE_SIZE * PAGES_PER_BLOCK);
	bank_hw[bank].status = 2;
	hrtimer_start(&bank_hw[bank].hrtimer, ktime_erase, HRTIMER_MODE_REL);
	// NAND operation

	DEBUG_FLASH_V("mlc: %s:     issued\n", __FUNCTION__);

	return 0;
}

enum hrtimer_restart flash_isr(struct hrtimer * ptr)
{
	uint32_t bank;

	for (bank=0; bank<NR_PHY_BANKS*NR_PHY_WAYS; bank++) {
		if (&bank_hw[bank].hrtimer == ptr) break;
	}

	ASSERT(bank, !=, NR_PHY_BANKS * NR_PHY_WAYS);
	ASSERT(bank_cmd[bank].host_cmd, !=, NULL);

	DEBUG_FLASH("mlc: %s: bank %u expired (block %u page %u)\n", __FUNCTION__,
			bank, bank_cmd[bank].block, bank_cmd[bank].page);

	ASSERT(bank_hw[bank].status, ==, 2);

	switch (bank_cmd[bank].type) {
		case CMD_READ:
			ASSERT(bank_cmd[bank].buf, !=, NULL);
			memcpy(bank_cmd[bank].buf, bank_hw[bank].read_buf + bank_cmd[bank].left_skip,
					PHY_PAGE_SIZE - bank_cmd[bank].left_skip - bank_cmd[bank].right_skip);
			break;
		case CMD_WRITE:
		case CMD_ERASE:
			;
	}

	cmd_dec_remain(bank_cmd[bank].host_cmd, 0);

	bank_cmd[bank].host_cmd = NULL;
	bank_hw[bank].status = 0;

#ifdef DEBUG_
	DEBUG_FLASH_V("mlc: %s:     done.\n", __FUNCTION__);
#endif

	return HRTIMER_NORESTART;
}

void *get_flash_page(uint32_t ppn)
{
	uint32_t bank = ftl_calc_bank(ppn);
	uint32_t block = ftl_calc_block(ppn);
	uint32_t page = ftl_calc_page(ppn);
	
#ifdef DEBUG_
	printk("[%s] ppn (%d) bank (%d) block (%d) page (%d)\n", __FUNCTION__, ppn, bank, block, page);
#endif
	return bank_hw[bank].page_mem[block][page];
}
