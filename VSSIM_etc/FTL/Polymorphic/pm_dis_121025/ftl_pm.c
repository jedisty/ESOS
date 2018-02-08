#ifdef VSSIM_PM
#include "ftl_pm.h"
#include "ftl.h"
#include "ftl_cache.h"
#include "ssd_io_manager.h"
#else
#include "pm.h"
#include "ftl.h"
#include "cache.h"
#endif

region_node *region_table;
static uint32_t *region_idx_table;			// region mapping info

uint32_t pm_open(void *mem)
{
	region_node *t_rg_node;
	int i;

	uint32_t s_region_table = sizeof(struct region_node_) * REGION_NODE_SIZE;

	// set data structure's address
	region_table = (region_node *)mem;
	region_idx_table = (uint32_t *)(mem + s_region_table);
	if (!region_table || !region_idx_table)
	{
#ifdef VSSIM_PM
		printf("Error: Fixed mapping table allocation failed %p %p\n", region_table, region_idx_table);
#else
		printk("Error: Fixed mapping table allocation failed %p %p\n", region_table, region_idx_table);
#endif
		return -1;
	}

	// initialize region table
	for (i=0; i<REGION_NODE_SIZE; i++)
	{
		t_rg_node = (region_node *)region_table + i;
		t_rg_node->map_type = 0;				// default : extents table(0) page mapping(1)
		t_rg_node->extents_ppn = -1;
		t_rg_node->ptr_cache_e = NULL;

		memset(t_rg_node->page_ppn, 0xFF, sizeof(uint32_t) * ROOT_NODE_SIZE);
//		memset(t_rg_node->ptr_cache_p, 0x00, sizeof(pm_entry *) * ROOT_NODE_SIZE);
	}
	
	return 0;
}

// extents table 초기화
void init_extents_table(extents_node *e_node)
{
	int j;
	leaf_node *t_i_node, *t_l_node;
	root_node *t_r_node;

	// initialize extents table 
	for (j=0; j<ROOT_NODE_SIZE; j++)
	{
		t_i_node = (leaf_node *)e_node->i_node + (j*INDEX_NODE_SIZE);
//		t_i_node = (leaf_node *)e_node->i_node + CALC_INDEX_NODE(j);
		t_i_node->size = NUM_PAGE_PER_ROOT;
		t_i_node->used = 1;		// used(1) unused(0)
		t_i_node->ppn = -1;

		t_r_node = (root_node *)e_node->r_node + j;
		t_r_node->index_node = t_i_node;
	}
	
	for (j=0; j<ROOT_NODE_SIZE*INDEX_NODE_SIZE; j++)
	{
		t_l_node = (leaf_node *)e_node->l_node + (j*LEAF_NODE_SIZE);
//		t_l_node = (leaf_node *)e_node->l_node + CALC_LEAF_NODE(j);
		t_l_node->size = 0;
		t_l_node->used = 0;
		t_l_node->ppn = -1;

		t_i_node = (leaf_node *)e_node->i_node + j;
		t_i_node->l_node = t_l_node;
	}
}

// extents table link 초기화
void init_extents_table_link(extents_node *e_node)
{
	int j;
	leaf_node *t_i_node, *t_l_node;
	root_node *t_r_node;

	// initialize extents table 
	for (j=0; j<ROOT_NODE_SIZE; j++)
	{
		t_i_node = (leaf_node *)e_node->i_node + (j*INDEX_NODE_SIZE);

		t_r_node = (root_node *)e_node->r_node + j;
		t_r_node->index_node = t_i_node;
	}
	
	for (j=0; j<ROOT_NODE_SIZE*INDEX_NODE_SIZE; j++)
	{
		t_l_node = (leaf_node *)e_node->l_node + (j*LEAF_NODE_SIZE);

		t_i_node = (leaf_node *)e_node->i_node + j;
		t_i_node->l_node = t_l_node;
	}
}

// leaf node 초기화
void init_leaf_node(leaf_node *l_node)
{
	leaf_node *t_l_node = NULL;
	int i=0;

	// create leaf node
	for(i=0; i<LEAF_NODE_SIZE; i++){
		t_l_node = (leaf_node*)l_node + i;
		t_l_node->lpn=0;
		t_l_node->size=0;
		t_l_node->used=0;
		t_l_node->ppn=-1;
	}
}

