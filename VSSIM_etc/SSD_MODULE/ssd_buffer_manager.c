// Copyright(c)2013 
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#include "common.h"

event_queue* e_queue;
event_queue* c_e_queue;

/* Global Variable for IO Buffer */
void* write_buffer;
void* read_buffer;
void* write_buffer_end;
void* read_buffer_end;

/* Global Variable for FTL Buffer */
void* ftl_buffer;
void* ftl_buffer_end;
event_queue_entry ftl_buffer_entry;
void* curr_ftl_buffer_entry;
unsigned int ftl_buffer_remain_entry;

/* Globale Variable for Valid array */
char* wb_valid_array;

/* Pointers for Write Buffer */
void* ftl_write_ptr;
void* sata_write_ptr;
void* write_limit_ptr;

/* Pointers for Read Buffer */
void* ftl_read_ptr;
void* sata_read_ptr;
void* read_limit_ptr;
event_queue_entry* last_read_entry;

int empty_write_buffer_frame;
int empty_read_buffer_frame;

int flag_seq_write;

//TEMPs
FILE* fp_buf;
int seq_tail_hit = 0;
int dep_tail_hit = 0;
int overwrite_hit = 0;
int64_t overwrite_hit_sectors = 0;
//TEMPe

void INIT_IO_BUFFER(void)
{
	/* Allocation event queue structure */
	e_queue = (event_queue*)calloc(1, sizeof(event_queue));
	if(e_queue == NULL){
		printf("[%s] Allocation event queue fail.\n",__FUNCTION__);
		return;
	}

	/* Initialization event queue structure */
	e_queue->entry_nb = 0;
	e_queue->head = NULL;
	e_queue->tail = NULL;

	/* Initialization valid array of event queue */
	INIT_WB_VALID_ARRAY();

	/* Allocation completed event queue structure */
	c_e_queue = (event_queue*)calloc(1, sizeof(event_queue));
	if(c_e_queue == NULL){
		printf("[%s] Allocation completed event queue fail.\n",__FUNCTION__);
		return;
	}

	/* Initialization event queue structure */
	c_e_queue->entry_nb = 0;
	c_e_queue->head = NULL;
	c_e_queue->tail = NULL;

	/* Allocation Write Buffer in DRAM */
	write_buffer = (void*)calloc(WRITE_BUFFER_FRAME_NB, SECTOR_SIZE);
	write_buffer_end = write_buffer + WRITE_BUFFER_FRAME_NB*SECTOR_SIZE;	

	/* Allocation Read Buffer in DRAM */
	read_buffer = (void*)calloc(READ_BUFFER_FRAME_NB, SECTOR_SIZE);
	read_buffer_end = read_buffer + READ_BUFFER_FRAME_NB*SECTOR_SIZE;	

	if(write_buffer == NULL || read_buffer == NULL){
		printf("[%s] Allocation IO Buffer Fail.\n",__FUNCTION__);
		return;
	}
	
	/* Initialization Buffer Pointers */
	ftl_write_ptr = write_buffer;
	sata_write_ptr = write_buffer;
	write_limit_ptr = write_buffer;

	ftl_read_ptr = read_buffer;
	sata_read_ptr = read_buffer;
	read_limit_ptr = read_buffer;
	last_read_entry = NULL;

	/* Initialization Other Global Variable */
	empty_write_buffer_frame = WRITE_BUFFER_FRAME_NB;
	empty_read_buffer_frame = READ_BUFFER_FRAME_NB; 
	flag_seq_write = 0;

	/* INIT FTL BUFFER */
	INIT_FTL_BUFFER();
}

void INIT_FTL_BUFFER(void)
{	
	/* Allocation FTL Buffer in DRAM */
	ftl_buffer = (void*)calloc(FTL_BUFFER_FRAME_NB, SECTOR_SIZE);
	ftl_buffer_end = ftl_buffer + FTL_BUFFER_FRAME_NB*SECTOR_SIZE;

	if(ftl_buffer == NULL){
		printf("[%s] \n", __FUNCTION__);
		return;
	}

	ftl_buffer_entry.valid = INVALID;
	curr_ftl_buffer_entry = ftl_buffer;
	ftl_buffer_remain_entry = FTL_BUFFER_FRAME_NB;
}

