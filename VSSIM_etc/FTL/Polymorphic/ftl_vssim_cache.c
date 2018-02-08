#include "common.h"

struct pm_map* vssim_pm_mapping_start;
struct pm_map* vssim_pm_mapping_end;

uint32_t vssim_pm_mapping_entry_nb;

void INIT_VSSIM_PM_MAPPING(void){
	FILE* fp = fopen("./data/vssim_pm_mapping.dat","r");
	if(fp != NULL){
// TEMP
	}
	else{
		vssim_pm_mapping_start = NULL;
		vssim_pm_mapping_end = NULL;
		vssim_pm_mapping_entry_nb = 0;
	}
}

int WRITE_MAP(uint32_t page_nb, void* buf)
{
//	pm_map* curr_pm_map;
	pm_map* curr_pm_map = LOOKUP_PM_MAP_ENTRY(page_nb);

	if(curr_pm_map != NULL){
//		memcpy(curr_pm_map->map_data, buf, PAGE_SIZE);
//		printf("ERROR[WRITE_MAP] hit?\n");
	}
	else{
		curr_pm_map = (pm_map*)calloc(1, sizeof(pm_map));
		curr_pm_map->map_data = (void*)calloc(1, PAGE_SIZE);

		curr_pm_map->ppn = page_nb;
		curr_pm_map->prev = NULL;
		curr_pm_map->next = NULL;

		memcpy(curr_pm_map->map_data, buf, PAGE_SIZE);

		vssim_pm_mapping_entry_nb++;
	
		REARRANGE_PM_MAP_ENTRY(curr_pm_map);
	}

	return SUCCESS;
}

void* READ_MAP(uint32_t page_nb)
{
	pm_map* curr_pm_map = LOOKUP_PM_MAP_ENTRY(page_nb);

	if(curr_pm_map == NULL){
		printf("ERROR[READ_MAP] There is no such map \n");
		return NULL;
	}
	else{
		return curr_pm_map->map_data;
	}
}

pm_map* LOOKUP_PM_MAP_ENTRY(uint32_t page_nb)
{
	pm_map* curr_pm_map = vssim_pm_mapping_start;
	int i;

	if(vssim_pm_mapping_entry_nb > 0 && curr_pm_map == NULL)
		printf("ERROR[LOOKUP_PM_MAP_ENTRY] %d \n", vssim_pm_mapping_entry_nb);
//BUG
	for(i=0; i<vssim_pm_mapping_entry_nb; i++){
		if( curr_pm_map->ppn == page_nb){
			return curr_pm_map;
		}
		else{
			curr_pm_map = curr_pm_map->next;
		}
	}
	return NULL;
}

int REARRANGE_PM_MAP_ENTRY(struct pm_map* new_entry)
{
	if(vssim_pm_mapping_start == NULL){
		vssim_pm_mapping_start = new_entry;
		vssim_pm_mapping_end = new_entry;
	}
	else{
		vssim_pm_mapping_end->next = new_entry;
		new_entry->prev = vssim_pm_mapping_end;
		vssim_pm_mapping_end = new_entry;
	}

	return SUCCESS;
}