void update_physical_address_in_extents(uint32_t lba, uint32_t sectors, uint32_t new_ppn, extents_node *e_node, void *callback)
{
	uint32_t lpn = lba / SECTORS_PER_PAGE;
	uint32_t size = (lba+sectors-1)/SECTORS_PER_PAGE - lpn + 1;
	
	// insert new leaf node into the root table
	insert_leaf_node(lpn, size, new_ppn, e_node, callback);
}

void insert_leaf_node(uint32_t lpn_ori, uint32_t size, uint32_t new_ppn, extents_node *e_node, void *callback)
{
	uint32_t divide_flag = 0;
	uint32_t i_num, l_num;
	uint32_t cur_start, cur_size, cur_ppn;
	
	uint32_t rg_num = GET_REGION_NUM(lpn_ori, NUM_PAGE_PER_REGION);
	uint32_t r_num = GET_ROOT_NUM(lpn_ori, NUM_PAGE_PER_ROOT);
	uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);					// per region
	uint32_t lpn = GET_PAGE_NUM_PER_ROOT(lpn_ori, NUM_PAGE_PER_ROOT);	// lpn in root

	root_node *r_node = e_node->r_node + r_offset;
	leaf_node *i_node = r_node->index_node;
	leaf_node *l_node = NULL;
	leaf_node *t_i_node, *t_l_node;

	ASSERT(r_node, !=, NULL);
	ASSERT(i_node, !=, NULL);

	// write request를 넣을 index node 검색
	for (i_num=0; i_num<INDEX_NODE_SIZE; i_num++)
	{
		t_i_node = (leaf_node *)i_node + i_num;

		if (t_i_node->lpn + t_i_node->size > lpn)
		{
			// get the leaf node
			l_node = t_i_node->l_node;

			break;
		}
	}

	// index node 내의 leaf node를 검색
	for (l_num=0; l_num<LEAF_NODE_SIZE; l_num++)
	{
		t_l_node = (leaf_node *)l_node + l_num;

		cur_start = t_l_node->lpn;
		cur_size = t_l_node->size;
		cur_ppn = t_l_node->ppn;
		
		if (t_l_node->used == 0)
		{
			break;
		}
		// 이전 leaf node에 삽입
		else if (cur_start >= lpn)
		{
			if (cur_start + cur_size <= lpn + size)
			{
				// ovwrite old leaf node
				break;
			}
			else
			{
				// if leaf node is full, divide it into halves.
				if (((leaf_node *)l_node + (LEAF_NODE_SIZE-1))->used)
				{
					l_node = divide_leaf_node (rg_num, r_num, i_num, l_num, e_node, callback);
					RE_INSERT_POS(l_num, i_num);

					divide_flag = 1;
				}
				else
				{
					move_back_node (l_node, l_num);
				}

				break;
			}
		}
	}

	// 현재 index node가 꽉 찼을 경우
	if (l_num == LEAF_NODE_SIZE)
	{
		l_node = divide_leaf_node (rg_num, r_num, i_num, l_num, e_node, callback);
		RE_INSERT_POS(l_num, i_num);

		divide_flag = 1;
	}

	// 현재 root node가 page mapping으로 변했을 경우
	if (!l_node)
	{
		region_node *rg_node = (region_node *)region_table + rg_num;
		
		// insert request into page mapping
		update_physical_address_in_pagemap(rg_node->ptr_cache_p[r_offset], lpn, size, new_ppn);
	}
	else
	{
		t_l_node = (leaf_node *)l_node + l_num;

		// insert request into leaf node
		t_l_node->used = 1;
		t_l_node->lpn = lpn;
		t_l_node->size = size;
		t_l_node->ppn = new_ppn;

		// index node의 모든 leaf node를 재정렬
		rearrange_index_node(rg_num, r_num, i_num, l_num, e_node, l_node, callback);
	}
}

