#ifndef __FTL_H
#define __FTL_H

#include "common.h"

// These macro are used for setting mapping type that is being used.
#define SET_ZERO_BIT(idx, bit)		(idx = (idx & 0x7FFFFFFF))
#define SET_FLAG_BIT(idx, bit)		(idx = (idx | (0x00000001 << bit)))
#define GET_FLAG_BIT(idx, bit)		(idx & (0x00000001 << bit))

// block bit map
#define SET_FREE_BLK(idx, bit)		SET_FLAG_BIT(idx, bit)
#define GET_FREE_BLK(idx, bit)		GET_FLAG_BIT(idx, bit)
#define GET_BLK_PAGENUM(blk)		(blk / 32)
#define GET_BLK_OFFSET(blk)		(blk % 32)

#define GET_DIVIDED_REGION(lba)						\
	((GET_ROOT_NUM(lba/SECTORS_PER_PAGE, NUM_PAGE_PER_ROOT)+1) 	\
	 * NUM_PAGE_PER_ROOT * SECTORS_PER_PAGE)

#pragma pack(4)
struct ftl_metadata_ {
	// block pointer
	uint32_t reserved_block_ptr;	// block in reserved region
	uint32_t block_ptr;		// recently allocated block
	uint32_t write_block_ptr;	// data block
	uint32_t write_map_block_ptr;	// map block
	// page pointer
	uint32_t reserved_write_ptr;	// region page in reserved region
	uint32_t write_ptr;		// data page in data block
	uint32_t write_map_ptr;		// map page pointer in map block
	// count
    uint32_t free_page_cnt;
    uint32_t reserved_free_page_cnt;
};
#pragma pack()

// ftl operation
int ftl_open(void);
void ftl_close(void);
void ftl_read_map(uint32_t ppn, void *cache);

#ifdef VSSIM_PM
int ftl_read(uint32_t lba, uint32_t sectors);
int ftl_write(uint32_t lba, uint32_t sectors);
void ftl_write_map(uint32_t new_ppn);
#else
int ftl_read(uint32_t lba, uint32_t sectors, void * buf, void * callback);
int ftl_write(uint32_t lba, uint32_t sectors, void * buf, void * callback);
void ftl_write_map(uint32_t new_ppn, void *buf, void *callback);
#endif

// calculate number
uint32_t ftl_calc_bank(uint32_t ppn);
uint32_t ftl_calc_block(uint32_t ppn);
uint32_t ftl_calc_page(uint32_t ppn);

// free page
uint32_t get_free_map_page(void);
uint32_t get_free_page(void);
uint32_t get_free_page_in_reserved(void);

// block bit map
void set_free_blk_bitmap(uint32_t blk_num);
uint32_t get_free_blk_bitmap(uint32_t blk_num);

// TODO
// int ftl_garbage_collection(...);
// void ftl_flush(void);
// bad block management
// format, save, load

#endif