void TERM_IO_BUFFER(void)
{
	/* Flush all event in event queue */
	FLUSH_EVENT_QUEUE_UNTIL(e_queue->tail);

	/* Terminate FTL Buffer */
	TERM_FTL_BUFFER();

//TEMPs
	printf("seq tail hit  : %d\n", seq_tail_hit);
	printf("dep tail hit  : %d\n", dep_tail_hit);
	printf("overwrite hit : %d\n", overwrite_hit);
	printf("overwrite_hit sectors : %ld\n", overwrite_hit_sectors);
//TEMPe
}

void TERM_FTL_BUFFER(void)
{
	FLUSH_FTL_BUFFER();
}

void INIT_WB_VALID_ARRAY(void)
{
	int i;
	wb_valid_array = (char*)calloc(WRITE_BUFFER_FRAME_NB , sizeof(char));
	if(wb_valid_array == NULL){
		printf("[%s] Calloc write buffer valid array fail. \n",__FUNCTION__);
		return;
	}

	for(i=0;i<WRITE_BUFFER_FRAME_NB;i++){
		wb_valid_array[i] = '0';
	}
}

void ENQUEUE_IO(int io_type, int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif

	if(e_queue->entry_nb == EVENT_QUEUE_DEPTH){
		FLUSH_EVENT_QUEUE_UNTIL(e_queue->tail);
	}

	if(io_type == READ){
		ENQUEUE_READ(sector_nb, length);
	}
	else if(io_type == WRITE){
		ENQUEUE_WRITE(sector_nb, length);
	}
	else{
		printf("ERROR[%s] Wrong IO type.\n", __FUNCTION__);
	}
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

void DEQUEUE_IO(void)
{
	if(e_queue->entry_nb == 0){
		printf("ERROR[%s] There is no event. \n", __FUNCTION__);
		return;
	}

	char valid;
	event_queue_entry* e_q_entry = e_queue->head;
	
	if(e_q_entry->valid == VALID){		

		/* Call FTL Function */
		if(e_q_entry->io_type == READ){

			if(e_q_entry->buf != NULL){
				/* The data is already read from write buffer */
			}
			else{
				/* Secure read buffer frame */
				SECURE_READ_BUFFER(e_q_entry->length);

				/* Allocate read pointer */
				e_q_entry->buf = ftl_read_ptr;

				FTL_READ(e_q_entry->sector_nb, e_q_entry->length);
			}
		}
		else if(e_q_entry->io_type == WRITE){
			valid = GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf);

			/* If the write event is sequential write, */
			if(valid == 'S' || valid == 'E'){
				MOVE_EVENT_TO_FTL_BUFFER(e_q_entry);
			}
			else{
				FTL_WRITE(e_q_entry->sector_nb, e_q_entry->length);
			}
		}
		else{
			printf("ERROR[%s] Invalid IO type. \n", __FUNCTION__);
		}
	}

	/* Deallocation event queue entry */
	e_queue->entry_nb--;
	if(e_queue->entry_nb == 0){
		e_queue->head = NULL;
		e_queue->tail = NULL;
	}
	else{
		e_queue->head = e_q_entry->next;
	}

	if(e_q_entry->io_type == WRITE){
		free(e_q_entry);
	}
	else{
		if(e_q_entry == last_read_entry){
			last_read_entry = NULL;
		}
		/* Move event queue entry to completed event queue */
		e_q_entry->next = NULL;
		if(c_e_queue->entry_nb == 0){
			c_e_queue->head = e_q_entry;
			c_e_queue->tail = e_q_entry;
		}
		else{
			c_e_queue->tail->next = e_q_entry;
			c_e_queue->tail = e_q_entry;
		}
		c_e_queue->entry_nb++;
	}
}

void DEQUEUE_COMPLETED_READ(void)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	if(c_e_queue->entry_nb == 0){
#ifdef SSD_IO_BUF_DEBUG
		printf("[%s] There is no completed read event. \n",__FUNCTION__);
#endif
		return;
	}

	event_queue_entry* c_e_q_entry = c_e_queue->head;
	event_queue_entry* temp_c_e_q_entry = NULL;

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] entry number %d\n",__FUNCTION__, c_e_queue->entry_nb);
#endif

	while(c_e_q_entry != NULL){

		/* Read data from buffer to host */
		READ_DATA_FROM_BUFFER_TO_HOST(c_e_q_entry);

		/* Remove completed read IO from queue */
		temp_c_e_q_entry = c_e_q_entry;
		c_e_q_entry = c_e_q_entry->next;

		/* Update completed event queue data */
		c_e_queue->entry_nb--;

		/* Deallication completed read IO */
		free(temp_c_e_q_entry);
	}

	if(c_e_queue->entry_nb != 0){
		printf("ERROR[%s] The entry number should be 0.\n",__FUNCTION__);
	}
	else{
		c_e_queue->head = NULL;
		c_e_queue->tail = NULL;
	}

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

