#ifndef _CACHE_H
#define _CACHE_H

#include "common.h"
#include "pm.h"

#define CACHE_MAP_SIZE			(PHY_PAGE_SIZE * CACHE_IDX_SIZE)	// 32k
#define CACHE_IDX_SIZE			(NR_PHY_PAGES / (8 * 1024))			// total size / 8k

// cache_idx_entry->ref_type
#define EXTENTS_MAP 0
#define PAGE_MAP	1

#define GET_CACHE_NUM(map)	(((uint32_t *)map - cache_map) / MAP_ENTRIES_PER_PAGE)

typedef struct cache_idx_entry_ {
	union {
		uint32_t root_num		: 29;	// page mapping
		uint32_t region_num 	: 29;	// extents mapping
	};
	uint32_t clock_bit	: 1;	// clock reference bit
	uint32_t ref_type	: 1;	// extents map(0)	page map(1)
	uint32_t update_bit	: 1;
} cache_idx_entry;

uint32_t cache_open(void *mem);

uint32_t select_clock(void);
uint32_t avoid_clock_selection(extents_node *e_node);
pm_entry *init_pagemap(uint32_t rg_num, uint32_t r_num, extents_node *e_node, void *callback);

uint32_t evict_cache_line(uint32_t cur_cache_num, void *callback);
void update_physical_address(uint32_t lba, uint32_t sectors, uint32_t new_ppn, void *callback);
void  *get_physical_address(uint32_t lpn, uint32_t *r_offset, uint32_t *i_offset, uint32_t *l_offset, void *callback);


#endif
