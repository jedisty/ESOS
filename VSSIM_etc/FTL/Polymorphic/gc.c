#include "common.h"

/**************************************/
/* data structure and global variable */
/**************************************/
extern region_node *region_table;
extern struct ftl_metadata_ *ftl_metadata;
extern uint32_t write_add_count;
extern void *tmp_buf;
#ifdef VSSIM_GC
sblock_meta* super_block_meta;
#else
sblock_meta super_block_meta[BLOCKS_PER_BANK];
#endif

extern int step_flag;
uint32_t vt_blk_num;

/*************************/
/* user function */
/*************************/
void gc_init(void){
	int i;
#ifdef VSSIM_GC
	super_block_meta = (sblock_meta*)malloc(BLOCK_NB * sizeof(sblock_meta));
	for(i=0; i<BLOCK_NB;i++ )
#else
	for(i=0; i<BLOCKS_PER_BANK;i++ )
#endif
	{
		super_block_meta[i].valid_cnt= 0;
	}	
}

void gc_print_valid(void){
	int i;
#ifdef VSSIM_GC
	for(i=0; i<BLOCK_NB; i++)
#else
	for(i=0; i<BLOCKS_PER_BANK; i++)
#endif
	{
		if(super_block_meta[i].valid_cnt != 0){
#ifdef VSSIM_GC
			printf("super_block_meta[%d].valid_cnt: %d \n", i, super_block_meta[i].valid_cnt);
#else
			printk("super_block_meta[%d].valid_cnt: %d \n", i, super_block_meta[i].valid_cnt);
#endif
		}
	}

}


void print_mapping_table(void){
	int x, i, j, k;

	extents_node *e_node;
	leaf_node *i_node;
	leaf_node *l_node;
	leaf_node *t_i_node, *t_l_node;

	for(x=0; x <REGION_NODE_SIZE; x++ ){
		if( region_table == NULL) {
#ifdef VSSIM_GC
			printf("region table does not exist\n");
#else
			printk("region table does not exist\n");
#endif
			break;
		}
		
		region_node *t_rg_node = (region_node *)((region_node *)region_table + x);
		
		if( t_rg_node == NULL) {
#ifdef VSSIM_GC
			printf("%d th information of the region table does not exist\n", x);
#else
			printk("%d th information of the region table does not exist\n", x);
#endif
			break;
		}
		
		e_node = t_rg_node->ptr_cache_e;
		if( e_node == NULL) {
#ifdef VSSIM_GC
			printf("in-memory extent mapping table in region %d does not exist: \n", x);
#else
			printk("in-memory extent mapping table in region %d does not exist: \n", x);
#endif
			break;
		}
	
		for (i=0; i<ROOT_NODE_SIZE; i++){
			if(BIT_GET(t_rg_node->map_type,GET_ROOT_NUM_PER_REGION(i))){
				for (j=0; j< MAP_ENTRIES_PER_PAGE; j++) {
					if(t_rg_node->ptr_cache_p == NULL || t_rg_node->ptr_cache_p[i] == NULL) {
#ifdef VSSIM_GC
						printf("in-memory page mapping table in region %d does not exist: \n",i);
#else
						printk("in-memory page mapping table in region %d does not exist: \n",i);
#endif
						break;
					}

					if((t_rg_node->ptr_cache_p[i]+j) != NULL){
						if(t_rg_node->ptr_cache_p[i][j].ppn != -1 ){	
#ifdef VSSIM_GC
							printf("region[%d] ppn[%d][%d]: %d, page calc block : %d   \n ", x, i, j, t_rg_node->ptr_cache_p[i][j].ppn, ftl_calc_block(t_rg_node->ptr_cache_p[i][j].ppn));
#else
							printk("region[%d] ppn[%d][%d]: %d, page calc block : %d   \n ", x, i, j, t_rg_node->ptr_cache_p[i][j].ppn, ftl_calc_block(t_rg_node->ptr_cache_p[i][j].ppn));
#endif
						}
					}		
				}
			}else{
				if(e_node->i_node == NULL) {
#ifdef VSSIM_GC
					printf("index root tree of in-memory extent mapping table in region %d does not exist: \n",x);
#else
					printk("index root tree of in-memory extent mapping table in region %d does not exist: \n",x);
#endif
					break;
				}
				i_node = e_node->i_node;
		
				for (j=0; j<INDEX_NODE_SIZE; j++){
					
					t_i_node = (leaf_node *)i_node + (i*INDEX_NODE_SIZE) + j;
	
					if (t_i_node && t_i_node->used){

#ifdef VSSIM_GC
						printf("[print_node] region[%d] root[%d] index[%d] used %d lpn %d size %d ppn %d\n", x , i, j, t_i_node->used, t_i_node->lpn, t_i_node->size, t_i_node->ppn);
#else
						printk("[print_node] region[%d] root[%d] index[%d] used %d lpn %d size %d ppn %d\n", x , i, j, t_i_node->used, t_i_node->lpn, t_i_node->size, t_i_node->ppn);
#endif
						l_node = t_i_node->l_node;

						if (t_i_node && t_i_node->used){
							for (k=0; k<LEAF_NODE_SIZE; k++){

				    			t_l_node = (leaf_node *)l_node + k;
								if (t_l_node && t_l_node->used){
#ifdef VSSIM_GC
									printf(" root[%d] index[%d] leaf[%d] used %d lpn %d size %d ppn %d block:%d \n", i, j, k, t_l_node->used, t_l_node->lpn, t_l_node->size, t_l_node->ppn, ftl_calc_block(t_l_node->ppn));
#else
									printk(" root[%d] index[%d] leaf[%d] used %d lpn %d size %d ppn %d block:%d \n", i, j, k, t_l_node->used, t_l_node->lpn, t_l_node->size, t_l_node->ppn, ftl_calc_block(t_l_node->ppn));
#endif
								}
							}
						}
					}
				}
			}
		}
	}
}