leaf_node *divide_leaf_node(uint32_t rg_num, uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node, void *callback)
{
	uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);
	uint32_t next_i_num = i_num + 1;

	root_node *r_node = (root_node *)e_node->r_node + r_offset;
	leaf_node *i_node = r_node->index_node;
	leaf_node *t_i_node, *src_i_node, *dest_i_node;

	ASSERT(e_node, !=, NULL);

	// index node가 꽉 찼을 경우 page mapping으로 변환
	if (((leaf_node *)i_node + (INDEX_NODE_SIZE-1))->used == 1)
	{
		// if index node can't be divided, index node change to page mapping
		change_pagemap(rg_num, r_num, e_node, i_node, callback);
	
		return NULL;
	}

	// 다음 index node를 사용
	t_i_node = (leaf_node *)i_node + (next_i_num);
	// 다음 index node가 사용 중일 경우
	if (t_i_node->used == 1)
	{
		move_back_inode(i_node, next_i_num);
		t_i_node = (leaf_node *)i_node + next_i_num;
		t_i_node->l_node = (leaf_node *)i_node->l_node + (next_i_num * LEAF_NODE_SIZE);
	}
	else
	{
		init_leaf_node(t_i_node->l_node);
	}

	// initialize next index node's leaf node
	init_leaf_node(t_i_node->l_node);

	src_i_node = (leaf_node *)i_node + i_num;
	dest_i_node = (leaf_node *)i_node + next_i_num;

	ASSERT(src_i_node, !=, NULL);
	ASSERT(dest_i_node, !=, NULL);

	// 꽉 찬 index node의 leaf node를 반으로 나누어 복사
	copy_node (src_i_node, dest_i_node, l_num);
	
	if (l_num < LEAF_MID_POS+1)
		return src_i_node->l_node;
	else
		return dest_i_node->l_node;
}
			
