#ifndef __GC_H
#define __GC_H

#include "common.h"

#define HARD 100

typedef struct _super_block_meta{
	int valid_cnt;
}sblock_meta;


extern void gc_print_valid(void);
extern void gc_init(void);
extern void gc_valid_count(void);
extern void print_mapping_table(void);
extern uint32_t gc_victim_block_select(void);
extern int garbage_collection(void);
#endif
