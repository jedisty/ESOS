#ifndef _CACHE_H
#define _CACHE_H

#include "common.h"
#include "pm.h"

/* 
 * macro 
 */
// cache
#ifdef VSSIM_PM
	#define CACHE_MAP_SIZE			(PAGE_SIZE * CACHE_IDX_SIZE)	// 1M
#else
	#define CACHE_MAP_SIZE			(PHY_PAGE_SIZE * CACHE_IDX_SIZE)	// 1M
#endif

#define CACHE_IDX_SIZE			128			// 128 * 8k = 1M

// Map Type
#define EXTENTS_MAP 0
#define PAGE_MAP	1

// get cache line number
#define GET_CACHE_NUM(map)	(((uint32_t *)map - cache_map) / MAP_ENTRIES_PER_PAGE)

/* 
 * data structure 
 */
typedef struct cache_idx_entry_ {
	union {
		uint32_t root_num		: 29;	// page mapping
		uint32_t region_num 	: 29;	// extents mapping
	};
	uint32_t clock_bit	: 1;	// clock reference bit
	uint32_t ref_type	: 1;	// extents map(0)	page map(1)
	uint32_t update_bit	: 1;
} cache_idx_entry;

/* 
 * user defined functions 
 */
uint32_t cache_init(void *mem);

uint32_t cache_select_victim(void);
#ifdef VSSIM_PM
uint32_t cache_evict_page(void);
#else
uint32_t cache_evict_page(void *callback);
#endif
void cache_avoid_victim_selection(extents_node *e_node);

void cache_update_info(uint32_t cache_num, uint32_t is_update);
void cache_update_extentsmap_info(uint32_t t_cache_wp, uint32_t rg_num, uint32_t is_update);
void cache_update_pagemap_info(uint32_t t_cache_wp, uint32_t r_num, uint32_t is_update);

#endif