void rearrange_index_node(uint32_t rg_num, uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node, leaf_node *l_node, void *callback)
{
	uint32_t cur_start, cur_size, cur_ppn;
	uint32_t next_start, next_size;
	uint32_t next_i_num, next_l_num;
	int i;
	leaf_node *t_l_node, *cur_node, *next_node;

	// new leaf node를 기준으로 이전 leaf node를 정렬
	if (l_num != 0)
	{
		cur_node = (leaf_node *)l_node + (l_num-1);
		cur_start = cur_node->lpn;
		cur_size = cur_node->size;
		cur_ppn = cur_node->ppn;
		
		next_node = (leaf_node *)l_node + l_num;
		next_start = next_node->lpn;
		next_size = next_node->size;

		// leaf node가 겹칠 경우
		if (cur_start + cur_size > next_start)
		{
			// leaf node가 기존 leaf node에 걸칠 경우
			if (cur_start + cur_size <= next_start + next_size)
			{
				cur_node->size = next_start - cur_start;
			}
			// leaf node가 기존 leaf node의 사이에 겹칠 경우
			else if (cur_start + cur_size > next_start + next_size)
			{
				cur_node->size = next_start - cur_start;
				
				i = l_num + 1;

				// if leaf node is full, divide it
				if (((leaf_node *)l_node + (LEAF_NODE_SIZE-1))->used)
				{
					l_node = divide_leaf_node(rg_num, r_num, i_num, i, e_node, callback);
					
					RE_INSERT_POS(i, i_num);

					if (l_node == NULL)
						return ;
				}
				else
				{
					move_back_node (l_node, i);
				}
				
				t_l_node = (leaf_node *)l_node + i;
				t_l_node->used = 1;
				t_l_node->lpn = next_start + next_size;
				t_l_node->size = (cur_start + cur_size) - (next_start + next_size);
				t_l_node->ppn = cur_ppn + (cur_size - t_l_node->size); 
			}
		}
	}

	next_i_num = i_num+1;
	
	// new leaf node를 기준으로 이후의 leaf node를 정렬
	for (i=l_num; i<LEAF_NODE_SIZE; i++)
	{
		next_l_num = i + 1;

		// root node의 마지막 leaf node일 경우
		if (next_i_num == INDEX_NODE_SIZE && next_l_num == LEAF_NODE_SIZE)
			break;
		
		ASSERT((leaf_node *)l_node + i, !=, NULL);

		cur_node = (leaf_node *)l_node + i;
		cur_start = cur_node->lpn;
		cur_size = cur_node->size;

		next_node = (leaf_node *)l_node + next_l_num;
		
		// index node의 마지막 leaf node일 경우
		if ((next_i_num != INDEX_NODE_SIZE) && ((next_l_num == LEAF_NODE_SIZE) || (next_l_num < LEAF_NODE_SIZE && !next_node->used)))
		{
			// next node is next index's first leaf node	
			uint32_t next_i_start;

			uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);
			root_node *r_node = (root_node *)e_node->r_node + r_offset;
			leaf_node *i_node = (leaf_node *)r_node->index_node + i_num;
			leaf_node *next_i_node = (leaf_node *)r_node->index_node + next_i_num;
			
			ASSERT(r_node, !=, NULL);
			ASSERT(i_node, !=, NULL);
			ASSERT(next_i_node, !=, NULL);

			next_node = next_i_node->l_node;
			next_i_start = next_i_node->lpn;

			// 다음 leaf node가 없을 경우
			if (i_node->used == 0 || next_node == NULL || next_node->used == 0)
				break;

			next_start = next_node->lpn;
			next_size = next_node->size;

			// leaf node가 겹치지 않을 경우 
			if (cur_start + cur_size <= next_start)
				break;

			// 기존 leaf node가 leaf node에 겹칠 경우
			if (cur_start + cur_size >= next_start + next_size)
			{
				i_node->size = (next_node->lpn + next_node->size) - i_node->lpn;
				next_i_node->lpn = i_node->lpn + i_node->size;
				
				if (next_i_node->lpn + next_i_node->size > NUM_PAGE_PER_ROOT)
					next_i_node->size = NUM_PAGE_PER_ROOT - next_i_node->lpn;

				move_forward_node (next_node, 0);
				i--;

				if (!next_i_node->l_node->used)
				{
					init_leaf_node (next_i_node);
					move_forward_inode ((leaf_node *)r_node->index_node, next_i_num);
				}

				continue;
			}
			// 기존 leaf node가 leaf node에 걸칠 경우
			else
			{
				i_node->size = (cur_node->lpn + cur_node->size) - i_node->lpn;
				next_i_node->lpn = i_node->lpn + i_node->size;

				if (next_i_node->lpn + next_i_node->size > MAP_ENTRIES_PER_PAGE)
					next_i_node->size = MAP_ENTRIES_PER_PAGE - next_i_node->lpn;
				
				next_node->ppn = next_node->ppn + (cur_node->lpn + cur_node->size - next_node->lpn);
				next_node->size = (next_node->lpn + next_node->size) - (cur_node->lpn + cur_node->size);
				next_node->lpn = cur_node->lpn + cur_node->size;

				break;
			}
		}

		next_start = next_node->lpn;
		next_size = next_node->size;

		// leaf node가 겹치지 않을 경우
		if (cur_start + cur_size <= next_start || !next_node->used)
			break;

		// leaf nodr가 겹칠 경우
		if (cur_start <= next_start)
		{
			if(cur_start + cur_size >= next_start + next_size)
			{
				// remove next node
				move_forward_node (l_node, i+1);
				i--;
			}
			else
			{
				// cur_start + cur_size < next_start + next_size
				next_node->lpn = cur_start + cur_size;
				next_node->size = (next_start + next_size) - (cur_start + cur_size);
				next_node->ppn += (cur_start + cur_size - next_start);
				
				break;
			}
		}
		else
		{
#ifdef VSSIM_PM
			printf("Error : can not happen in rearrange function!!!\n");
#else
			printk("Error : can not happen in rearrange function!!!\n");
#endif
		}
	}
}