void ENQUEUE_READ(int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	void* p_buf = NULL;
	event_queue_entry* ret_e_q_entry = NULL;
	event_queue_entry* new_e_q_entry = NULL;

	int32_t temp_sector_nb;
	int32_t temp_length;

	new_e_q_entry = ALLOC_NEW_EVENT(READ, sector_nb, length, p_buf);

	if(e_queue->entry_nb == 0){
		e_queue->head = new_e_q_entry;
		e_queue->tail = new_e_q_entry;
	}
	else{
		ret_e_q_entry = CHECK_IO_DEPENDENCY_FOR_READ(sector_nb, length);

		if(ret_e_q_entry != NULL){

			temp_sector_nb = ret_e_q_entry->sector_nb;
			temp_length = ret_e_q_entry->length;

			/* If the data can be read from write buffer, */
			if(temp_sector_nb <= sector_nb && \
				(sector_nb + length) <= (temp_sector_nb + temp_length)){
				COPY_DATA_TO_READ_BUFFER(new_e_q_entry, ret_e_q_entry);	
			}

			FLUSH_EVENT_QUEUE_UNTIL(ret_e_q_entry);
			ret_e_q_entry = NULL;
		}	

		/* If there is no read event */
		if(last_read_entry == NULL){
			if(e_queue->entry_nb == 0){
				e_queue->head = new_e_q_entry;
				e_queue->tail = new_e_q_entry;
			}
			else{
				new_e_q_entry->next = e_queue->head;
				e_queue->head = new_e_q_entry;
			}
		}
		else{
			if(last_read_entry == e_queue->tail){
				e_queue->tail->next = new_e_q_entry;
				e_queue->tail = new_e_q_entry;
			}
			else{
				new_e_q_entry->next = last_read_entry->next;
				last_read_entry->next = new_e_q_entry;
			}
		}
		last_read_entry = new_e_q_entry;
	}
	e_queue->entry_nb++;

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

void ENQUEUE_WRITE(int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	event_queue_entry* e_q_entry;
	event_queue_entry* new_e_q_entry = NULL;

	void* p_buf = NULL;
	int remain;
	int invalid_len;

	int flag_allocated = 0;
	int flag_new_seq_event = 0;

	void* temp_buf = NULL;
	int32_t temp_sector_nb;
	unsigned int temp_length;	

	/* Secure read buffer frame */
	SECURE_WRITE_BUFFER(length);

	/* Write SATA data to write buffer */
	p_buf = WRITE_DATA_TO_BUFFER(length);
//TEMPs
/*
	int skip_entry_nb = 0;
	if(BUFFER_LOOKUP_WINDOW != 0){
		e_q_entry = e_queue->head;
		if(e_queue->entry_nb > BUFFER_LOOKUP_WINDOW){
			skip_entry_nb = e_queue->entry_nb - BUFFER_LOOKUP_WINDOW;
		}
		else{
			skip_entry_nb = 0;
		}
	}
	else{
*/
//TEMPe
		if(last_read_entry != NULL){
			if(last_read_entry->next != NULL){
				e_q_entry = last_read_entry->next;
			}
			else{
				e_q_entry = NULL;
			}
		}
		else{
			e_q_entry = e_queue->head;
		}
//TEMPs
//	}
//TEMPe

	/* Check pending write event */
	while(e_q_entry != NULL){
//TEMPs
/*
		if(skip_entry_nb != 0){
			e_q_entry = e_q_entry->next;
			skip_entry_nb--;
			continue;
		}
*/
//TEMPe
		/* If the event is INVALID, then skip. */
		if(e_q_entry->valid != VALID){
			e_q_entry = e_q_entry->next;
			continue;
		}

		/* Check if there is overwrited event */
		if(CHECK_OVERWRITE(e_q_entry, sector_nb, length)==SUCCESS){
			e_q_entry = e_q_entry->next;
			continue;
		}
		/* Check if the event is prior sequential event */
		else if(CHECK_SEQUENTIALITY(e_q_entry, sector_nb)==SUCCESS){
			if(e_q_entry == e_queue->tail){
				/* Update the last write event */
				e_q_entry->length += length;
//TEMPs
				seq_tail_hit++;
//TEMPE

				/* Do not need to allocate new event */
				flag_allocated = 1;
				break;
			}
			else if(flag_seq_write == 0 && \
					(GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf)=='V'))
			{
				/* Marking validity of the event by 'S' */
				UPDATE_WB_VALID_ARRAY(e_q_entry, 'S');	

				/* Set the flag_seq_write flag */
				flag_seq_write = 1;

				/* Set the new sequential write event flag */
				flag_new_seq_event = 1;
				break;
			}
			else if(flag_seq_write == 1 && \
					(GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf)=='E'))
			{
				/* marking validity of the event by 'S' */
				UPDATE_WB_VALID_ARRAY(e_q_entry, 'S');
				
				/* Set the new sequential write event flag */
				flag_new_seq_event = 1;
				break;
			}
		}
		else if(CHECK_IO_DEPENDENCY_FOR_WRITE(e_q_entry, sector_nb, length)==SUCCESS){
				
			temp_sector_nb = e_q_entry->sector_nb;
			temp_length = e_q_entry->length;
			temp_buf = e_q_entry->buf;

			if(e_q_entry == e_queue->tail){
			
				/* Calculate Overlapped length */
				invalid_len = temp_sector_nb + temp_length - sector_nb;

				/* Invalidate the corresponding write buffer frame */
				UPDATE_WB_VALID_ARRAY_PARTIAL(e_q_entry, 'I', invalid_len, 1);

				/* Update the last write event */
				e_q_entry->length += (length - invalid_len);			
//TEMPs
				dep_tail_hit++;
//TEMPe
				/* Do not need to allocate new event */
				flag_allocated = 1;
				break;
			}
			else if(flag_seq_write == 0 && \
					 (GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf)=='V')){
				
				/* Calculate Overlapped length */
				invalid_len = temp_sector_nb + temp_length - sector_nb;
				remain = temp_length - invalid_len;

				/* Update valid array */
				UPDATE_WB_VALID_ARRAY_PARTIAL(e_q_entry, 'S', remain, 0);
				UPDATE_WB_VALID_ARRAY_PARTIAL(e_q_entry, 'I', invalid_len, 1);
				
				/* Update the seq write event */
				e_q_entry->length = remain;

				/* Set the flag_seq_write flag */
				flag_seq_write = 1;
				
				/* Set the new sequential write event flag */
				flag_new_seq_event = 1;
			}
			else if(flag_seq_write == 1 && \
					(GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf)=='E')){
				
				/* Calculate Overlapped length */
				invalid_len = temp_sector_nb + e_q_entry->length - sector_nb;
				remain = temp_length - invalid_len;

				/* Update valid array */
				UPDATE_WB_VALID_ARRAY_PARTIAL(e_q_entry, 'S', remain, 0);
				UPDATE_WB_VALID_ARRAY_PARTIAL(e_q_entry, 'I', invalid_len, 1);

				/* Update the seq write event */
				e_q_entry->length = remain;

				/* Set the new sequential write event flag */
				flag_new_seq_event = 1;
			}
		}
		e_q_entry = e_q_entry->next;
	}

	/* If need to allocate new event */
	if(flag_allocated == 0){
		/* Allocate new event at the tail of the event queue */
		new_e_q_entry = ALLOC_NEW_EVENT(WRITE, sector_nb, length, p_buf);	

		/* Add New IO event entry to event queue */
		if(e_queue->entry_nb == 0){
			e_queue->head = new_e_q_entry;
			e_queue->tail = new_e_q_entry;
		}
		else{
			e_queue->tail->next = new_e_q_entry;
			e_queue->tail = new_e_q_entry;
		}
		e_queue->entry_nb++;
	}

	if(flag_new_seq_event == 1){
		/* marking validity of the event by 'E' */
		UPDATE_WB_VALID_ARRAY(new_e_q_entry, 'E');
	}
