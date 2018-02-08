#ifndef POLYMORPHIC_MAPPING_H
#define POLYMORPHIC_MAPPING_H

#include "common.h"

/*
 * some more definitions 
 */
// These value is configured by mapping setting value 
// region

#ifdef VSSIM_PM
	#define GET_REGION_IDX(idx)			(idx / (PAGE_SIZE / 140))
	#define REGION_IDX_SIZE				(REGION_NODE_SIZE / (PAGE_SIZE / 140) + 1)
	#define REGION_ENTRIES_PER_PAGE		(PAGE_SIZE / sizeof(struct region_node_))
#else
	#define GET_REGION_IDX(idx)			(idx / (PHY_PAGE_SIZE / 140))
	#define REGION_IDX_SIZE				(REGION_NODE_SIZE / (PHY_PAGE_SIZE / 140) + 1)
	#define REGION_ENTRIES_PER_PAGE		(PHY_PAGE_SIZE / sizeof(struct region_node_))
#endif

// extents
#define GET_REGION_NUM(lpn, page)			(lpn / page)
#define GET_ROOT_NUM(lpn, page)				(lpn / page)		
#define GET_ROOT_NUM_PER_REGION(r_num)		(r_num % ROOT_NODE_SIZE)
#define GET_PAGE_NUM_PER_REGION(lpn, page)	(lpn % page)	 
#define GET_PAGE_NUM_PER_ROOT(lpn, page)	(lpn % page)
#define REGION_NODE_SIZE			(NR_PHY_PAGES / (1024 * 32))	// total size/256M
#define ROOT_NODE_SIZE			16
#define INDEX_NODE_SIZE 		6	// 64bit(6)	32bit(7)
#define LEAF_NODE_SIZE 			6	// 64bit(6)	32bit(8)
#define LEAF_MID_POS			(LEAF_NODE_SIZE / 2)

#define NUM_PAGE_PER_REGION		(ROOT_NODE_SIZE * NUM_PAGE_PER_ROOT)	// 8k
#define NUM_PAGE_PER_ROOT		(MAP_ENTRIES_PER_PAGE) 							// root:16M page:8k * 2k

// rearrange insertion position
#define RE_INSERT_POS(idx, i_num) 		\
	if(idx > LEAF_MID_POS) { 		\
		idx -= (LEAF_MID_POS+1); 	\
		i_num++; 			\
	} 												

/*
 * Data structure in use polymorphic mapping
 */
#pragma pack(4)
// page mapping's entry
typedef struct pm_entry_ {
	uint32_t ppn;
} pm_entry;

// extents table's leaf node
typedef struct leaf_node_ {
	union {
		uint32_t ppn;			// leaf_node
		struct leaf_node_ *l_node;	// index_node
	};
	uint32_t lpn	: 16;
	uint32_t size	: 15;
	uint32_t used	: 1;
} leaf_node;

// extents table's root node
typedef union root_node_ {
	struct leaf_node_ *index_node;
} root_node;

// extents table
typedef struct extents_node_ {
	root_node r_node[ROOT_NODE_SIZE];
	leaf_node i_node[ROOT_NODE_SIZE * INDEX_NODE_SIZE];
	leaf_node l_node[ROOT_NODE_SIZE * INDEX_NODE_SIZE * LEAF_NODE_SIZE];
} extents_node;																

// region table's entry
typedef struct region_node_ {	
	uint32_t map_type;			// extents table : 0 or page mapping : 1
	
	// extents ppn & cache pointer
	uint32_t extents_ppn;		// physical page number
	extents_node *ptr_cache_e;	// cache pointer
	
	// page ppn & cache pointer
	uint32_t page_ppn[ROOT_NODE_SIZE];		// physical page number
	pm_entry *ptr_cache_p[ROOT_NODE_SIZE];	// cache pointer
} region_node;	// total size (140)
#pragma pack()

/*
 * User defined functions in used polymorphic mapping
 */
uint32_t pm_init(void *mem);
void init_extents_table(extents_node *e_node);
void init_extents_table_link(extents_node *e_node);
void init_leaf_node(leaf_node *l_node);

// extents function

#ifdef VSSIM_PM
void insert_leaf_node(uint32_t lpn_ori, uint32_t size, uint32_t new_ppn, extents_node *e_node);
leaf_node *divide_leaf_node(uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node);
void rearrange_index_node(uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node, leaf_node *l_node);
#else
void insert_leaf_node(uint32_t lpn_ori, uint32_t size, uint32_t new_ppn, extents_node *e_node, void *callback);
leaf_node *divide_leaf_node(uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node, void *callback);
void rearrange_index_node(uint32_t r_num, uint32_t i_num, uint32_t l_num, extents_node *e_node, leaf_node *l_node, void *callback);
#endif
void copy_node(leaf_node *src_i_node, leaf_node *dest_i_node, uint32_t l_num);
void move_back_inode (leaf_node *i_node, int i_num);
void move_back_node (leaf_node *l_node, int l_num);
void move_forward_inode(leaf_node *i_node, int i_num);
void move_forward_node(leaf_node *l_node, int l_num);

// pagemap function
#ifdef VSSIM_PM
uint32_t change_pagemap(uint32_t r_num, extents_node *e_node, leaf_node *i_node);
#else
uint32_t change_pagemap(uint32_t r_num, extents_node *e_node, leaf_node *i_node, void *callback);
#endif

// ftl function
#ifdef VSSIM_PM
void update_physical_address_in_extents(uint32_t lba, uint32_t sectors, uint32_t new_ppn, extents_node *e_node);
#else
void update_physical_address_in_extents(uint32_t lba, uint32_t sectors, uint32_t new_ppn, extents_node *e_node, void *callback);
#endif
void update_physical_address_in_pagemap(pm_entry *page_map, uint32_t lpn, uint32_t size, uint32_t new_ppn);
leaf_node *read_extents(leaf_node *i_node, uint32_t lpn, uint32_t *i_offset, uint32_t *l_offset);
pm_entry *read_pagemap(pm_entry *page_map, uint32_t lpn);

// region function
#ifdef VSSIM_PM
uint32_t update_region(uint32_t rg_num);
#else
uint32_t update_region(uint32_t rg_num, void *callback);
#endif
void update_region_idx_ppn(uint32_t num, uint32_t free_page);
uint32_t get_region_idx_ppn(uint32_t num);

// ftl에서 쓰임
leaf_node *get_leaf(uint32_t rg_num, uint32_t *r_offset, uint32_t *i_offset, uint32_t *l_offset);
#endif