void copy_node(leaf_node *src_i_node, leaf_node *dest_i_node, uint32_t l_num)
{
	leaf_node *t_src_node, *t_dest_node;
	leaf_node *src_l_node = src_i_node->l_node;
	leaf_node *dest_l_node = dest_i_node->l_node;
	int i, j;
	uint32_t cp = (l_num< LEAF_MID_POS+1)?LEAF_MID_POS:LEAF_MID_POS+1;

	// copy from src_node to dest_node
	for (i=cp, j=0; i<LEAF_NODE_SIZE; i++, j++)
	{
		t_src_node = (leaf_node *)src_l_node + i;

		// 삽입 지점 지정
		if (l_num == i && i > LEAF_MID_POS)
				j++;

		if (t_src_node->used)
		{
			t_dest_node = (leaf_node *)dest_l_node + j;
			
			memcpy(t_dest_node, t_src_node, sizeof(struct leaf_node_));
			memset(t_src_node, 0x00, sizeof(struct leaf_node_));
		}
		else
		{
			break;
		}
	}

	// index node의 범위 수정
	dest_i_node->used = 1;

	if (l_num < LEAF_MID_POS+1)
	{
		t_dest_node = (leaf_node *)dest_l_node;
		dest_i_node->lpn = t_dest_node->lpn;
		dest_i_node->size = (src_i_node->lpn + src_i_node->size) - dest_i_node->lpn;
		src_i_node->size = src_i_node->size - dest_i_node->size;

		if (l_num < LEAF_MID_POS)
			move_back_node(src_l_node, l_num);
	}
	else
	{
		t_src_node = (leaf_node *)src_l_node + LEAF_MID_POS;

		dest_i_node->lpn = t_src_node->lpn + t_src_node->size;
		dest_i_node->size = (src_i_node->lpn + src_i_node->size) - dest_i_node->lpn;
		src_i_node->size = src_i_node->size - dest_i_node->size;
	}
}

void move_back_inode (leaf_node *i_node, int i_num)
{
	leaf_node *t_src_inode = NULL, *t_dest_inode = NULL;
	int i;

	// move back one node
	for (i=INDEX_NODE_SIZE-2; i>=i_num; i--)
	{
		t_src_inode = (leaf_node *)i_node + i;

		ASSERT(t_src_inode, !=, NULL);

		if (t_src_inode->used == 1)
		{
			t_dest_inode = (leaf_node *)i_node + (i+1);
		
			ASSERT(t_dest_inode, !=, NULL);

			memcpy(t_dest_inode->l_node, t_src_inode->l_node, sizeof(struct leaf_node_)*LEAF_NODE_SIZE);
			memcpy(t_dest_inode, t_src_inode, sizeof(struct leaf_node_));

			t_dest_inode->l_node = (leaf_node *)i_node->l_node + ((i+1)*LEAF_NODE_SIZE);
			t_src_inode->l_node = (leaf_node *)i_node->l_node + (i*LEAF_NODE_SIZE);
		}
	}

	if (t_src_inode && t_src_inode->used)
	{
		init_leaf_node(t_src_inode->l_node);
		memset(t_src_inode, 0x00, sizeof(struct leaf_node_));

		t_src_inode->l_node = (leaf_node *)i_node->l_node + (i*LEAF_NODE_SIZE);
		t_src_inode->ppn = -1;
	}
}

void move_back_node (leaf_node *l_node, int l_num)
{
	leaf_node *t_src_node = NULL, *t_dest_node = NULL;
	int i;

	if (l_num == LEAF_NODE_SIZE -1)
		return;

	// move back one node
	for (i=LEAF_NODE_SIZE-1; i>=l_num; i--)
	{
		t_src_node = (leaf_node *)l_node + i;

		if (t_src_node->used == 1)
		{
			t_dest_node = (leaf_node *)l_node + (i+1);
			memcpy(t_dest_node, t_src_node, sizeof(struct leaf_node_));
		}
	}

	if (t_src_node && t_src_node->used)
	{
		memset(t_src_node, 0x00, sizeof(struct leaf_node_));
		t_src_node->ppn = -1;
	}
}