//TEMPs
	fp_buf = fopen("./data/p_buf.txt","a");
	fprintf(fp_buf,"%d\t%d\t%d\t%ld\n",seq_tail_hit, dep_tail_hit, overwrite_hit, overwrite_hit_sectors);
	fclose(fp_buf);
//TEMPE
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

event_queue_entry* ALLOC_NEW_EVENT(int io_type, int32_t sector_nb, unsigned int length, void* buf)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	event_queue_entry* new_e_q_entry = calloc(1, sizeof(event_queue_entry));
	if(new_e_q_entry == NULL){
		printf("[%s] Allocation new event fail.\n", __FUNCTION__);
		return NULL;
	}

	new_e_q_entry->io_type = io_type;
	new_e_q_entry->valid = VALID;
	new_e_q_entry->sector_nb = sector_nb;
	new_e_q_entry->length = length;
	new_e_q_entry->buf = buf;
	new_e_q_entry->next = NULL;
	
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	return new_e_q_entry;
}

void* WRITE_DATA_TO_BUFFER(unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	void* p_buf = NULL;

	/* SAVE Buffer start pointer */
	p_buf = sata_write_ptr;

	/* Write Data to Write Buffer Frame */
	INCREASE_WB_SATA_POINTER(length);

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	return p_buf;
}

