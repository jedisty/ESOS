#ifndef _VSSIM_CACHE_H_
#define _VSSIM_CACHE_H_

extern struct pm_map* vssim_pm_mapping_start;
extern struct pm_map* vssim_pm_mapping_end;
extern uint32_t vssim_pm_mapping_entry_nb; 
typedef struct pm_map
{
	uint32_t ppn;
	void* map_data;
	struct pm_map* prev;
	struct pm_map* next;
}pm_map;

void INIT_VSSIM_PM_MAPPING(void);

int WRITE_MAP(uint32_t page_nb, void* buf);
void* READ_MAP(uint32_t page_nb);
pm_map* LOOKUP_PM_MAP_ENTRY(uint32_t page_nb);
int REARRANGE_PM_MAP_ENTRY(struct pm_map* new_entry);
#endif
