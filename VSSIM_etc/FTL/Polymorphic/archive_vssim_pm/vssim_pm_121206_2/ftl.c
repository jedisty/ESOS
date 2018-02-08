#include "common.h"

#ifdef VSSIM_PM
#include "ssd_io_manager.h"
#endif

/**************************************/
/* data structure and global variable */
/**************************************/
void *mem;					// total memory
extern region_node *region_table;		// (pm.c)
struct ftl_metadata_ *ftl_metadata;		// write pointer 구조체
uint32_t *free_blk_bitmap;			// free block bitmap
uint32_t write_add_count;			// 추가 발생하는 write count

uint32_t remain_js;
/*************************/
/* user defined function */
/*************************/

#ifdef VSSIM_PM
int g_init = 0;
#endif

int ftl_open(void)
{
#ifdef VSSIM_PM
	if(g_init == 0){
		printf("[ftl_open] Start!\n");
		INIT_SSD_CONFIG();
#endif
		// set data structure's size
		uint32_t s_region_table = sizeof(struct region_node_) * REGION_NODE_SIZE;
		uint32_t s_region_idx_table = sizeof(uint32_t *) * REGION_IDX_SIZE;
		uint32_t s_cache_map = sizeof(unsigned char) * CACHE_MAP_SIZE;
		uint32_t s_cache_idx_table = sizeof(cache_idx_entry *) * CACHE_IDX_SIZE;
		uint32_t s_ftl_metadata = sizeof(struct ftl_metadata_);
#ifdef VSSIM_PM
		uint32_t s_free_blk_bitmap = (BLOCK_NB / 8) + 1;
#else
		uint32_t s_free_blk_bitmap = (BLOCKS_PER_BANK / 8) + 1;
#endif	
		// total allocatied memory size
		uint32_t s_mem = s_region_table + s_region_idx_table + s_cache_map + s_cache_idx_table + s_ftl_metadata + s_free_blk_bitmap;

		// memory allocation
#ifdef VSSIM_PM
		mem = (void *)malloc(s_mem);
#else
		mem = (void *)vmalloc(s_mem);
#endif
		if (!mem)
		{
			DEBUG_ERR("Error: Memory allocation Failed size(%d) %p\n", s_mem, mem);
			return -1;
		}
		memset (mem, 0x00, s_mem);
		// initialize data structure's initial address & value associated with ftl
		pm_init(mem);
		cache_init(mem + (s_region_table + s_region_idx_table));
		ftl_init(mem + (s_region_table + s_region_idx_table + s_cache_map + s_cache_idx_table));

		/* set initial value */
		write_add_count = 0;

#ifdef VSSIM_PM
		INIT_PERF_CHECKER();
		INIT_VSSIM_PM_MAPPING();
	
		g_init = 1;
		printf("[ftl_open] Success!\n");
	}
#endif
	return 0;
}

int ftl_init(void *mem)
{
	uint32_t s_ftl_metadata = sizeof(struct ftl_metadata_);

	ftl_metadata = (struct ftl_metadata_ *)mem;
	free_blk_bitmap = (uint32_t *)(mem + s_ftl_metadata);
	if (!ftl_metadata || !free_blk_bitmap)
	{
		DEBUG_ERR("[%s] Memory allocation Failed \n", __FUNCTION__);
		ftl_close();

		return -1;
	}
	
	// initialize block write pointer
	ftl_metadata->reserved_block_ptr = 0;
	ftl_metadata->write_block_ptr = NR_RESERVED_PHY_BLOCK_SIZE;
	ftl_metadata->write_map_block_ptr = NR_RESERVED_PHY_BLOCK_SIZE + 1;
	ftl_metadata->block_ptr = NR_RESERVED_PHY_BLOCK_SIZE + 2;		// dd
	
	// initialize block meta data
	set_free_blk_bitmap(ftl_metadata->reserved_block_ptr);
	set_free_blk_bitmap(ftl_metadata->write_block_ptr);
	set_free_blk_bitmap(ftl_metadata->write_map_block_ptr);
	
	// initialize page write pointer
	ftl_metadata->reserved_write_ptr = 1;
#ifdef VSSIM_PM
	ftl_metadata->write_ptr = ftl_metadata->write_block_ptr * PAGE_NB * WAY_NB * FLASH_NB;
	ftl_metadata->write_map_ptr = ftl_metadata->write_map_block_ptr * PAGE_NB * WAY_NB * FLASH_NB;
#else
	ftl_metadata->write_ptr = ftl_metadata->write_block_ptr * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
	ftl_metadata->write_map_ptr = ftl_metadata->write_map_block_ptr * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
#endif
	return 0;
}