void READ_DATA_FROM_BUFFER_TO_HOST(event_queue_entry* c_e_q_entry)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	sata_read_ptr = c_e_q_entry->buf;

	/* Read the buffer data and increase SATA pointer */
	INCREASE_RB_SATA_POINTER(c_e_q_entry->length);

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

void COPY_DATA_TO_READ_BUFFER(event_queue_entry* dst_entry, event_queue_entry* src_entry)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	if(dst_entry == NULL || src_entry == NULL){
		printf("[%s] Null pointer error.\n",__FUNCTION__);
		return;
	}

	int count = 0;
	int offset;
	void* dst_buf;	// new read entry
	void* src_buf;  // write entry

	int32_t dst_sector_nb = dst_entry->sector_nb;
	int32_t src_sector_nb = src_entry->sector_nb;
	unsigned int dst_length = dst_entry->length;

	/* Update read entry buffer pointer */	
	dst_entry->buf = ftl_read_ptr;
	dst_buf = ftl_read_ptr; 

	/* Calculate write buffer frame address */
	src_buf = src_entry->buf;
	offset = dst_sector_nb - src_sector_nb;
	while(count != offset){
		if(GET_WB_VALID_ARRAY_ENTRY(src_buf)!='I'){
			count++;
		}

		src_buf = src_buf + SECTOR_SIZE;
		if(src_buf == write_buffer_end){
			src_buf = write_buffer;
		}
	}

	count = 0;
	while(count != dst_length){
		if(GET_WB_VALID_ARRAY_ENTRY(src_buf)=='I'){
			src_buf = src_buf + SECTOR_SIZE;
			if(src_buf == write_buffer_end){
				src_buf = write_buffer;
			}
			continue;
		}
		/* Copy Write Buffer Data to Read Buffer */
		memcpy(dst_buf, src_buf, SECTOR_SIZE);

		/* Increase offset */
		dst_buf = dst_buf + SECTOR_SIZE;
		src_buf = src_buf + SECTOR_SIZE;
		ftl_read_ptr = ftl_read_ptr + SECTOR_SIZE;

		empty_read_buffer_frame--;

		if(dst_buf == read_buffer_end){
			dst_buf = read_buffer;
			ftl_read_ptr = read_buffer;
		}
		if(src_buf == write_buffer_end){
			src_buf = write_buffer;
		}
		count++;
	}

	INCREASE_RB_LIMIT_POINTER();

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

void FLUSH_EVENT_QUEUE_UNTIL(event_queue_entry* e_q_entry)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	int i;
	int count = 1;
	event_queue_entry* temp_e_q_entry = e_queue->head;
	
	if(e_q_entry == NULL || temp_e_q_entry == NULL){
		printf("ERROR[%s] Invalid event pointer\n",__FUNCTION__);
		return;
	}

	/* Count how many event should be flushed */
	if(e_q_entry == e_queue->tail){
		count = e_queue->entry_nb;
	}
	else{
		while(temp_e_q_entry != e_q_entry){
			count++;
			temp_e_q_entry = temp_e_q_entry->next;
		}
	}

	/* Dequeue event */
	for(i=0; i<count; i++){
		DEQUEUE_IO();
	}

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
}

