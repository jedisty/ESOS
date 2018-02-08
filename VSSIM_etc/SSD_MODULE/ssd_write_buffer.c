#include "common.h"

#ifdef SSD_WRITE_BUFFER

write_buffer_entry* write_buffer_list_head;
write_buffer_entry* write_buffer_list_tail;

uint32_t MAX_MERGE_SECTORS;
unsigned int remaining_sectors;
unsigned int write_buffer_entry_nb;

void INIT_WRITE_BUFFER(void)
{
	write_buffer_list_head = NULL;
	write_buffer_list_tail = NULL;

	write_buffer_entry_nb = 0;
	remaining_sectors = SECTORS_PER_WRITE_BUFFER;

	MAX_MERGE_SECTORS = (16*1024*1024)/SECTOR_SIZE;

	if(SECTORS_PER_WRITE_BUFFER < MAX_MERGE_SECTORS){
		MAX_MERGE_SECTORS = SECTORS_PER_WRITE_BUFFER;
	}
}

void TERM_WRITE_BUFFER(void)
{
	FLUSH_ALL_WRITE_BUFFER();
	printf("[%s] Remaining sectors %u, Remaining entry %u \n", __FUNCTION__, remaining_sectors, write_buffer_entry_nb);
}

void WRITE_TO_BUFFER(int32_t sector_nb, unsigned int length)
{
#ifdef WRITE_BUFFER_DEBUG
	printf("[%s] Start, legnth %u, entry_nb %u \n", __FUNCTION__, length, write_buffer_entry_nb);
#endif
	SECURE_WRITE_BUFFER(length);

	if(remaining_sectors < length){
		printf("ERROR[%s] remain sects %d request sects %d\n",__FUNCTION__, remaining_sectors, length);
		return;
	}

	INSERT_WRITE_REQUEST(sector_nb, length);

#ifdef WRITE_BUFFER_DEBUG
	printf("[%s] End \n", __FUNCTION__);
#endif
}

void SECURE_WRITE_BUFFER(unsigned int length)
{
	if(remaining_sectors < length){

		while(remaining_sectors < length){
			FLUSH_WRITE_BUFFER();
		}
	}
}

void INSERT_WRITE_REQUEST(int32_t sector_nb, unsigned int length)
{
	int32_t last_sector_nb = 0;

	if(write_buffer_entry_nb != 0){
		if(write_buffer_list_tail == NULL){
			printf("ERROR[%s] write_buffer_list_tail has NULL pointer !\n", __FUNCTION__);
		}
		last_sector_nb = write_buffer_list_tail->sector_nb + write_buffer_list_tail->length - 1;

		/* If Sequential Write Request */
		if( (last_sector_nb+1) == sector_nb && \
				(write_buffer_list_tail->length + length <= MAX_MERGE_SECTORS) )
		{

			/* Merge Write request */
			write_buffer_list_tail->length += length;

			/* Update Global Variable */
			remaining_sectors -= length;
		}
		else{
			/* Alloc New Entry */
			ALLOC_WRITE_BUFFER_ENTRY(sector_nb, length);
		}
	}
	else{
		/* Alloc New Entry */
		ALLOC_WRITE_BUFFER_ENTRY(sector_nb, length);
	}
}

void ALLOC_WRITE_BUFFER_ENTRY(int32_t sector_nb, unsigned int length)
{
	/* Alloc New Entry */
	write_buffer_entry* new_entry = (write_buffer_entry*)malloc(sizeof(struct write_buffer_entry));
	if(new_entry == NULL){
		printf("ERROR[%s] Calloc new entry fail\n", __FUNCTION__);
		return;
	}

	/* Init New Entry */
	new_entry->sector_nb = sector_nb;
	new_entry->length = length;
	new_entry->prev = NULL;
	new_entry->next = NULL;

	/* Rearrange buffer list */
	if(write_buffer_entry_nb != 0){
		write_buffer_list_tail->next = new_entry;
		new_entry->prev = write_buffer_list_tail;
		write_buffer_list_tail = new_entry;
	}
	else{
		write_buffer_list_head = new_entry;
		write_buffer_list_tail = new_entry;
	}

	/* Update Global Variable */
	write_buffer_entry_nb++;
	remaining_sectors -= length;
}

