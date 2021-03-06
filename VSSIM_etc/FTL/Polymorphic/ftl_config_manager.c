#include <stdlib.h>
#include "ftl_config_manager.h"


#ifdef VSSIM_PM
int WAY_NB;

int PHY_SPARE_SIZE;

int64_t NR_PHY_BLOCKS;
int64_t NR_PHY_PAGES;
int64_t NR_PHY_SECTORS;

int NR_RESERVED_PHY_SUPER_BLOCKS;
int NR_RESERVED_PHY_BLOCKS;
int NR_RESERVED_PHY_PAGES;

int CACHE_IDX_SIZE;
#endif

char gFile_Name[1024] = {0,};

void INIT_SSD_CONFIG(void)
{
	FILE* pfData;
	pfData = fopen("./data/ssd.conf", "r");
	
	char* szCommand = NULL;
	uint32_t s_szCommand = 1024 * sizeof(char);

	szCommand = (char*)malloc(s_szCommand);
	memset(szCommand, 0x00, s_szCommand);

	if(pfData!=NULL)
	{
		while(fscanf(pfData, "%s", szCommand)!=EOF)
		{
			if(strcmp(szCommand, "FILE_NAME") == 0)
			{
				fscanf(pfData, "%s", gFile_Name);
			}
			else if(strcmp(szCommand, "PAGE_SIZE") == 0)
			{
				fscanf(pfData, "%d", &PAGE_SIZE);
			}
			else if(strcmp(szCommand, "PAGE_NB") == 0)
			{
				fscanf(pfData, "%d", &PAGE_NB);
			}
			else if(strcmp(szCommand, "SECTOR_SIZE") == 0)
			{
				fscanf(pfData, "%d", &SECTOR_SIZE);
			}	
			else if(strcmp(szCommand, "FLASH_NB") == 0)
			{
				fscanf(pfData, "%d", &FLASH_NB);
			}	
			else if(strcmp(szCommand, "BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &BLOCK_NB);
			}					
			else if(strcmp(szCommand, "PLANES_PER_FLASH") == 0)
			{
				fscanf(pfData, "%d", &PLANES_PER_FLASH);
			}
			else if(strcmp(szCommand, "LOG_RAND_BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &LOG_RAND_BLOCK_NB);
			}	
			else if(strcmp(szCommand, "LOG_SEQ_BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &LOG_SEQ_BLOCK_NB);
			}	
			else if(strcmp(szCommand, "LOG_BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &LOG_BLOCK_NB);
			}				
			else if(strcmp(szCommand, "REG_WRITE_DELAY") == 0)
			{
				fscanf(pfData, "%d", &REG_WRITE_DELAY);
			}	
			else if(strcmp(szCommand, "CELL_PROGRAM_DELAY") == 0)
			{
				fscanf(pfData, "%d", &CELL_PROGRAM_DELAY);
			}
			else if(strcmp(szCommand, "REG_READ_DELAY") == 0)
			{
				fscanf(pfData, "%d", &REG_READ_DELAY);
			}
			else if(strcmp(szCommand, "CELL_READ_DELAY") == 0)
			{
				fscanf(pfData, "%d", &CELL_READ_DELAY);
			}
			else if(strcmp(szCommand, "BLOCK_ERASE_DELAY") == 0)
			{
				fscanf(pfData, "%d", &BLOCK_ERASE_DELAY);
			}
			else if(strcmp(szCommand, "CHANNEL_SWITCH_DELAY_R") == 0)
			{
				fscanf(pfData, "%d", &CHANNEL_SWITCH_DELAY_R);
			}
			else if(strcmp(szCommand, "CHANNEL_SWITCH_DELAY_W") == 0)
			{
				fscanf(pfData, "%d", &CHANNEL_SWITCH_DELAY_W);
			}
			else if(strcmp(szCommand, "CACHE_IDX_SIZE") == 0)
			{
				fscanf(pfData, "%d", &CACHE_IDX_SIZE);
			}
			else if(strcmp(szCommand, "DSM_TRIM_ENABLE") == 0)
			{
				fscanf(pfData, "%d", &DSM_TRIM_ENABLE);
			}
			else if(strcmp(szCommand, "IO_PARALLELISM") == 0)
			{
				fscanf(pfData, "%d", &IO_PARALLELISM);
			}
			else if(strcmp(szCommand, "WRITE_BUFFER_SIZE") == 0)
			{
				fscanf(pfData, "%u", &WRITE_BUFFER_SIZE);
			}
			else if(strcmp(szCommand, "CHANNEL_NB") == 0)
			{
				fscanf(pfData, "%d", &CHANNEL_NB);
			}
			else if(strcmp(szCommand, "MAPPING_CACHE_SIZE") == 0)
			{
				fscanf(pfData, "%d", &MAPPING_CACHE_SIZE);
			}
			memset(szCommand, 0x00, 1024);
		}	
		fclose(pfData);

	}
	DATA_BLOCK_NB = BLOCK_NB - LOG_RAND_BLOCK_NB - LOG_SEQ_BLOCK_NB;
	SECTORS_PER_PAGE = PAGE_SIZE / SECTOR_SIZE;
	PAGES_PER_FLASH = PAGE_NB * BLOCK_NB;
	
	SECTOR_NB = (int64_t)SECTORS_PER_PAGE * (int64_t)PAGE_NB * (int64_t)BLOCK_NB * (int64_t)FLASH_NB;
	PAGE_MAPPING_ENTRY_NB = (int64_t)PAGE_NB * (int64_t)BLOCK_NB * (int64_t)FLASH_NB;
	BLOCK_MAPPING_ENTRY_NB = (int64_t)BLOCK_NB * (int64_t)FLASH_NB;

	EACH_EMPTY_TABLE_ENTRY_NB = (int64_t)BLOCK_NB / (int64_t)PLANES_PER_FLASH;

	EMPTY_TABLE_ENTRY_NB = FLASH_NB * PLANES_PER_FLASH;
	VICTIM_TABLE_ENTRY_NB = FLASH_NB * PLANES_PER_FLASH;

	/* Write Buffer */
	SECTORS_PER_WRITE_BUFFER = (WRITE_BUFFER_SIZE * 1024)/SECTOR_SIZE;

	GC_THRESHOLD = 0.7; // 70%
	GC_THRESHOLD_BLOCK_NB = (int)((1-GC_THRESHOLD) * (double)BLOCK_MAPPING_ENTRY_NB);
	GC_THRESHOLD_BLOCK_NB_EACH = (int)((1-GC_THRESHOLD) * (double)EACH_EMPTY_TABLE_ENTRY_NB);
	GC_VICTIM_NB = 10;

#ifdef VSSIM_PM
//	WAY_NB = FLASH_NB / CHANNEL_NB;
	WAY_NB = 1;

	PHY_SPARE_SIZE = 436;

	NR_PHY_BLOCKS = FLASH_NB * WAY_NB * BLOCK_NB;
	NR_PHY_PAGES = NR_PHY_BLOCKS * PAGE_NB;
	NR_PHY_SECTORS = NR_PHY_PAGES * SECTORS_PER_PAGE;

	NR_RESERVED_PHY_SUPER_BLOCKS = 4;
	NR_RESERVED_PHY_BLOCKS = FLASH_NB * WAY_NB * NR_RESERVED_PHY_SUPER_BLOCKS;
	NR_RESERVED_PHY_PAGES = NR_RESERVED_PHY_BLOCKS * PAGE_NB;
#endif

	free(szCommand);
}

char* GET_FILE_NAME(void){
	return gFile_Name;
}