void ftl_close(void)
{
	if (mem){
#ifdef VSSIM_PM
		free(mem);
#else
		vfree(mem);
#endif
	}
}

uint32_t ftl_calc_bank(uint32_t ppn)
{
#ifdef VSSIM_PM
	return ppn % (WAY_NB * FLASH_NB);
#else
	return ppn % (NR_PHY_WAYS * NR_PHY_BANKS);
#endif
}

uint32_t ftl_calc_block(uint32_t ppn)
{
#ifdef VSSIM_PM
	return (ppn / (WAY_NB * FLASH_NB)) / PAGE_NB;
#else
	return (ppn / (NR_PHY_WAYS * NR_PHY_BANKS)) / PAGES_PER_BLOCK;
#endif
}

uint32_t ftl_calc_page(uint32_t ppn)
{
#ifdef VSSIM_PM
	return (ppn / (WAY_NB * FLASH_NB)) % PAGE_NB;
#else
	return (ppn / (NR_PHY_WAYS * NR_PHY_BANKS)) % PAGES_PER_BLOCK;
#endif
}

static uint32_t get_free_block(void)
{
	uint32_t res = ftl_metadata->block_ptr;
#ifdef VSSIM_PM
	int i = BLOCK_NB - NR_RESERVED_PHY_BLOCK_SIZE;
#else
	int i = BLOCKS_PER_BANK - NR_RESERVED_PHY_BLOCK_SIZE;
#endif

	while (i--)
	{
		res++;

#ifdef VSSIM_PM
		if (res >= BLOCK_NB)
#else
		if (res >= BLOCKS_PER_BANK)
#endif
		{
			res = NR_RESERVED_PHY_BLOCK_SIZE;
		}

		if (ftl_metadata->write_map_block_ptr != res && ftl_metadata->write_block_ptr != res)
//					&& !get_free_blk_bitmap(res))
		{
			ftl_metadata->block_ptr = res;
			set_free_blk_bitmap(res);
			
			return res;
		}
	}

	return -1;
}

static uint32_t get_free_block_in_reserved(void)
{
	uint32_t res = ftl_metadata->reserved_block_ptr;
	int i = NR_RESERVED_PHY_BLOCK_SIZE;

	while (i--)
	{
		res++;
		
		if (res >= NR_RESERVED_PHY_BLOCK_SIZE)
			res = 0;

//		if (!get_free_blk_bitmap(res))
		{
			ftl_metadata->reserved_block_ptr = res;
			set_free_blk_bitmap(res);

			return res;
		}
	}

	return -1;
}

uint32_t get_free_page_in_reserved(void)
{
	uint32_t res = ftl_metadata->reserved_write_ptr;
	uint32_t blk = ftl_calc_block(ftl_metadata->reserved_write_ptr);
	
	if (blk != ftl_metadata->reserved_block_ptr || res > NR_RESERVED_PHY_PAGES)
	{
		blk = get_free_block_in_reserved();
		if (blk != -1)
		{
			ftl_metadata->reserved_block_ptr = blk;
	
			if (blk == 0){
				ftl_metadata->reserved_write_ptr = 1;
			}
			else{
#ifdef VSSIM_PM
				ftl_metadata->reserved_write_ptr = blk * PAGE_NB * WAY_NB * FLASH_NB;
#else
				ftl_metadata->reserved_write_ptr = blk * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
#endif
			}	
			res = ftl_metadata->reserved_write_ptr++;
		}
		else
		{
			DEBUG_ERR("[%s] No More Free Block in Reserved Region! (%d)\n", __FUNCTION__, blk);

			return 0;
		}
	}
	else
		ftl_metadata->reserved_write_ptr++;
	
	return res;
}

