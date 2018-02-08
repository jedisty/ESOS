#include "common.h"

/**************************************/
/* data structure and global variable */
/**************************************/
extern region_node *region_table;
uint32_t *cache_map;				// cache table
cache_idx_entry *cache_idx_table;	// Cache info

uint32_t cache_wp;					// cache write pointer
static uint32_t clock_hand;			// clock

/*************************/
/* user defined function */
/*************************/
uint32_t cache_init(void *mem)
{
	uint32_t s_cache_map = sizeof(unsigned char) * CACHE_MAP_SIZE;

	cache_map = (uint32_t *)mem;
	cache_idx_table = (cache_idx_entry *)(mem + s_cache_map);
	
	if (!cache_map || !cache_idx_table)
	{
#ifdef VSSIM_PM
		printf("[%s] Memory Allocation failed\n", __FUNCTION__);
#else
		printk("[%s] Memory Allocation failed\n", __FUNCTION__);
#endif
		return -1;
	}

	cache_wp = 0;
	clock_hand = 0;

	return 0;
}

uint32_t cache_select_victim(void)
{
	uint32_t idx;
	uint32_t evict_cache_num = -1;

	while(1)
	{
		idx = clock_hand;

		if (idx == CACHE_IDX_SIZE)
			idx = 0;

		if (++clock_hand == CACHE_IDX_SIZE)
			clock_hand = 0;

		if (cache_idx_table[idx].clock_bit)
		{
			cache_idx_table[idx].clock_bit = 0;
		}
		else
		{
			cache_idx_table[idx].clock_bit = 1;
			evict_cache_num = idx;

			return evict_cache_num;
		}
	}
}

#ifdef VSSIM_PM
uint32_t cache_evict_page()
#else
uint32_t cache_evict_page(void *callback)
#endif
{
	region_node *evict_rg_node;
	void *cache = NULL;

	uint32_t evict_rg_num;
	uint32_t t_cache_wp = cache_select_victim();			// cache num
	uint32_t new_free_page = ftl_get_free_map_page();	// new free physical page 

	if (!new_free_page)
	{
#ifdef VSSIM_PM
		printf("[%s] Error : new page allocation failed.\n", __FUNCTION__);
#else
		printk("[%s] Error : new page allocation failed.\n", __FUNCTION__);
#endif
		return -1;
	}

	// evict page map
	if (cache_idx_table[t_cache_wp].ref_type == PAGE_MAP)
	{
		uint32_t evict_num;
		uint32_t evict_r_num;
		
		evict_num = cache_idx_table[t_cache_wp].root_num;		// root number
		evict_rg_num = evict_num / ROOT_NODE_SIZE;				// region number
		evict_r_num = GET_ROOT_NUM_PER_REGION(evict_num);		// root offset

		evict_rg_node = (region_node *)region_table + evict_rg_num;
		cache = (void *)evict_rg_node->ptr_cache_p[evict_r_num];

		// update되었을 경우에 write
		if (cache_idx_table[t_cache_wp].update_bit)
		{
			ASSERT(cache, !=, NULL);

			evict_rg_node->page_ppn[evict_r_num] = new_free_page;
			evict_rg_node->ptr_cache_p[evict_r_num] = NULL;
		
			// flush a victim map page
#ifdef VSSIM_PM
			ftl_write_map(new_free_page, cache);
#else
			ftl_write_map(new_free_page, cache, callback);
#endif
			cache_idx_table[t_cache_wp].update_bit = 0;
		}
	}
	// evict extents map
	else
	{
		evict_rg_num = cache_idx_table[t_cache_wp].region_num;	// region number
		
		evict_rg_node = (region_node *)region_table + evict_rg_num;
		cache = (void *)evict_rg_node->ptr_cache_e;

		// update되었을 경우에 write
		if (cache_idx_table[t_cache_wp].update_bit)
		{
			ASSERT(cache, !=, NULL);

			evict_rg_node->extents_ppn = new_free_page;
			evict_rg_node->ptr_cache_e = NULL;
			
			// flush a victim map page
#ifdef VSSIM_PM
			ftl_write_map(new_free_page, cache);
#else
			ftl_write_map(new_free_page, cache, callback);
#endif
			cache_idx_table[t_cache_wp].update_bit = 0;
		}
	}

#ifdef VSSIM_PM
	memset (cache, 0x00, PAGE_SIZE);
#else
	memset (cache, 0x00, PHY_PAGE_SIZE);
#endif

	// write region table into flash
#ifdef VSSIM_PM
	update_region(evict_rg_num);
#else
	update_region(evict_rg_num, callback);
#endif
	return t_cache_wp;
}

void cache_avoid_victim_selection(extents_node *e_node)
{
	cache_idx_table[GET_CACHE_NUM(e_node)].clock_bit = 1;
}

void cache_update_info(uint32_t cache_num, uint32_t is_update)
{
	cache_idx_table[cache_num].clock_bit = 1;

	// update시에만 체크
	if (is_update)
		cache_idx_table[cache_num].update_bit = 1;
}

void cache_update_extentsmap_info(uint32_t t_cache_wp, uint32_t rg_num, uint32_t is_update)
{
	cache_idx_table[t_cache_wp].region_num = rg_num;
	cache_idx_table[t_cache_wp].ref_type = EXTENTS_MAP;
	cache_idx_table[t_cache_wp].clock_bit = 1;
	
	// update시에만 체크
	if (is_update)
		cache_idx_table[t_cache_wp].update_bit = 1;
}

void cache_update_pagemap_info(uint32_t t_cache_wp, uint32_t r_num, uint32_t is_update)
{
	cache_idx_table[t_cache_wp].root_num = r_num;
	cache_idx_table[t_cache_wp].ref_type = PAGE_MAP;
	cache_idx_table[t_cache_wp].clock_bit = 1;

	// update시에만 체크
	if (is_update)
		cache_idx_table[t_cache_wp].update_bit = 1;
}