void move_forward_inode(leaf_node *i_node, int i_num)
{
	int i;
	leaf_node *t_src_inode = NULL, *t_dest_inode = NULL;

	// move forward one node
	for (i=i_num; i<INDEX_NODE_SIZE-1; i++)
	{
		t_src_inode = (leaf_node *)i_node + (i+1);
		t_dest_inode = (leaf_node *)i_node + i;

		ASSERT(t_src_inode, !=, NULL);
		ASSERT(t_dest_inode, !=, NULL);

		memcpy(t_dest_inode->l_node, t_src_inode->l_node, sizeof(struct leaf_node_)*LEAF_NODE_SIZE);
		memcpy(t_dest_inode, t_src_inode, sizeof(struct leaf_node_)*LEAF_NODE_SIZE);
	
		t_dest_inode->l_node = (leaf_node *)i_node->l_node + (i*LEAF_NODE_SIZE);
		t_src_inode->l_node = (leaf_node *)i_node->l_node + ((i+1)*LEAF_NODE_SIZE);

		if (t_src_inode->used == 0)
			break;
	}

	if (t_src_inode && t_src_inode->used)
	{
		init_leaf_node(t_src_inode->l_node);
		memset (t_src_inode, 0x00, sizeof(struct leaf_node_));
		
		t_src_inode->l_node = (leaf_node *)i_node->l_node + ((i+1)*LEAF_NODE_SIZE);
		t_src_inode->ppn = -1;
	}
	else
	{
		init_leaf_node(((leaf_node *)i_node + (LEAF_NODE_SIZE-1))->l_node);
		memset ((leaf_node *)i_node + (LEAF_NODE_SIZE-1), 0x00, sizeof(struct leaf_node_));

		((leaf_node*)i_node + (LEAF_NODE_SIZE-1))->l_node = (leaf_node *)i_node->l_node + (LEAF_NODE_SIZE-1)*LEAF_NODE_SIZE;
		((leaf_node*)i_node + (LEAF_NODE_SIZE-1))->ppn = -1;
	}
}

void move_forward_node(leaf_node *l_node, int l_num)
{
	int i;
	leaf_node *t_src_node = NULL, *t_dest_node = NULL;

	// move forward one node
	for (i=l_num; i<LEAF_NODE_SIZE-1; i++)
	{
		t_src_node = (leaf_node *)l_node + (i+1);
		t_dest_node = (leaf_node *)l_node + i;

		ASSERT(t_src_node, !=, NULL);
		ASSERT(t_dest_node, !=, NULL);

		memcpy(t_dest_node, t_src_node, sizeof(struct leaf_node_));
	
		if (t_src_node->used == 0)
			break;
	}

	if (t_src_node && t_src_node->used)
	{
		memset (t_src_node, 0x00, sizeof(struct leaf_node_));
		t_src_node->ppn = -1;
	}
	else
	{
		memset ((leaf_node *)l_node + (LEAF_NODE_SIZE-1), 0x00, sizeof(struct leaf_node_));
		((leaf_node *)l_node + (LEAF_NODE_SIZE-1))->ppn = -1;
	}

}

pm_entry *change_pagemap(uint32_t rg_num, uint32_t r_num, extents_node *e_node, leaf_node *i_node, void *callback)
{
	region_node *rg_node = (region_node *)region_table + rg_num;
	leaf_node *t_i_node, *t_l_node;
	pm_entry *page_map;

	uint32_t r_offset = GET_ROOT_NUM_PER_REGION(r_num);
	uint32_t i_num, l_num;

	// page mapping 영역 초기화
	page_map = init_pagemap(rg_num, r_num, e_node, callback);

	// extents mapping되어 있는 root node를  page mapping으로 변환
	for (i_num=0; i_num<INDEX_NODE_SIZE; i_num++)
	{
		t_i_node = (leaf_node *)i_node + i_num;

		if (t_i_node->used)
		{
			for (l_num=0; l_num<LEAF_NODE_SIZE; l_num++)
			{
				t_l_node = (leaf_node *)t_i_node->l_node + l_num;

				if (t_l_node->used)
				{
					update_physical_address_in_pagemap(page_map, t_l_node->lpn, t_l_node->size, t_l_node->ppn);
				}
			}

			// index node 초기화
			memset(t_i_node->l_node, 0x00, sizeof(struct leaf_node_)*LEAF_NODE_SIZE);
			memset(t_i_node, 0x00, sizeof(struct leaf_node_));
		}
	}

	// page mapping 변환 여부 체크
	SET_FLAG_BIT(rg_node->map_type, r_offset);
	
	return page_map;
}