uint32_t get_free_map_page(void)
{
	uint32_t res = ftl_metadata->write_map_ptr;
	uint32_t blk = ftl_calc_block(ftl_metadata->write_map_ptr);

	if (blk != ftl_metadata->write_map_block_ptr || res > NR_PHY_PAGES) 
	{
		// allocate new block
		blk = get_free_block();
		if (blk != -1)
		{
			ftl_metadata->write_map_block_ptr = blk;
#ifdef VSSIM_PM
			ftl_metadata->write_map_ptr = blk * PAGE_NB * WAY_NB * FLASH_NB;
#else
			ftl_metadata->write_map_ptr = blk * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
#endif
			res = ftl_metadata->write_map_ptr++;
		}
		else
		{
			DEBUG_ERR("[%s] No More Free Block in Flash! (%d)\n", __FUNCTION__, blk);

			return 0;
		}
	}
	else
		ftl_metadata->write_map_ptr++;

	return res;
}

uint32_t get_free_page(void)
{
	uint32_t res = ftl_metadata->write_ptr;
	uint32_t blk = ftl_calc_block(ftl_metadata->write_ptr);

	if (blk != ftl_metadata->write_block_ptr || res > NR_PHY_PAGES)
	{		
		// allocate new block
		blk = get_free_block();
		if (blk != -1)
		{
			ftl_metadata->write_block_ptr = blk;
#ifdef VSSIM_PM
			ftl_metadata->write_ptr = blk * PAGE_NB * WAY_NB * FLASH_NB;
#else
			ftl_metadata->write_ptr = blk * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
#endif

			res = ftl_metadata->write_ptr++;
		}
		else
		{
			DEBUG_ERR("[%s] No More Free Block in Flash! (%d)\n", __FUNCTION__, blk);

			return 0;
		}
	}
	else
		ftl_metadata->write_ptr++;

	return res;
}