int CHECK_OVERWRITE(event_queue_entry* e_q_entry, int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	int ret = 0;
	int32_t temp_sector_nb = e_q_entry->sector_nb;
	unsigned int temp_length = e_q_entry->length;

	if(e_q_entry->io_type == WRITE){
		if( sector_nb <= temp_sector_nb && \
			(sector_nb + length) >= (temp_sector_nb + temp_length)){
				
			/* Update event entry validity */
			e_q_entry->valid = INVALID;
//TEMPs
			overwrite_hit++;
			overwrite_hit_sectors += temp_length;
//TEMPe
			/* Update write buffer valid array */
			UPDATE_WB_VALID_ARRAY(e_q_entry, 'I');

			ret = 1;
		}
	}
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	if(ret == 0)
		return FAIL;
	else
		return SUCCESS;
}

int CHECK_SEQUENTIALITY(event_queue_entry* e_q_entry, int32_t sector_nb)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	int ret = 0;
	int32_t temp_sector_nb = e_q_entry->sector_nb;
	unsigned int temp_length = e_q_entry->length;

	if((e_q_entry->io_type == WRITE) && \
			(temp_sector_nb + temp_length == sector_nb)){
		ret = 1;	
	}

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	if(ret == 0)
		return FAIL;
	else
		return SUCCESS;
}

event_queue_entry* CHECK_IO_DEPENDENCY_FOR_READ(int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	int32_t last_sector_nb = sector_nb + length - 1;
	int32_t temp_sector_nb;
	int32_t temp_last_sector_nb;

	event_queue_entry* ret_e_q_entry = NULL;
	event_queue_entry* e_q_entry = NULL;
	if(last_read_entry == NULL){
		e_q_entry = e_queue->head;
	}
	else{
		e_q_entry = last_read_entry->next;
	}

	while(e_q_entry != NULL){
		if(e_q_entry->io_type == WRITE && e_q_entry->valid == VALID){
			temp_sector_nb = e_q_entry->sector_nb;
			temp_last_sector_nb = temp_sector_nb + e_q_entry->length - 1; 

			/* Find the last IO event which has dependency */		
			if(temp_sector_nb <= last_sector_nb && \
				sector_nb <= temp_last_sector_nb){
				
				ret_e_q_entry = e_q_entry;
			}
		}
		e_q_entry = e_q_entry->next;
	}

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	return ret_e_q_entry;
}

int CHECK_IO_DEPENDENCY_FOR_WRITE(event_queue_entry* e_q_entry, int32_t sector_nb, unsigned int length)
{
#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] Start.\n",__FUNCTION__);
#endif
	int ret = 0;
	int32_t last_sector_nb = sector_nb + length - 1;
	int32_t temp_sector_nb = e_q_entry->sector_nb;
	int32_t temp_last_sector_nb = temp_sector_nb + e_q_entry->length - 1;

	if(e_q_entry->io_type == WRITE && e_q_entry->valid == VALID){

		/* Find the last IO event which has dependency */		
		if(temp_sector_nb < sector_nb && \
			sector_nb < temp_last_sector_nb && \
			temp_last_sector_nb < last_sector_nb ){
			
			ret = 1;
		}
	}

#ifdef SSD_IO_BUF_DEBUG
	printf("[%s] End.\n",__FUNCTION__);
#endif
	if(ret == 1)
		return SUCCESS;
	else
		return FAIL;
}
 
void SECURE_WRITE_BUFFER(unsigned int length)
{
	int i = 0;
	void* temp_ptr = sata_write_ptr;

	for(i=0; i<length; i++){
		temp_ptr = temp_ptr + SECTOR_SIZE;
		if(temp_ptr == write_buffer_end){
			temp_ptr = write_buffer;
		}
		if(temp_ptr == write_limit_ptr){
			FLUSH_EVENT_QUEUE_UNTIL(e_queue->tail);
			break;
		}
	}
}

void SECURE_READ_BUFFER(unsigned int length)
{
	int i = 0;
	void* temp_ptr = ftl_read_ptr;

	for(i=0; i<length; i++){
		temp_ptr = temp_ptr + SECTOR_SIZE;
		if(temp_ptr == read_buffer_end){
			temp_ptr = read_buffer;
		}
		if(temp_ptr == sata_read_ptr){
			DEQUEUE_COMPLETED_READ();
			break;
		}
	}
}

char GET_WB_VALID_ARRAY_ENTRY(void* buffer_pointer)
{
	/* Calculate index of write buffer valid array */
	int index = (buffer_pointer - write_buffer)/SECTOR_SIZE;
	
	/* Update write buffer valid array */
	return wb_valid_array[index];
}

