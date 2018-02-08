#ifndef __FTL_H
#define __FTL_H

#include "common.h"

/* 
 * macro 
 */
// block bit map
#define SET_FREE_BLK(data, bit)	BIT_SET(data, bit)		// set free block bit
#define GET_FREE_BLK(data, bit)	BIT_GET(data, bit)		// get free block bit
#define GET_BLK_PAGENUM(blk)	(blk / 32)				// block이 들어갈 page number
#define GET_BLK_OFFSET(blk)		(blk % 32)				// block이 들어갈 page offset

// 같은 root인지 확인한다 x = lba , y = sectors
#define IS_SAME_REGION(x, y)	\
	((x/SECTORS_PER_PAGE/NUM_PAGE_PER_ROOT) == ((x+y-1)/SECTORS_PER_PAGE/NUM_PAGE_PER_ROOT))
// 다음 root의 첫번째 lba를 가져온다
#define GET_DIVIDED_REGION(lba)									\
	((GET_ROOT_NUM(lba/SECTORS_PER_PAGE, NUM_PAGE_PER_ROOT)+1) 	\
	 * NUM_PAGE_PER_ROOT * SECTORS_PER_PAGE)

/* 
 * data structure 
 */
struct ftl_metadata_ {
	// block pointer
	uint32_t reserved_block_ptr;	// block in reserved region
	uint32_t block_ptr;				// recently allocated block
	uint32_t write_block_ptr;		// data block
	uint32_t write_map_block_ptr;	// map block
	// page pointer
	uint32_t reserved_write_ptr;	// region page in reserved region
	uint32_t write_ptr;				// data page in data block
	uint32_t write_map_ptr;			// map page pointer in map block
};

/* 
 * user defined functions 
 */
int ftl_open(void);
void ftl_close(void);

// ftl operation
int ftl_init(void *mem);
void ftl_read_map(uint32_t ppn, void *cache);

#ifdef VSSIM_PM
void ftl_read(uint32_t lba, uint32_t sectors);
void ftl_write(uint32_t lba, uint32_t sectors);
void ftl_write_map(uint32_t new_ppn, void* cache);
#else
int ftl_read(uint32_t lba, uint32_t sectors, void * buf, void * callback);
int ftl_write(uint32_t lba, uint32_t sectors, void * buf, void * callback);
void ftl_write_map(uint32_t new_ppn, void *buf, void *callback);
#endif

// calculate number
uint32_t ftl_calc_bank(uint32_t ppn);
uint32_t ftl_calc_block(uint32_t ppn);
uint32_t ftl_calc_page(uint32_t ppn);

// free block
// static uint32_t ftl_get_free_block_reserved(void)
// static uint32_t ftl_get_free_block(void)

// free page
uint32_t ftl_get_free_page_reserved(void);
uint32_t ftl_get_free_page(void);
uint32_t ftl_get_free_map_page(void);

// block bit map
void ftl_set_free_blk_bitmap(uint32_t blk_num);
uint32_t ftl_get_free_blk_bitmap(uint32_t blk_num);

// TODO
// int ftl_garbage_collection(...);
// void ftl_flush(void);
// bad block management
// format, save, load

#endif
