#include "ftl_cache.h"
#include "ftl.h"
#include "ftl_pm.h"
#ifndef VSSIM_PM
#include "flash.h"
#endif

extern region_node *region_table;

uint32_t *cache_map;				// cache table
cache_idx_entry *cache_idx_table;	// Cache info

uint32_t cache_wp;					// cache write pointer
static uint32_t clock_hand;			// clock

uint32_t cache_open(void *mem)
{
	uint32_t s_cache_map = sizeof(unsigned char) * CACHE_MAP_SIZE;

	cache_map = (uint32_t *)mem;
	cache_idx_table = (cache_idx_entry *)(mem + s_cache_map);
	
	if (!cache_map || !cache_idx_table)
	{
#ifdef VSSIM_PM
		printf("Error : Memory Allocation failed1\n");
#else
		printk("Error : Memory Allocation failed1\n");
#endif
		return -1;
	}

	cache_wp = 0;
	clock_hand = 0;

	return 0;
}

uint32_t select_clock(void)
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
uint32_t evict_cache_line(uint32_t cur_cache_num)
#else
uint32_t evict_cache_line(uint32_t cur_cache_num, void *callback)
#endif
{
	region_node *evict_rg_node;
	void *cache = NULL;

	uint32_t evict_rg_num;
	uint32_t t_cache_wp = select_clock();			// cache num
	uint32_t new_free_page = get_free_map_page();	// new free physical page 

	if (!new_free_page)
	{
#ifdef VSSIM_PM
		printf("[%s] Error : new page allocation failed.\n", __FUNCTION__);
#else
		printk("[%s] Error : new page allocation failed.\n", __FUNCTION__);
#endif
		return -1;
	}

	// 선택된 cache가 사용중일 경우
	if(t_cache_wp == cur_cache_num)
	{
		cache_idx_table[t_cache_wp].clock_bit = 1;
		t_cache_wp = select_clock();
	}

	// evict page map
	if (cache_idx_table[t_cache_wp].ref_type == PAGE_MAP)
	{
		uint32_t evict_num;
		uint32_t evict_r_num;
		
		evict_num = cache_idx_table[t_cache_wp].root_num;
		evict_rg_num = evict_num / ROOT_NODE_SIZE;
		evict_r_num = GET_ROOT_NUM_PER_REGION(evict_num);

		evict_rg_node = (region_node *)region_table + evict_rg_num;
		cache = (void *)evict_rg_node->ptr_cache_p[evict_r_num];

		if (cache_idx_table[t_cache_wp].update_bit)
		{
			ASSERT(cache, !=, NULL);

			evict_rg_node->page_ppn[evict_r_num] = new_free_page;
			evict_rg_node->ptr_cache_p[evict_r_num] = NULL;
		
			// flush the victim map page
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
		evict_rg_num = cache_idx_table[t_cache_wp].region_num;
		
		evict_rg_node = (region_node *)region_table + evict_rg_num;
		cache = (void *)evict_rg_node->ptr_cache_e;

		if (cache_idx_table[t_cache_wp].update_bit)
		{
			ASSERT(cache, !=, NULL);

			evict_rg_node->extents_ppn = new_free_page;
			evict_rg_node->ptr_cache_e = NULL;
			
			// flush the victim map page
#ifdef VSSIM_PM
			ftl_write_map(new_free_page, cache);
#else
			ftl_write_map(new_free_page, cache, callback);
#endif
			cache_idx_table[t_cache_wp].update_bit = 0;
		}
	}

	memset (cache, 0x00, PHY_PAGE_SIZE);

	// write region table into flash
#ifdef VSSIM_PM
	update_region(evict_rg_num);
#else
	update_region(evict_rg_num, callback);
#endif

	return t_cache_wp;
}

#ifdef VSSIM_PM
pm_entry *init_pagemap(uint32_t rg_num, uint32_t r_num, extents_node *e_node)
#else
pm_entry *init_pagemap(uint32_t rg_num, uint32_t r_num, extents_node *e_node, void*callback)
#endif
{
	region_node *rg_node = (region_node *)region_table + rg_num;
	pm_entry *pagemap;

	uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);
	uint32_t t_cache_wp;

	// cache selection : cache가 꽉 안 찼을 경우
	if (cache_wp < CACHE_IDX_SIZE)
	{
		t_cache_wp = select_clock();
		cache_wp++;
	}
	// cache selection : cache가 꽉 찬 경우
	else
	{
#ifdef VSSIM_PM
		t_cache_wp = evict_cache_line(avoid_clock_selection(e_node));
#else
		t_cache_wp = evict_cache_line(avoid_clock_selection(e_node), callback);
#endif
	}

	// cache table 내의 page map 영역을 초기화
	pagemap = (pm_entry *)(cache_map + (t_cache_wp * MAP_ENTRIES_PER_PAGE));
	memset(pagemap, 0xFF, PHY_PAGE_SIZE);
	
	// update cache info
	cache_idx_table[t_cache_wp].root_num = r_num;
	cache_idx_table[t_cache_wp].ref_type = PAGE_MAP;
	cache_idx_table[t_cache_wp].clock_bit = 1;

	rg_node->ptr_cache_p[r_offset] = pagemap;
	rg_node->page_ppn[r_offset] = -1;

	// write region table into flash
#ifdef VSSIM_PM
	update_region(rg_num);
#else
	update_region(rg_num, callback);
#endif

	return pagemap;
}

