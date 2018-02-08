#ifndef _SSD_WRITE_BUFFER_H_
#define _SSD_WRITE_BUFFER_H_

extern uint32_t MAX_MERGE_SECTORS;

typedef struct write_buffer_entry
{
	int32_t sector_nb;
	unsigned int length;
	struct write_buffer_entry* prev;
	struct write_buffer_entry* next;
}write_buffer_entry;

void INIT_WRITE_BUFFER(void);
void TERM_WRITE_BUFFER(void);

void WRITE_TO_BUFFER(int32_t sector_nb, unsigned int length);
void SECURE_WRITE_BUFFER(unsigned int length);
void INSERT_WRITE_REQUEST(int32_t sector_nb, unsigned int length);

void ALLOC_WRITE_BUFFER_ENTRY(int32_t sector_nb, unsigned int length);

void FLUSH_WRITE_BUFFER(void);
void FLUSH_ALL_WRITE_BUFFER(void);

int CHECK_WRITE_BUFFER_FOR_READ(int32_t sector_nb, unsigned int length);

#endif