#ifdef VSSIM_PM
void ftl_read(uint32_t lba, uint32_t sectors)
#else
int ftl_read(uint32_t lba, uint32_t sectors, void * buf, void* callback)
#endif
{
#ifdef VSSIM_PM
	if(lba == 0 && sectors == 1)
		return;
#endif

#ifdef FTL_DEBUG
	printf("[ftl_read] Start %u %u \n", lba, sectors);
#endif
	uint32_t left_skip = lba % SECTORS_PER_PAGE;
	uint32_t right_skip;
	uint32_t sects = 0;
	uint32_t remain;
	int count = 0;

	uint32_t ori_lba = lba;
	uint32_t lpn=lba/SECTORS_PER_PAGE;
	uint32_t t_sectors = 0;
	uint32_t ppn=0;
	
	uint32_t divided_lba = 0;
	uint32_t divided_lpn = 0;
	uint32_t loop = 1;
	
	uint32_t root_offset = 0;
	uint32_t index_offset = 0;
	uint32_t leaf_offset = 0;

	//void *res = NULL;
	void *res;

#ifdef VSSIM_PM
	int read_page_nb = 0;
#endif
#ifndef VSSIM_PM	
	DEBUG_FTL_RW("mlc: ftl_read(%10u, %6u, 0 %p)\n", lba, sectors, buf);
#endif
#ifdef VSSIM_PM
	ALLOC_IO_REQUEST(lba, sectors, READ);
#endif

	write_add_count = 0;

	// 같은 region(root node) 여부 확인
	if(IS_SAME_REGION(lba, sectors))
	{
		loop = 1;
		t_sectors = sectors;
	}
	else
	{
		divided_lba = GET_DIVIDED_REGION(lba);
		divided_lpn = divided_lba / SECTORS_PER_PAGE;
		loop = 2;
		
		t_sectors = divided_lba - ori_lba;
	}
	while (loop--)
	{
		uint32_t rg_num = GET_REGION_NUM(lpn, NUM_PAGE_PER_REGION);
		uint32_t r_num = GET_ROOT_NUM(lpn, NUM_PAGE_PER_ROOT);
		uint32_t r_offset_per_region = GET_ROOT_NUM_PER_REGION(r_num);	
		region_node *rg_node = (region_node *)region_table + rg_num;
		uint32_t flag = rg_node->map_type;

		// Page Mapping
		if(BIT_GET(flag, r_offset_per_region))
		{
			// page 단위로 처리
#ifdef VSSIM_PM
			for (remain = t_sectors; remain > 0; lba += sects, remain -= sects, left_skip = 0)
#else
			for (remain = t_sectors; remain > 0; buf += sects*512, lba += sects, remain -= sects, left_skip = 0)
#endif
			{
				lpn = lba/SECTORS_PER_PAGE;

#ifdef VSSIM_PM
				res = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset);
#else
				res = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset, callback);
#endif

				// page와 sector의 크기 차이로 인한 skip
				if (remain > SECTORS_PER_PAGE - left_skip)
					right_skip = 0;
				else
					right_skip = SECTORS_PER_PAGE - left_skip - remain;
				sects = SECTORS_PER_PAGE - left_skip - right_skip;

				// 쓰인적 없는 page를 읽는 경우 FF..FF 반환
				if(!res)
				{
#ifndef VSSIM_PM
					memset(buf, 0xFF, sects * 512);
#endif
					continue;
				}
				else
				{
					ppn = ((pm_entry *)res)->ppn;
					
					if (ppn == -1)
					{
#ifndef VSSIM_PM
						memset(buf, 0xFF, sects * 512);
#endif
						continue;
					}
				}

#ifdef VSSIM_PM
				CELL_READ(ftl_calc_bank(ppn), ftl_calc_block(ppn), ftl_calc_page(ppn), read_page_nb, READ);
				read_page_nb++;
#else
				flash_read(callback, 
						ftl_calc_bank(ppn),
						ftl_calc_block(ppn),
						ftl_calc_page(ppn), 
						buf, left_skip * 512, right_skip * 512);
#endif
				count ++;
			}
		}
		else{	// Extents Mapping

			leaf_node *leaf = NULL;
			int32_t leaf_size = 0;
			uint32_t add_size = 0;
			
			// get the leaf node
#ifdef VSSIM_PM
			res = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset);
#else
			res = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset, callback);
#endif
			// 가져온 leaf node의 ppn을 요청한 lpn에 맞게 조정
			if(res)
			{
				leaf = (leaf_node *)res;
				leaf_size = leaf->size;
				add_size = GET_PAGE_NUM_PER_ROOT(lpn, NUM_PAGE_PER_ROOT) - leaf->lpn;

				ppn =  ((leaf_node *)leaf)->ppn + add_size;
			}

			while(1)	
			{
#ifdef VSSIM_PM
				remain = t_sectors;
				while(1)
#else
				for (remain = t_sectors; remain > 0 && leaf_size > 0; buf += sects * 512, lba += sects, remain -= sects, left_skip = 0, --leaf_size)
#endif
				{
					printf("1 remain : %d, leaf_size %d\n", remain, leaf_size);

					// page와 sector의 크기 차이로 인한 skip
					if (remain > SECTORS_PER_PAGE - left_skip)
						right_skip = 0;
					else
						right_skip = SECTORS_PER_PAGE - left_skip - remain;
					sects = SECTORS_PER_PAGE - left_skip - right_skip;
		
					// 쓰인적 없는 page를 읽는 경우 FF..FF 반환
					if (!res || leaf->used == 0 || leaf->ppn == -1) {
#ifndef VSSIM_PM
						memset(buf, 0xFF, sects * 512);
//MOD
						continue;
#endif
					}

#ifdef VSSIM_PM
					CELL_READ(ftl_calc_bank(ppn), ftl_calc_block(ppn), ftl_calc_page(ppn), read_page_nb, READ);
					read_page_nb++;

#else	
					flash_read(callback, 
							ftl_calc_bank(ppn),
							ftl_calc_block(ppn),
							ftl_calc_page(ppn),
							buf, left_skip * 512, right_skip * 512);
#endif				
					ppn++;
					count++;		

#ifdef VSSIM_PM
					remain -= sects;
					--leaf_size;

//					if(remain == 0 && (leaf_size == 0 || leaf_size < 0 ))
					if(remain == 0)
					{
						break;
					}
					else{
						lba += sects;
						left_skip = 0;
					}
#endif

					printf("2 remain : %d, leaf_size %d\n", remain,leaf_size);
				}

				if (remain>0 && res) 
				{
					leaf = get_leaf(rg_num, &root_offset, &index_offset, &leaf_offset);
					leaf_size = leaf->size;
					ppn = leaf->ppn;
				}
				else
					break;
			}
		}
		// request가 두개로 나뉘어졌을 경우
		if(loop) 
		{
			lpn = divided_lpn;
			t_sectors = sectors - t_sectors;
		}
	}