void gc_valid_count(void){
	int x, y, i, j, k;

	printf("\t[%s] start!\n",__FUNCTION__);

	extents_node *e_node;
	leaf_node *i_node;
	leaf_node *l_node;
	leaf_node *t_i_node, *t_l_node;

	gc_init();
	for(x=0; x <REGION_NODE_SIZE; x++ ){
		region_node *t_rg_node = (region_node *)region_table + x;
		e_node = t_rg_node->ptr_cache_e;
		if( e_node == NULL){
			continue;
		}

		for (i=0; i<ROOT_NODE_SIZE; i++){
			if(BIT_GET(t_rg_node->map_type,GET_ROOT_NUM_PER_REGION(i))){
				if(t_rg_node->ptr_cache_p == NULL || t_rg_node->ptr_cache_p[i] == NULL){
					continue;
				}

				for (j=0; j< MAP_ENTRIES_PER_PAGE; j++) {
					if((t_rg_node->ptr_cache_p[i]+j) != NULL){
						if(t_rg_node->ptr_cache_p[i][j].ppn != -1 ){	
						super_block_meta[ftl_calc_block(t_rg_node->ptr_cache_p[i][j].ppn)].valid_cnt++;
						}
					}		
				}
			}else{
				if( e_node->i_node == NULL){
					continue;
				}

				i_node = e_node->i_node;
		
				for (j=0; j<INDEX_NODE_SIZE; j++){
					
					t_i_node = (leaf_node *)i_node + (i*INDEX_NODE_SIZE) + j;
	
					if (t_i_node && t_i_node->used){

						l_node = t_i_node->l_node;

						if (t_i_node && t_i_node->used){
							for (k=0; k<LEAF_NODE_SIZE; k++){

				    			t_l_node = (leaf_node *)l_node + k;
								if (t_l_node && t_l_node->used){

									if(t_l_node->ppn != -1){
										super_block_meta[ftl_calc_block(t_l_node->ppn)].valid_cnt++;
										
										for(y= 1; y<t_l_node->size; y++){	
											super_block_meta[ftl_calc_block(t_l_node->ppn+y)].valid_cnt++;
						
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	printf("\t[%s] end!\n",__FUNCTION__);
//gc_print_valid();

}


uint32_t gc_victim_block_select(void){
	int i;
	int min = NR_RESERVED_PHY_SUPER_BLOCKS;

	printf("\t[%s] start!\n",__FUNCTION__);
	gc_valid_count();
#ifdef VSSIM_GC
	for (i=NR_RESERVED_PHY_SUPER_BLOCKS; i < BLOCK_NB; i++)
#else
	for (i=NR_RESERVED_PHY_SUPER_BLOCKS; i < BLOCKS_PER_BANK; i++)
#endif
	{	
		if ( (ftl_get_free_blk_bitmap(i) != 0) &&
			 ((ftl_metadata->write_map_block_ptr) != i) && 
			 ((ftl_metadata->write_block_ptr) != i) && 
			 ((ftl_metadata->block_ptr) != i) ){
				min = i;
				goto next;
		}			
	}
#ifdef VSSIM_GC
	if (i == BLOCK_NB)
#else
	if (i == BLOCKS_PER_BANK)
#endif
	{
#ifdef VSSIM_GC
		printf("victim blk: not found!\n");
#else
		printk("victim blk: not found!\n");
#endif
		return -1;
	}


	next:
	
#ifdef VSSIM_GC
	for (i=min+1; i < BLOCK_NB; i++)
#else
	for (i=min+1; i < BLOCKS_PER_BANK; i++)
#endif
	{	
		if (super_block_meta[min].valid_cnt > super_block_meta[i].valid_cnt) {
		 	if ( (ftl_get_free_blk_bitmap(i) != 0) &&
				 ((ftl_metadata->write_map_block_ptr) != i) && 
				 ((ftl_metadata->write_block_ptr) != i) && 
				 ((ftl_metadata->block_ptr) != i) ){
					min = i;
							
			}
		}
   	}
#ifdef VSSIM_GC
	if (super_block_meta[min].valid_cnt >= PAGE_NB*FLASH_NB*WAY_NB )
#else
	if (super_block_meta[min].valid_cnt >= PAGES_PER_BLOCK*NR_PHY_BANKS*NR_PHY_WAYS )
#endif
	{
#ifdef VSSIM_GC
		printf("victim blk: all valid page !!\n");
#else
		printk("victim blk: all valid page !!\n");
#endif
		return -1;
	} else{
		return min;
	}
	printf("\t[%s] end!\n",__FUNCTION__);
}


int garbage_collection(void){
//	uint32_t blk_num = gc_victim_block_select();
	uint32_t res;
	uint32_t remain_pages;
	int i, j, k, l, n, o;

	extents_node *e_node;
	leaf_node *i_node;
	leaf_node *l_node;
	leaf_node *t_i_node, *t_l_node;

	if(step_flag == 0 || step_flag == HARD){
#ifdef PMM_GC_DEBUG
		printf("GC step start: %d blk: %d \n", step_flag, vt_blk_num);
#endif
		vt_blk_num = gc_victim_block_select();
#ifdef PMM_GC_DEBUG
		printf("GC step finish: %d blk: %d \n", step_flag, vt_blk_num);
#endif
		if(step_flag != HARD)
			step_flag = 1;
	}
#ifdef HOST_QUEUE
	if(IS_EVENT(e_queue) && step_flag != HARD){
		printf("preempt !!!\n");
		return 0;
	}
#endif

#ifdef VSSIM_GC
	if (vt_blk_num == BLOCK_NB)
#else
	if (vt_blk_num == BLOCKS_PER_BANK)
#endif
	{
#ifdef VSSIM_GC
		printf("gc error\n");
#else
		printk("gc error\n");
#endif
	}
	else
	{
#ifdef VSSIM_GC
	//	printf("\tEnter gc\n");
#else
		printk("\tEnter gc\n");
#endif
	}

	if(step_flag == 1 || step_flag == HARD){
#ifdef PMM_GC_DEBUG
	//	printf("GC step start: %d blk: %d \n", step_flag, vt_blk_num);
#endif
	}

#ifdef VSSIM_GC
	remain_pages = (ftl_calc_block(ftl_metadata->write_ptr)+1)*FLASH_NB*WAY_NB*PAGE_NB-ftl_metadata->write_ptr;
#else
	remain_pages = (ftl_calc_block(ftl_metadata->write_ptr)+1)*NR_PHY_BANKS*NR_PHY_WAYS*PAGES_PER_BLOCK-ftl_metadata->write_ptr;
#endif
	for(i=0; i <REGION_NODE_SIZE; i++ ){
		region_node *t_rg_node = (region_node *)region_table + i;
		e_node = t_rg_node->ptr_cache_e;
		if( e_node == NULL) {
			continue;
		}
	

		for (j=0; j<ROOT_NODE_SIZE; j++){
			if(BIT_GET(t_rg_node->map_type,j)){
				if(t_rg_node->ptr_cache_p == NULL || t_rg_node->ptr_cache_p[j] == NULL) {
					continue;
				}

				for (k=0; k<NUM_PAGE_PER_ROOT; k++) {

					if ((t_rg_node->ptr_cache_p[j]+k) != NULL){
						if (ftl_calc_block(t_rg_node->ptr_cache_p[j][k].ppn) == vt_blk_num) {

#ifndef VSSIM_GC
							flash_read_gc(ftl_calc_bank(t_rg_node->ptr_cache_p[j][k].ppn),	
										 ftl_calc_block(t_rg_node->ptr_cache_p[j][k].ppn), 
										 ftl_calc_page(t_rg_node->ptr_cache_p[j][k].ppn), 
										 tmp_buf);
#endif						
							res = ftl_get_free_page();
							t_rg_node->ptr_cache_p[j][k].ppn = res;

#ifndef VSSIM_GC
							flash_write_gc(ftl_calc_bank(res),
										ftl_calc_block(res),
										ftl_calc_page(res),
										tmp_buf);
#endif
						}
					}
				}

			} else {
				if( e_node->i_node == NULL) {
				continue;		
				}
					
				i_node = e_node->i_node;
	
				for (k=0; k<INDEX_NODE_SIZE; k++){
					t_i_node = (leaf_node *)i_node + (j*INDEX_NODE_SIZE) + k;
					if (t_i_node && t_i_node->used){
						l_node = t_i_node->l_node;
						for (l=0; l<LEAF_NODE_SIZE; l++){
							t_l_node = (leaf_node *)l_node + l;
							if (t_l_node && t_l_node->used){
								if (ftl_calc_block(t_l_node->ppn) == vt_blk_num) {
#ifdef VSSIM_GC
									remain_pages = (ftl_calc_block(ftl_metadata->write_ptr)+1)*FLASH_NB*WAY_NB*PAGE_NB-ftl_metadata->write_ptr;
#else
									remain_pages = (ftl_calc_block(ftl_metadata->write_ptr)+1)*NR_PHY_BANKS*NR_PHY_WAYS*PAGES_PER_BLOCK-ftl_metadata->write_ptr;
#endif
									if (t_l_node->size > remain_pages) {
										for(n=0; n<remain_pages; n++){
											res = ftl_get_free_page();
										}
									}
#ifndef VSSIM_GC
									flash_read_gc(ftl_calc_bank(t_l_node->ppn),	
										 		 ftl_calc_block(t_l_node->ppn), 
										 		 ftl_calc_page(t_l_node->ppn), 
										 		 tmp_buf);
									
#endif
									res = ftl_get_free_page();
									t_l_node->ppn= res;
#ifndef VSSIM_GC									
									flash_write_gc(ftl_calc_bank(res),
											ftl_calc_block(res),
											ftl_calc_page(res),
											tmp_buf);				
#endif
									for(o=1; o < t_l_node->size; o++){
#ifndef VSSIM_GC
										flash_read_gc(ftl_calc_bank(t_l_node->ppn+o),	
										 		 ftl_calc_block(t_l_node->ppn+o), 
										 		 ftl_calc_page(t_l_node->ppn+o), 
										 		 tmp_buf);
#endif
										res = ftl_get_free_page();		
#ifndef VSSIM_GC
										flash_write_gc(	ftl_calc_bank(res),
														ftl_calc_block(res),
														ftl_calc_page(res),
														tmp_buf);
#endif
									}
								}
							}
						}		
					}
				}
			}
		}

	//	printf("GC step finish: %d blk: %d\n", step_flag, vt_blk_num);
		if(step_flag != HARD)
			step_flag = 2;
	}
#ifdef HOST_QUEUE
	if(IS_EVENT(e_queue) && step_flag != HARD)
	{
		printf("preempt!\n");
		return 0;
	}
#endif
	if(step_flag == 2 || step_flag == HARD){
	//	printf("GC step start: %d blk: %d \n", step_flag, vt_blk_num);
		for(i=0; i< FLASH_NB;i++){
#ifdef VSSIM_GC
//			flash_erase(i, vt_blk_num);
			SSD_BLOCK_ERASE(i, vt_blk_num);
#endif
		}
		ftl_clear_free_blk_bitmap(vt_blk_num);
		free_page_cnt += FLASH_NB * PAGE_NB;
	//	printf("GC step finish: %d blk: %d \n", step_flag, vt_blk_num);
		if(step_flag != HARD)
			step_flag = 0;
	}

	return 0;
}
