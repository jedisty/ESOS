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

int WRITE_MAP(uint32_t flash_nb, uint32_t block_nb, uint32_t page_nb, void* buf)
{
	pm_map* curr_pm_map = LOOKUP_PM_MAP_ENTRY(flash_nb, block_nb, page_nb);

	if(curr_pm_map != NULL){
		memcpy(curr_pm_map->map_data, buf, PAGE_SIZE);
	}
	else{
		curr_pm_map = (pm_map*)calloc(1, sizeof(pm_map));
		curr_pm_map->map_data = (void*)calloc(1, PAGE_SIZE);

		curr_pm_map->phy_flash_nb = flash_nb;
		curr_pm_map->phy_block_nb = block_nb;
		curr_pm_map->phy_page_nb = page_nb;
		curr_pm_map->prev = NULL;
		curr_pm_map->next = NULL;

		memcpy(curr_pm_map->map_data, buf, PAGE_SIZE);

		vssim_pm_mapping_entry_nb++;
	}

	REARRANGE_PM_MAP_ENTRY(curr_pm_map);

	return SUCCESS;
}

int READ_MAP(uint32_t flash_nb, uint32_t block_nb, uint32_t page_nb, void* buf)
{
	pm_map* curr_pm_map = LOOKUP_PM_MAP_ENTRY(flash_nb, block_nb, page_nb);

	if(curr_pm_map == NULL){
		printf("ERROR[READ_MAP] There is no such map \n");
		return FAIL;
	}
	else{
		memcpy(buf, curr_pm_map->map_data, PAGE_SIZE);
		return SUCCESS;
	}
}

pm_map* LOOKUP_PM_MAP_ENTRY(uint32_t flash_nb, uint32_t block_nb, uint32_t page_nb)
{
	pm_map* curr_pm_map = vssim_pm_mapping_start;
	int i;

	if(vssim_pm_mapping_entry_nb > 0 && curr_pm_map == NULL)
		printf("ERROR[LOOKUP_PM_MAP_ENTRY] %d \n", vssim_pm_mapping_entry_nb);

	for(i=0; i<vssim_pm_mapping_entry_nb; i++){
		if( curr_pm_map->phy_flash_nb == flash_nb \
				&& curr_pm_map->phy_block_nb == block_nb \
				&& curr_pm_map->phy_page_nb == page_nb){
			return curr_pm_map;
		}
		else{
			if(curr_pm_map->next != NULL){
				curr_pm_map = curr_pm_map->next;
			}
			else{
				break;
			}
		}
	}
	return NULL;
}

int REARRANGE_PM_MAP_ENTRY(struct pm_map* new_entry)
{
	if(new_entry->prev == NULL && new_entry->next == NULL){
		if(vssim_pm_mapping_start == NULL){
			vssim_pm_mapping_start = new_entry;
			vssim_pm_mapping_end = new_entry;
		}
		else{
			new_entry->next = vssim_pm_mapping_start;
			vssim_pm_mapping_start->prev = new_entry;
			vssim_pm_mapping_start = new_entry;
		}
	}
	else{
		if(new_entry == vssim_pm_mapping_start){
			return SUCCESS;
		}
		else if(new_entry == vssim_pm_mapping_end){
			vssim_pm_mapping_end = new_entry->prev;
			new_entry->prev->next = NULL;
			new_entry->prev = NULL;
			vssim_pm_mapping_start->prev = new_entry;
			new_entry->next = vssim_pm_mapping_start;
			vssim_pm_mapping_start = new_entry;
		}
		else{
			if(vssim_pm_mapping_entry_nb < 3){
				printf("ERROR[REARRANGE_PM_MAP_ENTRY]\n");
			}
			new_entry->prev->next = new_entry->next;
			new_entry->next->prev = new_entry->prev;

			vssim_pm_mapping_start->prev = new_entry;
			new_entry->next = vssim_pm_mapping_start;
			vssim_pm_mapping_start = new_entry;
		}
	}
	return SUCCESS;
}