uint32_t avoid_clock_selection(extents_node *e_node)
{
//	uint32_t cache_num = ((uint32_t *)e_node - cache_map) / MAP_ENTRIES_PER_PAGE;
	uint32_t cache_num = GET_CACHE_NUM(e_node);

	// victim selection을 피하기 위해 clock bit 체크
	cache_idx_table[cache_num].clock_bit = 1;

	return cache_num;
}

#ifdef VSSIM_PM
void update_physical_address(uint32_t lba, uint32_t sectors, uint32_t new_ppn)
#else
void update_physical_address(uint32_t lba, uint32_t sectors, uint32_t new_ppn, void *callback)
#endif
{
	uint32_t lpn = lba / SECTORS_PER_PAGE;
	uint32_t size = (lba+sectors-1)/SECTORS_PER_PAGE - lpn + 1;

	uint32_t rg_num = GET_REGION_NUM(lpn, NUM_PAGE_PER_REGION);
	uint32_t r_num = GET_ROOT_NUM(lpn, NUM_PAGE_PER_ROOT);
	uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);
	uint32_t page_offset_per_root = GET_PAGE_NUM_PER_ROOT(lpn, NUM_PAGE_PER_ROOT);

	region_node *rg_node = (region_node *)region_table + rg_num;
	uint32_t flag = rg_node->map_type;

	uint32_t cache_num;
	uint32_t t_cache_wp;

	ASSERT(rg_node, !=, NULL);

	// pagemap
	if (GET_FLAG_BIT(flag, r_offset))
	{
		pm_entry *pagemap = rg_node->ptr_cache_p[r_offset];

		// 요청 page가 cache 내에 있을 경우
		if (pagemap != NULL)
		{
			// update region, pagemap in cache
			update_physical_address_in_pagemap(pagemap, page_offset_per_root, size, new_ppn);

//			cache_num = ((uint32_t *)pagemap - cache_map) / MAP_ENTRIES_PER_PAGE;
			cache_num = GET_CACHE_NUM(pagemap);
			cache_idx_table[cache_num].clock_bit = 1;
			cache_idx_table[cache_num].update_bit = 1;
#ifdef DEBUG_FTL_
#ifdef VSSIM_PM
			printf("[%s] in cache : pagemap(%d) cache_num(%d)\n", __FUNCTION__, r_offset, cache_num);
#else
			printk("[%s] in cache : pagemap(%d) cache_num(%d)\n", __FUNCTION__, r_offset, cache_num);
#endif
#endif
		}
		// 요청 page가 cache 내에 없을 경우
		else
		{
			uint32_t page_ppn = rg_node->page_ppn[r_offset];
			
			ASSERT(rg_node->page_ppn[r_offset], !=, -1);

			// cache selection : cache is not full
			if (cache_wp < CACHE_IDX_SIZE)
			{
				t_cache_wp = select_clock();
				cache_wp++;
			}
			// cache selection : cache is full
			else
			{
#ifdef VSSIM_PM
				t_cache_wp = evict_cache_line(-1);
#else
				t_cache_wp = evict_cache_line(-1, callback);
#endif
			}

			// extents talbe's first node in cache
			pagemap = (pm_entry *)(cache_map + (t_cache_wp * MAP_ENTRIES_PER_PAGE));
			ASSERT(pagemap, !=, NULL);
				
#ifdef DEBUG_FTL_
#ifdef VSSIM_PM
			printf("[%s] not in cache : pagemap(%d) ppn(%d) -> cache_num(%d)\n", __FUNCTION__, r_offset, page_ppn, t_cache_wp);
#else
			printk("[%s] not in cache : pagemap(%d) ppn(%d) -> cache_num(%d)\n", __FUNCTION__, r_offset, page_ppn, t_cache_wp);
#endif
#endif
			// copy from flash to cache
			ftl_read_map(page_ppn, (void *)pagemap);

			// update region, pagemap in cache
			update_physical_address_in_pagemap(pagemap, page_offset_per_root, size, new_ppn);

			// update cache info
			cache_idx_table[t_cache_wp].root_num = r_num;
			cache_idx_table[t_cache_wp].ref_type = PAGE_MAP;
			cache_idx_table[t_cache_wp].clock_bit = 1;
			cache_idx_table[t_cache_wp].update_bit = 1;

			rg_node->ptr_cache_p[r_offset] = pagemap;

			// write region table into flash
#ifdef VSSIM_PM
			update_region(rg_num);
#else
			update_region(rg_num, callback);
#endif
		}
	}
	// extents
	else
	{
		extents_node *e_node = rg_node->ptr_cache_e;

		// 요청 page가 cache 내에 있을 경우
		if (e_node != NULL)
		{
#ifdef VSSIM_PM
			update_physical_address_in_extents(lba, sectors, new_ppn, e_node);
#else
			update_physical_address_in_extents(lba, sectors, new_ppn, e_node, callback);
#endif
			//cache_num = ((uint32_t *)e_node - cache_map) / MAP_ENTRIES_PER_PAGE;
			cache_num = GET_CACHE_NUM(e_node);
			cache_idx_table[cache_num].clock_bit = 1;
			cache_idx_table[cache_num].update_bit = 1;
#ifdef DEBUG_FTL_
#ifdef VSSIM_PM
			printf("[%s] in cache : extents(%d) cache_num(%d)\n", __FUNCTION__, rg_num, cache_num);
#else
			printk("[%s] in cache : extents(%d) cache_num(%d)\n", __FUNCTION__, rg_num, cache_num);
#endif
#endif
		}
		// 요청 page가 cache 내에 없을 경우
		else
		{
			int init_flag = 0;

			if (rg_node->extents_ppn == -1)
				init_flag = 1;

			// cache selection : cache is not full
			if (cache_wp < CACHE_IDX_SIZE)
			{
				t_cache_wp = select_clock();
				cache_wp++;
			}
			// cache selection : cache is full
			else
			{
#ifdef VSSIM_PM
				t_cache_wp = evict_cache_line(-1);
#else
				t_cache_wp = evict_cache_line(-1, callback);
#endif
			}

			// extents talbe's first node in cache
			e_node = (extents_node *)(cache_map + (t_cache_wp * MAP_ENTRIES_PER_PAGE));

			// extents table 존재하지 않을 시 초기화
			if (init_flag)
			{
				init_extents_table(e_node);
			}
			// copy from flash to cache
			else
			{
				uint32_t extents_ppn = rg_node->extents_ppn;
				
				ASSERT (rg_node->extents_ppn, !=, -1);
				
				ftl_read_map(extents_ppn, (void *)e_node);
				init_extents_table_link(e_node);
			}
			
			ASSERT(e_node, !=, NULL);

#ifdef VSSIM_PM
			update_physical_address_in_extents(lba, sectors, new_ppn, e_node);
#else
			update_physical_address_in_extents(lba, sectors, new_ppn, e_node, callback);
#endif

			// update cache info
			cache_idx_table[t_cache_wp].region_num = rg_num;
			cache_idx_table[t_cache_wp].ref_type = EXTENTS_MAP;
			cache_idx_table[t_cache_wp].clock_bit = 1;
			cache_idx_table[t_cache_wp].update_bit = 1;

			rg_node->ptr_cache_e = e_node;

			// write region_table into flash
#ifdef VSSIM_PM
			update_region(rg_num);
#else
			update_region(rg_num, callback);
#endif
		}
	}
}