#ifdef VSSIM_PM
	INCREASE_IO_REQUEST_SEQ_NB();
#endif
#ifdef FTL_DEBUG
	printf("[ftl_read] Success!\n");
#endif

#ifndef VSSIM_PM
	return (count + write_add_count);
#endif
}

#ifdef VSSIM_PM
void ftl_write(uint32_t lba, uint32_t sectors)
#else
int ftl_write(uint32_t lba, uint32_t sectors, void * buf, void* callback)
#endif
{
//	if(lba == 0 && sectors == 1)
//		return 0;

#ifdef FTL_DEBUG
	printf("[ftl_write] Start %u %u\n", lba, sectors);
#endif

	uint32_t left_skip = lba % SECTORS_PER_PAGE;
	uint32_t right_skip;
	uint32_t sects = 0;
	uint32_t remain;
	int count=0;

	uint32_t ori_lba = lba;
	uint32_t lpn = lba / SECTORS_PER_PAGE;
	uint32_t new_ppn = 0;
	uint32_t ori_ppn = 0xFFFFFFFF;
	uint32_t old_ppn = 0;
	uint32_t ppn;
	
	uint32_t first_block, second_block;
	uint32_t last_ppn = 0;
	
	uint32_t root_offset, index_offset, leaf_offset;
	uint32_t rg_num, r_num, r_offset_per_region;
	uint32_t flag;
	region_node *rg_node;
	
	write_add_count = 0;

	// TO DO : garbage collection
	
#ifdef VSSIM_PM
	int write_page_nb = 0;
#endif

#ifndef VSSIM_PM
	DEBUG_FTL_RW("mlc: ftl_write(%10u, %6u, 0 %p)\n", lba, sectors, buf);
#endif

#ifdef VSSIM_PM
	ALLOC_IO_REQUEST(lba, sectors, WRITE);
#endif

#ifdef VSSIM_PM
	for (remain = sectors; remain > 0; lba += sects, remain -= sects, left_skip = 0) 
#else
	for (remain = sectors; remain > 0; buf += sects * 512, lba += sects, remain -= sects, left_skip = 0) 
#endif
	{
		old_ppn=0;
		lpn = lba / SECTORS_PER_PAGE;
		
		// get a free page
		new_ppn = get_free_page();

		if(ori_ppn == 0xFFFFFFFF)
			ori_ppn = new_ppn;

		// page와 sector의 크기 차이로 인한 skip
		if (remain > SECTORS_PER_PAGE - left_skip)
			right_skip = 0;
		else
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		sects = SECTORS_PER_PAGE - left_skip - right_skip;

		// skip된 sectors가 있을 경우 partial write
		if (left_skip || right_skip) 
		{
			void *ptr = NULL;
			
			root_offset = 0;
			index_offset = 0;
			leaf_offset = 0;

#ifdef VSSIM_PM
			ptr = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset);
#else
			ptr = get_physical_address(lpn, &root_offset, &index_offset, &leaf_offset, callback);
#endif

			if (ptr)
			{
				rg_num = GET_REGION_NUM(lpn, NUM_PAGE_PER_REGION);
				r_num = GET_ROOT_NUM(lpn, NUM_PAGE_PER_ROOT);
				r_offset_per_region = GET_ROOT_NUM_PER_REGION(r_num);
				rg_node = (region_node *)region_table + rg_num;
				flag = rg_node->map_type;

				// Page Mapping
				if (BIT_GET(flag, r_offset_per_region))
				{
					if (((pm_entry*)ptr)->ppn != -1)
					{
						old_ppn = ((pm_entry*)ptr)->ppn;
					}
				}
				// Extents Mapping
				else
				{
					if (((leaf_node *)ptr)->used && ((leaf_node *)ptr)->ppn != -1)
					{
						old_ppn =  ((leaf_node*)ptr)->ppn + (GET_PAGE_NUM_PER_ROOT(lpn, NUM_PAGE_PER_ROOT) - ((leaf_node*)ptr)->lpn);
					}
				}
				
				if (old_ppn == -1)	
					old_ppn = 0;
			}

#ifdef VSSIM_PM
			CELL_WRITE(ftl_calc_bank(new_ppn), ftl_calc_block(new_ppn), ftl_calc_page(new_ppn), write_page_nb, WRITE);
			write_page_nb++;
#else
			flash_partial_write(callback, 
					ftl_calc_bank(new_ppn), 
					ftl_calc_block(new_ppn), 
					ftl_calc_page(new_ppn),
					buf,
					ftl_calc_bank(old_ppn),
					ftl_calc_block(old_ppn),
					ftl_calc_page(old_ppn),
					left_skip * 512, right_skip * 512);
#endif
		}
		else
		{
#ifdef VSSIM_PM
			CELL_WRITE(ftl_calc_bank(new_ppn), ftl_calc_block(new_ppn), ftl_calc_page(new_ppn), write_page_nb, WRITE);
			write_page_nb++;
#else
			flash_write(callback,
					ftl_calc_bank(new_ppn),
					ftl_calc_block(new_ppn),
					ftl_calc_page(new_ppn),
					buf);
#endif
		}

		count ++;
	}

			/* update physical addressa */
	last_ppn = new_ppn;
	first_block = ftl_calc_block(ori_ppn);
	second_block = ftl_calc_block(last_ppn);

	// 요청한 free page가 다른 allocation block에 할당될 경우
	if (first_block != second_block)
	{
#ifdef VSSIM_PM
		uint32_t divided_ppn = ((first_block+1) * PAGE_NB * WAY_NB * FLASH_NB);
#else
		uint32_t divided_ppn = ((first_block+1) * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS);	// next block's first ppn
#endif

		left_skip = ori_lba % SECTORS_PER_PAGE;

		for (remain = sectors, lba = ori_lba, ppn = ori_ppn; remain > 0; lba += sects, remain -= sects, left_skip = 0)
		{
			// first allocation block
			if (remain == sectors)
			{
				sects = ((divided_ppn - ori_ppn) * SECTORS_PER_PAGE) - left_skip;
				ppn = ori_ppn;
			}
			// second allocation block
			else
			{
				sects = remain;
#ifdef VSSIM_PM
				ppn = second_block * PAGE_NB * WAY_NB * FLASH_NB;
#else
				ppn = second_block * PAGES_PER_BLOCK * NR_PHY_WAYS * NR_PHY_BANKS;
#endif
			}

			// 같은 region(root node) 여부 확인
			if(IS_SAME_REGION(lba, sects))
			{
#ifdef VSSIM_PM
				update_physical_address(lba, sects, ppn);
#else
				update_physical_address(lba, sects, ppn, callback);
#endif
			}
			else
			{
				uint32_t divided_lba = GET_DIVIDED_REGION(lba);
				uint32_t divided_size = divided_lba - lba;
				uint32_t add_ppn = (divided_lba/SECTORS_PER_PAGE) - (lba/SECTORS_PER_PAGE);
					
				// update physical address in two 
#ifdef VSSIM_PM
				update_physical_address(lba, divided_size, ori_ppn);
			 	update_physical_address(divided_lba, sects-divided_size, ppn+add_ppn);
#else
				update_physical_address(lba, divided_size, ori_ppn, callback);
			 	update_physical_address(divided_lba, sects-divided_size, ppn+add_ppn, callback);
#endif
			}
		}
	}
	// 같은 allocation block에 할당될 경우
	else
	{
		// 같은 region(root node) 여부 확인
		if(IS_SAME_REGION(ori_lba, sectors))
		{
#ifdef VSSIM_PM
			update_physical_address(ori_lba, sectors, ori_ppn);
#else
			update_physical_address(ori_lba, sectors, ori_ppn, callback);
#endif
		}
		else
		{
			uint32_t divided_lba = GET_DIVIDED_REGION(ori_lba);
			uint32_t divided_size = divided_lba - ori_lba;
			uint32_t add_ppn = divided_lba/SECTORS_PER_PAGE - ori_lba/SECTORS_PER_PAGE;
				
			// update physical address in two
#ifdef VSSIM_PM
			printf("Write Semi End 1\n");
			printf("ori_lba %d dvd_size %d ori_ppn %d \n", ori_lba, divided_size, ori_ppn);
			update_physical_address(ori_lba, divided_size, ori_ppn);
			printf("Write Semi End 2\n");
			update_physical_address(divided_lba, sectors-divided_size, ori_ppn+add_ppn);
#else
			update_physical_address(ori_lba, divided_size, ori_ppn, callback);
			update_physical_address(divided_lba, sectors-divided_size, ori_ppn+add_ppn, callback);
#endif
		}
	}
	