void FLUSH_WRITE_BUFFER(void)
{
	write_buffer_entry* curr_entry = write_buffer_list_head;
	int32_t sector_nb;
	unsigned int length;

	if(write_buffer_entry_nb != 0 && curr_entry != NULL){

		sector_nb = curr_entry->sector_nb;
		length = curr_entry->length;
		
		/* Write curr entry to FLASH */
#ifdef PMM_FTL
		FTL_WRITE((uint32_t)sector_nb, (uint32_t)length);
#else
		FTL_WRITE(sector_nb, length);
#endif

		/* Update Write buffer list */
		if(write_buffer_entry_nb == 1){
			write_buffer_list_head = NULL;
			write_buffer_list_tail = NULL;	
		}
		else{
			curr_entry->next->prev = NULL;
			write_buffer_list_head = curr_entry->next;
		}

		/* Free curr entry */
		free(curr_entry);

		write_buffer_entry_nb--;
		remaining_sectors += length;
	}
	else{
		printf("ERROR[%s] There is no buffered entry\n", __FUNCTION__);
		return;	
	}
}

void FLUSH_ALL_WRITE_BUFFER(void)
{
	write_buffer_entry* curr_entry = write_buffer_list_head;
	write_buffer_entry* temp_entry;
	int32_t sector_nb;
	unsigned int length;
	int i;
	
	for(i=0;i<write_buffer_entry_nb; i++){
		sector_nb = curr_entry->sector_nb;
		length = curr_entry->length;

		/* Write curr entry to FLASH */
#ifdef PMM_FTL
		FTL_WRITE((uint32_t)sector_nb, (uint32_t)length);
#else
		FTL_WRITE(sector_nb, length);
#endif
		remaining_sectors += length;
		temp_entry = curr_entry;

		/* Free curr entry */
		free(temp_entry);
		
		curr_entry = curr_entry->next;
		
	}
	write_buffer_entry_nb = 0;
	write_buffer_list_head = NULL;
	write_buffer_list_tail = NULL;
}

void CHECK_WRITE_BUFFER_FOR_READ(int32_t sector_nb, unsigned int length)
{
#ifdef WRITE_BUFFER_DEBUG
	printf("[%s] Start! sector_nb %d, lnegth %u\n",__FUNCTION__, sector_nb, length);
#endif
	write_buffer_entry* curr_entry = write_buffer_list_head;
	write_buffer_entry* temp_entry = NULL;

	int32_t curr_sect_nb;
	int32_t curr_last_nb;

	int32_t read_sect_nb = sector_nb;
	int32_t read_last_nb = sector_nb + length - 1;

	int ret = FAIL;

	while(curr_entry != NULL){
		
		curr_sect_nb = curr_entry->sector_nb;
		curr_last_nb = curr_sect_nb + curr_entry->length - 1;

		/* If write buffer hit */
		if(curr_sect_nb <= read_last_nb && read_sect_nb <= curr_last_nb){

#ifdef PMM_FTL
			FTL_WRITE((uint32_t)(curr_entry->sector_nb), (uint32_t)(curr_entry->length));
#else
			FTL_WRITE(curr_entry->sector_nb, curr_entry->length);
#endif
			temp_entry = curr_entry;

			if(curr_entry == write_buffer_list_head && curr_entry == write_buffer_list_tail)
			{
				write_buffer_list_head = NULL;
				write_buffer_list_tail = NULL;
				curr_entry = NULL;
			}
			else if(curr_entry == write_buffer_list_head)
			{
				curr_entry->next->prev = NULL;
				write_buffer_list_head = curr_entry->next;
				curr_entry = write_buffer_list_head;
			}
			else if(curr_entry == write_buffer_list_tail)
			{
				curr_entry->prev->next = NULL;
				write_buffer_list_tail = curr_entry->prev;
				curr_entry = NULL;
			}
			else
			{
				curr_entry->prev->next = curr_entry->next;
				curr_entry->next->prev = curr_entry->prev;
				curr_entry = curr_entry->next;
			}
			write_buffer_entry_nb--;
			remaining_sectors += temp_entry->length;
			free(temp_entry);
		}
		else{
			curr_entry = curr_entry->next;
		}
	}
#ifdef WRITE_BUFFER_DEBUG
	printf("[%s] end\n",__FUNCTION__);
#endif
}

#endif