#ifdef VSSIM_PM
void *get_physical_address(uint32_t lpn, uint32_t *r_offset, uint32_t *i_offset, uint32_t *l_offset)
#else
void *get_physical_address(uint32_t lpn, uint32_t *r_offset, uint32_t *i_offset, uint32_t *l_offset, void *callback)
#endif
{
	uint32_t rg_num = GET_REGION_NUM(lpn, NUM_PAGE_PER_REGION);
	uint32_t r_num = GET_ROOT_NUM(lpn, NUM_PAGE_PER_ROOT);
	uint32_t r_offset_per_region = GET_ROOT_NUM_PER_REGION(r_num);		// per region
	uint32_t page_offset_per_root = GET_PAGE_NUM_PER_ROOT(lpn, NUM_PAGE_PER_ROOT);

	region_node *rg_node = (region_node *)region_table + rg_num;
	uint32_t flag = rg_node->map_type;
	
	uint32_t cache_num;
	uint32_t t_cache_wp;

	*r_offset = r_num;

	// page map
	if (GET_FLAG_BIT(flag, r_offset_per_region))
	{
		pm_entry *pagemap = rg_node->ptr_cache_p[r_offset_per_region];

		// 요청 page가 cache 내에 있을 경우
		if (pagemap != NULL)
		{
//			cache_num = ((uint32_t *)pagemap - cache_map) / MAP_ENTRIES_PER_PAGE;
			cache_num = GET_CACHE_NUM(pagemap);
			cache_idx_table[cache_num].clock_bit = 1;
		}
		// 요청 page가 cache 내에 없을 경우
		else
		{
			if (rg_node->page_ppn[r_offset_per_region] != -1)
			{
				uint32_t page_ppn = rg_node->page_ppn[r_offset_per_region];

				// cache selection : cache is not full
				if (cache_wp < CACHE_IDX_SIZE)
				{
					t_cache_wp = select_clock();
					cache_wp++;
				}
				// cache selection : cache is full
				else
				{
#ifdef VSSIM_PM
					t_cache_wp = evict_cache_line(-1);
#else
					t_cache_wp = evict_cache_line(-1, callback);
#endif
				}

				// extents talbe's first node in cache
				pagemap = (pm_entry *)(cache_map + (t_cache_wp * MAP_ENTRIES_PER_PAGE));
				ASSERT(pagemap, !=, NULL);
					
				// copy from flash to cache
				ftl_read_map(page_ppn, (void *)pagemap);

				// update cache info
				cache_idx_table[t_cache_wp].root_num = r_num;
				cache_idx_table[t_cache_wp].ref_type = PAGE_MAP;
				cache_idx_table[t_cache_wp].clock_bit = 1;

				rg_node->ptr_cache_p[r_offset_per_region] = pagemap;
				
				// write region table into flash
#ifdef VSSIM_PM
				update_region(rg_num);
#else
				update_region(rg_num, callback);
#endif
			}
			else
				return NULL;
		}

		// page mapping
		ASSERT(pagemap, !=, NULL);
		return (void *)(read_pagemap(pagemap, page_offset_per_root));
	}
	// extents map
	else
	{
		extents_node *e_node = rg_node->ptr_cache_e;
		root_node *r_node;
		leaf_node *i_node;
		
		// 요청 page가 cache 내에 있을 경우
		if (e_node != NULL)
		{
//			cache_num = ((uint32_t *)e_node - cache_map) / MAP_ENTRIES_PER_PAGE;
			cache_num = GET_CACHE_NUM(e_node);
			cache_idx_table[cache_num].clock_bit = 1;
		}
		// 요청 page가 cache 내에 없을 경우
		else
		{
			if (rg_node->extents_ppn != -1)
			{
				uint32_t extents_ppn = rg_node->extents_ppn;

				// cache selction : cache is not full
				if (cache_wp < CACHE_IDX_SIZE)
				{
					t_cache_wp = select_clock();
					cache_wp++;
				}
				// cache selection : cache is full
				else
				{
#ifdef VSSIM_PM
					t_cache_wp = evict_cache_line(-1);
#else
					t_cache_wp = evict_cache_line(-1, callback);
#endif
				}
				
				// extents talbe's first node in cache
				e_node = (extents_node *)(cache_map + (t_cache_wp * MAP_ENTRIES_PER_PAGE));
				ASSERT(e_node, !=, NULL);

				// copy from flash to cache
				ftl_read_map(extents_ppn, (void *)e_node);
				init_extents_table_link(e_node);

				// update cache info
				cache_idx_table[t_cache_wp].region_num = rg_num;
				cache_idx_table[t_cache_wp].ref_type = EXTENTS_MAP;
				cache_idx_table[t_cache_wp].clock_bit = 1;
					
				rg_node->ptr_cache_e = e_node;

				// write region table into flash
#ifdef VSSIM_PM
				update_region(rg_num);
#else
				update_region(rg_num, callback);
#endif
			}
			else 
				return NULL;
		}

		// extents table
		r_node = (root_node *)e_node->r_node + r_offset_per_region;
		i_node = r_node->index_node;
		
		ASSERT(r_node, !=, NULL);
		ASSERT(i_node, !=, NULL);

		return (void *)(read_extents(i_node, page_offset_per_root, i_offset, l_offset));
	}
}