#ifdef VSSIM_PM
	INCREASE_IO_REQUEST_SEQ_NB();
#endif

#ifdef FTL_DEBUG
	printf("[ftl_write] Success!\n");
#endif
#ifndef VSSIM_PM
	return (count + write_add_count);
#endif
}

// map page를 read할 경우 사용
void ftl_read_map(uint32_t ppn, void *cache)
{
#ifdef VSSIM_PM
	READ_MAP(ftl_calc_bank(ppn), ftl_calc_block(ppn), ftl_calc_page(ppn), cache);
	CELL_READ(ftl_calc_bank(ppn), ftl_calc_block(ppn), ftl_calc_page(ppn), 0, MAP_READ);
#else
	uint32_t *phy_page = (uint32_t *)get_flash_page(ppn);

	ASSERT (cache, !=, NULL);
	ASSERT (ppn, <=, NR_PHY_PAGES);

	memcpy(cache, phy_page, PHY_PAGE_SIZE);
#endif
}

// map page를 write할 경우 사용
#ifdef VSSIM_PM
void ftl_write_map(uint32_t new_ppn, void *cache)
#else
void ftl_write_map(uint32_t new_ppn, void *buf, void *callback)
#endif
{
#ifndef VSSIM_PM
	ASSERT (buf, !=, NULL);
#else
	ASSERT (new_ppn, <=, NR_PHY_PAGES);
#endif

#ifdef VSSIM_PM
	WRITE_MAP(ftl_calc_bank(new_ppn), ftl_calc_block(new_ppn), ftl_calc_page(new_ppn), cache);
	printf("\tWrite Map !!\n");
	CELL_WRITE(ftl_calc_bank(new_ppn), ftl_calc_block(new_ppn), ftl_calc_page(new_ppn), 0, MAP_WRITE);
#else	
	flash_write(callback,
			ftl_calc_bank(new_ppn), 
			ftl_calc_block(new_ppn), 
			ftl_calc_page(new_ppn),
			buf);
#endif	
	write_add_count++;
}

// free block bitmap에 한 bit을 set한다
void set_free_blk_bitmap(uint32_t blk_num)
{
	uint32_t bitmap_num = GET_BLK_PAGENUM(blk_num);
	uint32_t blk_offset = GET_BLK_OFFSET(blk_num);

	SET_FREE_BLK(free_blk_bitmap[bitmap_num], blk_offset);
}

// free block bitmap에서 한 bit를 가져온다
uint32_t get_free_blk_bitmap(uint32_t blk_num)
{
	uint32_t bitmap_num = GET_BLK_PAGENUM(blk_num);
	uint32_t blk_offset = GET_BLK_OFFSET(blk_num);
	
	return GET_FREE_BLK(free_blk_bitmap[bitmap_num], blk_offset);
}