void update_physical_address_in_pagemap(pm_entry *page_map, uint32_t lpn, uint32_t size, uint32_t new_ppn)
{
	int i;

	for (i=0; i<size; i++)
	{
		((pm_entry *)page_map + (lpn+i))->ppn = new_ppn++;
	}
}

pm_entry *read_pagemap(pm_entry *page_map, uint32_t lpn)
{
	return (pm_entry *)page_map + lpn;
}

leaf_node *read_extents(leaf_node *i_node, uint32_t lpn, uint32_t *i_offset, uint32_t *l_offset)
{
	uint32_t start, size;
	uint32_t i_num, l_num;
	leaf_node *l_node = NULL, *t_i_node, *t_l_node;

	ASSERT(i_node, !=, NULL);

	// index node 탐색
	for (i_num=0; i_num<INDEX_NODE_SIZE; i_num++)
	{
		t_i_node = (leaf_node *)i_node + i_num;
		ASSERT(t_i_node, !=, NULL);
		
		if(!t_i_node->used)
			return NULL;

		start = t_i_node->lpn;
		size = t_i_node->size;

		if (start <= lpn && start + size > lpn)
		{
			l_node = t_i_node->l_node;

			break;
		}
	}

	// 해당 index node가 없을 경우
	if (i_num == INDEX_NODE_SIZE || !l_node->used)
	{
		return NULL;
	}

	// index node내의 leaf node 탐색
	for (l_num=0; l_num<LEAF_NODE_SIZE; l_num++)
	{
		t_l_node = (leaf_node *)l_node + l_num;

		start = t_l_node->lpn;
		size = t_l_node->size;
		
		if (start <= lpn && start + size > lpn)
		{
			*i_offset = i_num;
			*l_offset = l_num;

			// 해당 leaf node 반환
			return t_l_node;
		}
	}

	return NULL;
}

leaf_node *get_leaf(uint32_t rg_num, uint32_t *r_offset, uint32_t *i_offset, uint32_t *l_offset)
{
	uint32_t r_num = *r_offset, i_num = *i_offset, l_num = *l_offset;
	uint32_t r_offset_per_region = GET_ROOT_NUM_PER_REGION(r_num);		// per region
	leaf_node *i_node, *t_i_node;
	region_node *rg_node = (region_node *)region_table + rg_num;
	extents_node *e_node = rg_node->ptr_cache_e;
	root_node *r_node = (root_node *)e_node->r_node + r_offset_per_region;
	i_node = r_node->index_node;

	if (l_num == INDEX_NODE_SIZE)
	{
		// 다음 index node의 leaf node 반환
		t_i_node = (leaf_node *)i_node + (i_num + 1);

		*i_offset = i_num + 1;
		*l_offset = 0;

		return (leaf_node *)(t_i_node->l_node);
	}
	else
	{
		// 현재 index node의 다음 leaf node 반환
		t_i_node = (leaf_node *)i_node + i_num;
		*l_offset = l_num + 1;

		return (leaf_node *)((leaf_node *)t_i_node->l_node + (l_num+1));
	}
}

uint32_t update_region(uint32_t rg_num, void *callback)
{
	int32_t page_num = rg_num / REGION_ENTRIES_PER_PAGE;
	int32_t rg_idx = page_num * REGION_ENTRIES_PER_PAGE;
	uint32_t free_page;

	region_node *rg_node = NULL;

	// TO DO : garbage collection
	
	free_page = get_free_page_in_reserved();
	
	if (free_page)
	{
		rg_node = (region_node *)region_table + rg_idx;
		
		ftl_write_map(free_page, rg_node, callback);
		update_region_idx_ppn(page_num, free_page);
	}
	else
	{
#ifdef VSSIM_PM
		printf("[%s] free page allocation failed!!\n", __FUNCTION__);
#else
		printk("[%s] free page allocation failed!!\n", __FUNCTION__);
#endif
	}
	
	return 0;
}

void update_region_idx_ppn(uint32_t num, uint32_t free_page)
{
	uint32_t rg_idx = GET_REGION_IDX(num);

	region_idx_table[rg_idx] = free_page;
}

uint32_t get_region_idx_ppn(uint32_t num)
{
	return region_idx_table[num];
}