void UPDATE_WB_VALID_ARRAY(event_queue_entry* e_q_entry, char new_value)
{
	void* p_buf = e_q_entry->buf;
	int index = (p_buf - write_buffer)/SECTOR_SIZE;
	int count = 0;
	int length = e_q_entry->length;

	while(count != length){
		if(GET_WB_VALID_ARRAY_ENTRY(p_buf)!='I'){
			wb_valid_array[index] = new_value;	
			count++;
		}

		/* Increase index and buffer pointer */
		p_buf = p_buf + SECTOR_SIZE;
		index++;
		if(index == WRITE_BUFFER_FRAME_NB){
			p_buf = write_buffer;
			index = 0;
		} 
	}
}

void UPDATE_WB_VALID_ARRAY_ENTRY(void* buffer_pointer, char new_value)
{
	/* Calculate index of write buffer valid array */
	int index = (int)(buffer_pointer - write_buffer)/SECTOR_SIZE;
	if(index >= WRITE_BUFFER_FRAME_NB){
		printf("ERROR[%s] Invlald index. \n",__FUNCTION__);
		return;
	}
	
	/* Update write buffer valid array */
	wb_valid_array[index] = new_value;
}

void UPDATE_WB_VALID_ARRAY_PARTIAL(event_queue_entry* e_q_entry, char new_value, int length, int mode)
{
	// mode 0: change valid value of the front array
	// mode 1: change valid value of the rear array

	int count = 0;
	int offset = 0;
	void* p_buf = e_q_entry->buf;

	if(mode == 1){
		offset = e_q_entry->length - length;

		while(count != offset){
			if(GET_WB_VALID_ARRAY_ENTRY(p_buf)!='I'){
				count++;
			}
			p_buf = p_buf + SECTOR_SIZE;
		}
	}

	count = 0;
	while(count != length){
		if(GET_WB_VALID_ARRAY_ENTRY(p_buf)!='I'){
			UPDATE_WB_VALID_ARRAY_ENTRY(p_buf, new_value);
			count++;
		}

		/* Increase index and buffer pointer */
		p_buf = p_buf + SECTOR_SIZE;
		if(p_buf == write_buffer_end){
			p_buf = write_buffer;
		}
	}
}

void INCREASE_WB_SATA_POINTER(int entry_nb)
{
	int i;

	for(i=0; i<entry_nb; i++){
		/* Decrease the # of empty write buffer frame */
		empty_write_buffer_frame--;

		/* Update write buffer valid array */
		UPDATE_WB_VALID_ARRAY_ENTRY(sata_write_ptr, 'V');

		/* Increase sata write pointer */
		sata_write_ptr = sata_write_ptr + SECTOR_SIZE;
		
		if(sata_write_ptr == write_buffer_end){
			sata_write_ptr = write_buffer;
		}
	}
}

void INCREASE_RB_SATA_POINTER(int entry_nb)
{
	int i;

	for(i=0; i<entry_nb; i++){
		empty_read_buffer_frame++;

		sata_read_ptr = sata_read_ptr + SECTOR_SIZE;

		if(sata_read_ptr == read_buffer_end){
			sata_read_ptr = read_buffer;
		}
	}
}

void INCREASE_WB_FTL_POINTER(int entry_nb)
{
	int count = 0;
	char validity;

	while(count != entry_nb){
		/* Get write buffer frame status */
		validity = GET_WB_VALID_ARRAY_ENTRY(ftl_write_ptr);

		if(validity == 'I'){
			/* Increase ftl pointer by SECTOR_SIZE */
			ftl_write_ptr = ftl_write_ptr + SECTOR_SIZE;
			if(ftl_write_ptr == write_buffer_end){
				ftl_write_ptr = write_buffer;
			}
		}
		else{
			/* Update write buffer valid array */
			UPDATE_WB_VALID_ARRAY_ENTRY(ftl_write_ptr, 'F');

			/* Increase ftl pointer by SECTOR_SIZE */
			ftl_write_ptr = ftl_write_ptr + SECTOR_SIZE;
			if(ftl_write_ptr == write_buffer_end){
				ftl_write_ptr = write_buffer;
			}

			count++;
		}
	}
}

