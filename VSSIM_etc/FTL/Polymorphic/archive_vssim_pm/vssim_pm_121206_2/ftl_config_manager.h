#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include "common.h"

//extern FILE * v_FP;

extern int PAGE_SIZE;
extern int PAGE_NB;
extern int SECTOR_SIZE;
extern int FLASH_NB;
extern int BLOCK_NB;
extern int PLANES_PER_FLASH;
extern int PAGES_PER_FLASH;

extern int64_t SECTOR_NB;
extern int64_t PAGE_MAPPING_ENTRY_NB; 		// added by js
extern int64_t BLOCK_MAPPING_ENTRY_NB;		// added by js
extern int64_t EACH_EMPTY_TABLE_ENTRY_NB;	// added by js

extern int EMPTY_TABLE_ENTRY_NB;
extern int VICTIM_TABLE_ENTRY_NB;

extern int DATA_BLOCK_NB;
extern int LOG_RAND_BLOCK_NB;
extern int LOG_SEQ_BLOCK_NB;
extern int LOG_BLOCK_NB;
extern int SECTORS_PER_PAGE;

extern int REG_WRITE_DELAY;
extern int CELL_PROGRAM_DELAY;
extern int REG_READ_DELAY;
extern int CELL_READ_DELAY;
extern int BLOCK_ERASE_DELAY;
extern int CHANNEL_SWITCH_DELAY_R;
extern int CHANNEL_SWITCH_DELAY_W;

extern int DSM_TRIM_ENABLE;
extern int IO_PARALLELISM;

extern double GC_THRESHOLD;			// added by js
extern int GC_THRESHOLD_BLOCK_NB;		// added by js
extern int GC_THRESHOLD_BLOCK_NB_EACH;
extern int GC_VICTIM_NB;

extern unsigned int FTL_BUFFER_SIZE;
extern unsigned int SSD_BUFFER_SIZE;

extern int CHANNEL_NB;
extern int MAPPING_CACHE_SIZE;

#ifdef VSSIM_PM
extern int WAY_NB;

extern int PHY_SPARE_SIZE;

extern int64_t NR_PHY_BLOCKS;
extern int64_t NR_PHY_PAGES;
extern int64_t NR_PHY_SECTORS;

extern int NR_RESERVED_PHY_BLOCK_SIZE;
extern int NR_RESERVED_PHY_BLOCKS;
extern int NR_RESERVED_PHY_PAGES;
#endif

void INIT_SSD_CONFIG(void);
int64_t GET_SECTOR_NB(void);
char* GET_FILE_NAME(void);

#endif