void INCREASE_RB_FTL_POINTER(int entry_nb)
{
	int i;

	for(i=0;i<entry_nb;i++){

		/* Increase ftl read pointer by SECTOR_SIZE */
		ftl_read_ptr = ftl_read_ptr + SECTOR_SIZE;
		if(ftl_read_ptr == read_buffer_end){
			ftl_read_ptr = read_buffer;
		}
		empty_read_buffer_frame--;
	}
}

void INCREASE_WB_LIMIT_POINTER(void)
{
	/* Increase write limit pointer until ftl write pointer */
	while(write_limit_ptr != ftl_write_ptr){

		/* Update write buffer valid array */
		UPDATE_WB_VALID_ARRAY_ENTRY(write_limit_ptr, '0');

		/* Incrase write limit pointer by SECTOR_SIZE */
		write_limit_ptr = write_limit_ptr + SECTOR_SIZE;
		if(write_limit_ptr == write_buffer_end){
			write_limit_ptr = write_buffer;
		}

		empty_write_buffer_frame++;
	}
}

void INCREASE_RB_LIMIT_POINTER(void)
{
	/* Increase read limit pointer until ftl read pointer */
	while(read_limit_ptr != ftl_read_ptr){

		/* Increase read lmit pointer by SECTOR_SIZE */
		read_limit_ptr = read_limit_ptr + SECTOR_SIZE;
		if(read_limit_ptr == read_buffer_end){
			read_limit_ptr = read_buffer;
		}
	}
}

void MOVE_EVENT_TO_FTL_BUFFER(event_queue_entry* e_q_entry)
{
	unsigned int length = e_q_entry->length;

	if(ftl_buffer_entry.valid == VALID){
		if(ftl_buffer_entry.sector_nb + ftl_buffer_entry.length != e_q_entry->sector_nb){
			printf("ERROR[%s] This entry is not sequential write \n", __FUNCTION__);
			return;
		}
	}

	/* Secure FTL Buffer for new write event */
	if(ftl_buffer_remain_entry < length){
		FLUSH_FTL_BUFFER();
	}

	/* Copy data from write buffer to FTL buffer */
	COPY_DATA_TO_FTL_BUFFER(e_q_entry);

	if(ftl_buffer_entry.valid == INVALID){
		ftl_buffer_entry.io_type = WRITE;
		ftl_buffer_entry.valid = VALID;
		ftl_buffer_entry.sector_nb = e_q_entry->sector_nb;
		ftl_buffer_entry.length = e_q_entry->length;
		ftl_buffer_entry.buf = ftl_buffer;
		ftl_buffer_entry.next = NULL;
	}
	else{
		/* Update ftl event entry */
		ftl_buffer_entry.length += length;
	}

	/* If new seq event is the last seq event, */
	if(GET_WB_VALID_ARRAY_ENTRY(e_q_entry->buf)=='E'){
		FLUSH_FTL_BUFFER();
	}
}

void COPY_DATA_TO_FTL_BUFFER(event_queue_entry* e_q_entry)
{
	int count = 0;
	int length = e_q_entry->length;
	void* src_buf = e_q_entry->buf;

	while(count != length){
		if(GET_WB_VALID_ARRAY_ENTRY(src_buf)!='I'){
			memcpy(curr_ftl_buffer_entry, src_buf, SECTOR_SIZE);
			curr_ftl_buffer_entry = curr_ftl_buffer_entry + SECTOR_SIZE;
			ftl_buffer_remain_entry--;
			count++;
		}

		src_buf = src_buf + SECTOR_SIZE;
		if(src_buf == write_buffer_end){
			src_buf = write_buffer;
		}
	}
}

void FLUSH_FTL_BUFFER(void)
{
	if(ftl_buffer_entry.valid == INVALID){
		printf("ERROR[%s] There is no event.\n",__FUNCTION__);
		return;
	}
	FTL_WRITE(ftl_buffer_entry.sector_nb, ftl_buffer_entry.length);

	/* Update FTL Buffer Global Valiable */
	ftl_buffer_remain_entry += ftl_buffer_entry.length;
	if(ftl_buffer_remain_entry != FTL_BUFFER_FRAME_NB){
		printf("ERROR[%s] ftl_buffer_remain_entry: %d/%d \n",__FUNCTION__,ftl_buffer_remain_entry, FTL_BUFFER_FRAME_NB);
		return;
	}
	
	curr_ftl_buffer_entry = ftl_buffer;
	ftl_buffer_entry.valid = INVALID;
}
