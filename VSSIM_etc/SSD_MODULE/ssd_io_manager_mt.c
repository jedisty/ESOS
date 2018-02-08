#include "common.h"
#include <pthread.h>

int F_O_DIRECT_VSSIM_MT;

pthread_t* ch_thread_id;
pthread_t* flash_thread_id;

/* condition variable list */
pthread_cond_t ch_ready_0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_4 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_5 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_6 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_7 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_8 = PTHREAD_COND_INITIALIZER;
pthread_cond_t ch_ready_9 = PTHREAD_COND_INITIALIZER;

pthread_cond_t flash_ready_0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_4 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_5 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_6 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_7 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_8 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_9 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_10 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_11 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_12 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_13 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_14 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_15 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_16 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_17 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_18 = PTHREAD_COND_INITIALIZER;
pthread_cond_t flash_ready_19 = PTHREAD_COND_INITIALIZER;

/* channel lock list */
pthread_mutex_t ch_lock_0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_8 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ch_lock_9 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t flash_lock_0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_5 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_6 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_7 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_8 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_9 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_10 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_11 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_12 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_13 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_14 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_15 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_16 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_17 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_18 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flash_lock_19 = PTHREAD_MUTEX_INITIALIZER;

/* command list */
int ch_io_cmd_0[2];
int ch_io_cmd_1[2];
int ch_io_cmd_2[2];
int ch_io_cmd_3[2];
int ch_io_cmd_4[2];
int ch_io_cmd_5[2];
int ch_io_cmd_6[2];
int ch_io_cmd_7[2];
int ch_io_cmd_8[2];
int ch_io_cmd_9[2];

int flash_io_cmd_0;
int flash_io_cmd_1;
int flash_io_cmd_2;
int flash_io_cmd_3;
int flash_io_cmd_4;
int flash_io_cmd_5;
int flash_io_cmd_6;
int flash_io_cmd_7;
int flash_io_cmd_8;
int flash_io_cmd_9;
int flash_io_cmd_10;
int flash_io_cmd_11;
int flash_io_cmd_12;
int flash_io_cmd_13;
int flash_io_cmd_14;
int flash_io_cmd_15;
int flash_io_cmd_16;
int flash_io_cmd_17;
int flash_io_cmd_18;
int flash_io_cmd_19;

int old_ch_nb;
int old_ch_cmd;

int* flash_io_done;
pthread_mutex_t flash_io_done_lock = PTHREAD_MUTEX_INITIALIZER;

int* ch_io_done;
pthread_mutex_t ch_io_done_lock = PTHREAD_MUTEX_INITIALIZER;

int SSD_MT_IO_INIT(void){

	int i;
	int temp;
	int ret;
#ifdef DEBUG_SSD_MT_IO
	printf("[%s] Start \n", __FUNCTION__);
#endif

	/* Init Command and Command type */

	ch_io_cmd_0[0] = NOOP;
	ch_io_cmd_0[1] = FLASH_NB;
	ch_io_cmd_1[0] = NOOP;
	ch_io_cmd_1[1] = FLASH_NB;
	ch_io_cmd_2[0] = NOOP;
	ch_io_cmd_2[1] = FLASH_NB;
	ch_io_cmd_3[0] = NOOP;
	ch_io_cmd_3[1] = FLASH_NB;
	ch_io_cmd_4[0] = NOOP;
	ch_io_cmd_4[1] = FLASH_NB;
	ch_io_cmd_5[0] = NOOP;
	ch_io_cmd_5[1] = FLASH_NB;
	ch_io_cmd_6[0] = NOOP;
	ch_io_cmd_6[1] = FLASH_NB;
	ch_io_cmd_7[0] = NOOP;
	ch_io_cmd_7[1] = FLASH_NB;
	ch_io_cmd_8[0] = NOOP;
	ch_io_cmd_8[1] = FLASH_NB;
	ch_io_cmd_9[0] = NOOP;
	ch_io_cmd_9[1] = FLASH_NB;

	flash_io_cmd_0 = NOOP;
	flash_io_cmd_1 = NOOP;
	flash_io_cmd_2 = NOOP;
	flash_io_cmd_3 = NOOP;
	flash_io_cmd_4 = NOOP;
	flash_io_cmd_5 = NOOP;
	flash_io_cmd_6 = NOOP;
	flash_io_cmd_7 = NOOP;
	flash_io_cmd_8 = NOOP;
	flash_io_cmd_9 = NOOP;
	flash_io_cmd_10 = NOOP;
	flash_io_cmd_11 = NOOP;
	flash_io_cmd_12 = NOOP;
	flash_io_cmd_13 = NOOP;
	flash_io_cmd_14 = NOOP;
	flash_io_cmd_15 = NOOP;
	flash_io_cmd_16 = NOOP;
	flash_io_cmd_17 = NOOP;
	flash_io_cmd_18 = NOOP;
	flash_io_cmd_19 = NOOP;

	ch_io_done = (int*)calloc(CHANNEL_NB, sizeof(int));
	for(i=0; i<CHANNEL_NB; i++){
		ch_io_done[i] = -1;
	}

	flash_io_done = (int*)calloc(FLASH_NB, sizeof(int));
	for(i=0; i<FLASH_NB; i++){
		flash_io_done[i] = -1;
	}

	/* Create Channel Thread */
	ch_thread_id = (pthread_t*)calloc(CHANNEL_NB, sizeof(pthread_t));

	for(i=0; i< CHANNEL_NB; i++){
		temp=i;
		ret = pthread_create(&ch_thread_id[i], NULL, CH_THREAD_FUNCTION, &temp);
		if (ret != 0){
			printf("Error[%s] channel thread creeate error \n",__FUNCTION__);
			return 0;
		}
		usleep("1000");
	}

	/* Create FLASH Thread */
	flash_thread_id = (pthread_t*)calloc(FLASH_NB, sizeof(pthread_t));

	for(i=0; i< FLASH_NB; i++){
		temp = i;
		ret = pthread_create(&flash_thread_id[i], NULL, FLASH_THREAD_FUNCTION, &temp);
		if (ret != 0){
			printf("Error[%s] flash thread creeate error \n",__FUNCTION__);
			return 0;
		}
		usleep("1000");
	}

	F_O_DIRECT_VSSIM_MT = -1;
	old_ch_nb = -1;

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] End \n", __FUNCTION__);
#endif

	return 0;
}

void *CH_THREAD_FUNCTION(void* arg)
{
	int ch_cmd;
	int ch_nb = *(int *)arg;
	int* ch_io_cmd = find_ch_cmd_var(ch_nb);
	pthread_cond_t* ch_ready = find_ch_cond_var(ch_nb);
	pthread_mutex_t* ch_lock = find_ch_lock(ch_nb);

	int* flash_io_cmd ;
	pthread_cond_t* flash_ready;
	pthread_mutex_t* flash_lock;

	int flash_nb;
	int64_t start_time, end_time;

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] ch thread %d is created!\n", __FUNCTION__, ch_nb);
#endif

	while(1){
		pthread_mutex_lock(&(*ch_lock));

		while(ch_io_cmd[0] == NOOP){
#ifdef DEBUG_SSD_MT_IO
			printf("[%s] ch[%d] wait signal ...\n", __FUNCTION__, ch_nb);
#endif
			pthread_cond_wait(&(*ch_ready), &(*ch_lock));
		}
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] ch[%d] get up! and get ch_lock \n",__FUNCTION__, ch_nb);
#endif
		ch_cmd = ch_io_cmd[0];
		flash_nb = ch_io_cmd[1];
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] ch[%d] relase ch_lock before delay\n",__FUNCTION__, ch_nb);
#endif
		pthread_mutex_unlock(&(*ch_lock));

		if(ch_cmd == WRITE){
#ifdef DEBUG_SSD_MT_IO
			printf("[%s] ch[%d] write data \n",__FUNCTION__, ch_nb);
#endif
			flash_ready = find_flash_cond_var(flash_nb);
			flash_lock = find_flash_lock(flash_nb);	
			flash_io_cmd = find_flash_cmd_var(flash_nb);
			while(*flash_io_cmd!=NOOP){/* wait */}

			start_time = get_usec();
			end_time = start_time + REG_WRITE_DELAY;
			while(1){
				if(get_usec() >= end_time) break;
			}

			pthread_mutex_lock(&(*flash_lock));
#ifdef DEBUG_SSD_MT_IO
			printf("[%s] ch[%d] update f[%d] cmd \n",__FUNCTION__, ch_nb, flash_nb);
#endif
			*flash_io_cmd=WRITE;
			pthread_cond_signal(&(*flash_ready));
			pthread_mutex_unlock(&(*flash_lock));
		}
		else if(ch_cmd == READ){
			start_time = get_usec();
			end_time = start_time + REG_READ_DELAY;
			while(1){
				if(get_usec() >= end_time) break;
			}
		}
		else{
			printf("Error[%s] Wrong channel command \n",__FUNCTION__);
		}


		pthread_mutex_lock(ch_lock);
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] ch[%d] get ch Lock! and become NOOP\n", __FUNCTION__, ch_nb);
#endif
		ch_io_cmd[0] = NOOP;
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] ch[%d] release ch Lock after delay\n",__FUNCTION__, ch_nb);
#endif
		pthread_mutex_unlock(ch_lock);

		pthread_mutex_lock(&ch_io_done_lock);
		ch_io_done[ch_nb] = 1;
		pthread_mutex_unlock(&ch_io_done_lock);
	}
}

void *FLASH_THREAD_FUNCTION(void* arg)
{
	int flash_nb = *(int *)arg;
#ifdef DEBUG_SSD_MT_IO
	printf("[%s] flash thread %d is created!\n", __FUNCTION__, flash_nb);
#endif
	int flash_cmd;
	int ch_nb = flash_nb % CHANNEL_NB;
	int64_t start_time, end_time;

	int* flash_io_cmd = find_flash_cmd_var(flash_nb);
	pthread_cond_t* flash_ready = find_flash_cond_var(flash_nb);
	pthread_mutex_t* flash_lock = find_flash_lock(flash_nb);

	int* ch_io_cmd = find_ch_cmd_var(ch_nb);
	pthread_cond_t* ch_ready = find_ch_cond_var(ch_nb);
	pthread_mutex_t* ch_lock = find_ch_lock(ch_nb);

	while(1){
		pthread_mutex_lock(&(*flash_lock));

		while(*flash_io_cmd == NOOP){
#ifdef DEBUG_SSD_MT_IO
			printf("[%s] f[%d] wait signal ...\n", __FUNCTION__, flash_nb);
#endif
			pthread_cond_wait(&(*flash_ready), &(*flash_lock));
		}
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] f[%d] get up!\n",__FUNCTION__, flash_nb);
#endif
		flash_cmd = *flash_io_cmd;
		pthread_mutex_unlock(&(*flash_lock));

		if(flash_cmd == READ){
			start_time = get_usec();
			end_time = start_time + CELL_READ_DELAY;
			while(1){
				if(get_usec() >= end_time) break;
			}

			while(ch_io_cmd[0]!=NOOP){/* wait */}
			
			pthread_mutex_lock(&(*ch_lock));
			ch_io_cmd[0]=READ;
			ch_io_cmd[1]=flash_nb;
			pthread_mutex_unlock(&(*ch_lock));

			pthread_cond_signal(&(*ch_ready));
		}
		else if(flash_cmd == WRITE){
#ifdef DEBUG_SSD_MT_IO
			printf("[%s] f[%d] Write page \n",__FUNCTION__, flash_nb);
#endif
			start_time = get_usec();
			end_time = start_time + CELL_PROGRAM_DELAY;
			while(1){
				if(get_usec() >= end_time) break;
			}

		}
		else{
			printf("Error[%s] Wrong channel command \n",__FUNCTION__);
		}


		pthread_mutex_lock(&(*flash_lock));
#ifdef DEBUG_SSD_MT_IO
		printf("[%s] f[%d] get Lock and become NOOP\n", __FUNCTION__, flash_nb);
#endif
		*flash_io_cmd = NOOP;
		pthread_mutex_unlock(&(*flash_lock));

		pthread_mutex_lock(&flash_io_done_lock);
		flash_io_done[flash_nb] = 1;
		pthread_mutex_unlock(&flash_io_done_lock);
	}
}

int SSD_PAGE_READ_MT(unsigned int flash_nb, unsigned int block_nb, unsigned int page_nb)
{
#ifdef DEBUG_SSD_MT_IO
	printf("[%s] %d %d %d \n", __FUNCTION__, flash_nb, block_nb, page_nb);
#endif
	int ch_nb = flash_nb % CHANNEL_NB;
	int* flash_io_cmd = find_flash_cmd_var(flash_nb);
	pthread_cond_t* flash_ready = find_flash_cond_var(flash_nb);
	pthread_mutex_t* flash_lock = find_flash_lock(flash_nb);

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] Wait flash NOOP ...\n",__FUNCTION__);
#endif

	while(*flash_io_cmd != NOOP){/*wait*/}
	while(ch_io_done[ch_nb]==-1){/*wait*/}
	

	SSD_CH_ENABLE_MT(ch_nb, READ);
	old_ch_nb = ch_nb;

	pthread_mutex_lock(&ch_io_done_lock);
	ch_io_done[ch_nb] = -1;
	pthread_mutex_unlock(&ch_io_done_lock);

	pthread_mutex_lock(&(*flash_lock));
	*flash_io_cmd=READ;
	pthread_mutex_unlock(&(*flash_lock));
	pthread_cond_signal(&(*flash_ready));

	if(F_O_DIRECT_VSSIM_MT == 1){
		while(ch_io_done[ch_nb]==-1){/*wait*/}
	}

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] End \n", __FUNCTION__);
#endif
}

int SSD_PAGE_WRITE_MT(unsigned int flash_nb, unsigned int block_nb, unsigned int page_nb)
{
#ifdef DEBUG_SSD_MT_IO
	printf("[%s] %d %d %d \n", __FUNCTION__, flash_nb, block_nb, page_nb);
#endif

	int ch_nb = flash_nb % CHANNEL_NB;
	int* ch_io_cmd = find_ch_cmd_var(ch_nb);
	int* flash_io_cmd = find_flash_cmd_var(flash_nb);
	pthread_cond_t* ch_ready = find_ch_cond_var(ch_nb);
	pthread_mutex_t* ch_lock = find_ch_lock(ch_nb);

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] Wait channel NOOP ...\n",__FUNCTION__);
#endif

	while(ch_io_cmd[0]!=NOOP){/*wait*/}
	while(*flash_io_cmd!=NOOP){/*wait*/}

	SSD_CH_ENABLE_MT(ch_nb, WRITE);
	old_ch_nb = ch_nb;

	pthread_mutex_lock(&flash_io_done_lock);
	flash_io_done[flash_nb] = -1;
	pthread_mutex_unlock(&flash_io_done_lock);

	pthread_mutex_lock(&(*ch_lock));
	ch_io_cmd[0]=WRITE;
	ch_io_cmd[1]=flash_nb;
	pthread_mutex_unlock(&(*ch_lock));
	pthread_cond_signal(&(*ch_ready));

	if(F_O_DIRECT_VSSIM_MT == 1){
		while(flash_io_done[flash_nb]==-1){/*wait*/}
	}

#ifdef DEBUG_SSD_MT_IO
	printf("[%s] End \n", __FUNCTION__);
#endif
}

void SSD_CH_ENABLE_MT(int channel, int cmd)
{
	if(CHANNEL_SWITCH_DELAY_R == 0 && CHANNEL_SWITCH_DELAY_W == 0)
		return;

	if(old_ch_nb != channel){
		SSD_CH_SWITCH_DELAY_MT(channel, cmd);
	}
}

void SSD_CH_SWITCH_DELAY_MT(int channel, int cmd)
{
	int64_t start = 0;
       	int64_t	end = 0;
	int64_t diff = 0;

	int64_t switch_delay = 0;

	if(cmd == READ){
		switch_delay = CHANNEL_SWITCH_DELAY_R;
	}
	else if(cmd == WRITE){
		switch_delay = CHANNEL_SWITCH_DELAY_W;
	}
	else{
		return;
	}

	start = get_usec();
	end = start + switch_delay;

	while(1){
		if(get_usec()>=end) break;
	}
}

int* find_ch_cmd_var(int channel_nb)
{
	int* ret;
	switch(channel_nb){
		case 0:	ret = ch_io_cmd_0; break;
		case 1: ret = ch_io_cmd_1; break;
		case 2:	ret = ch_io_cmd_2; break;
		case 3:	ret = ch_io_cmd_3; break;
		case 4:	ret = ch_io_cmd_4; break;
		case 5:	ret = ch_io_cmd_5; break;
		case 6:	ret = ch_io_cmd_6; break;
		case 7:	ret = ch_io_cmd_7; break;
		case 8: ret = ch_io_cmd_8; break;
		case 9:	ret = ch_io_cmd_9; break;
	}
	return ret;
}

pthread_cond_t* find_ch_cond_var(int channel_nb)
{
	switch(channel_nb){
		case 0:	return &ch_ready_0;
		case 1:	return &ch_ready_1;
		case 2:	return &ch_ready_2;
		case 3:	return &ch_ready_3;
		case 4:	return &ch_ready_4;
		case 5:	return &ch_ready_5;
		case 6:	return &ch_ready_6;
		case 7:	return &ch_ready_7;
		case 8:	return &ch_ready_8;
		case 9:	return &ch_ready_9;
	}
}

pthread_mutex_t* find_ch_lock(int channel_nb)
{
	switch(channel_nb){
		case 0:	return &ch_lock_0;
		case 1:	return &ch_lock_1;
		case 2:	return &ch_lock_2;
		case 3:	return &ch_lock_3;
		case 4:	return &ch_lock_4;
		case 5:	return &ch_lock_5;
		case 6:	return &ch_lock_6;
		case 7:	return &ch_lock_7;
		case 8:	return &ch_lock_8;
		case 9:	return &ch_lock_9;
	}
}

int* find_flash_cmd_var(int flash_nb)
{
	switch(flash_nb){
		case 0:	return &flash_io_cmd_0;
		case 1:	return &flash_io_cmd_1;
		case 2:	return &flash_io_cmd_2;
		case 3:	return &flash_io_cmd_3;
		case 4:	return &flash_io_cmd_4;
		case 5:	return &flash_io_cmd_5;
		case 6:	return &flash_io_cmd_6;
		case 7:	return &flash_io_cmd_7;
		case 8:	return &flash_io_cmd_8;
		case 9:	return &flash_io_cmd_9;
		case 10: return &flash_io_cmd_10;
		case 11: return &flash_io_cmd_11;
		case 12: return &flash_io_cmd_12;
		case 13: return &flash_io_cmd_13;
		case 14: return &flash_io_cmd_14;
		case 15: return &flash_io_cmd_15;
		case 16: return &flash_io_cmd_16;
		case 17: return &flash_io_cmd_17;
		case 18: return &flash_io_cmd_18;
		case 19: return &flash_io_cmd_19;
	}
}

pthread_cond_t* find_flash_cond_var(int flash_nb)
{
	switch(flash_nb){
		case 0: return &flash_ready_0;
		case 1:	return &flash_ready_1;
		case 2:	return &flash_ready_2;
		case 3:	return &flash_ready_3;
		case 4:	return &flash_ready_4;
		case 5:	return &flash_ready_5;
		case 6:	return &flash_ready_6;
		case 7:	return &flash_ready_7;
		case 8:	return &flash_ready_8;
		case 9:	return &flash_ready_9;
		case 10: return &flash_ready_10;
		case 11: return &flash_ready_11;
		case 12: return &flash_ready_12;
		case 13: return &flash_ready_13;
		case 14: return &flash_ready_14;
		case 15: return &flash_ready_15;
		case 16: return &flash_ready_16;
		case 17: return &flash_ready_17;
		case 18: return &flash_ready_18;
		case 19: return &flash_ready_19;
	}
}

pthread_mutex_t* find_flash_lock(int flash_nb)
{
	switch(flash_nb){
		case 0: return &flash_lock_0;
		case 1:	return &flash_lock_1;
		case 2: return &flash_lock_2;
		case 3: return &flash_lock_3;
		case 4: return &flash_lock_4;
		case 5: return &flash_lock_5;
		case 6:	return &flash_lock_6;
		case 7:	return &flash_lock_7;
		case 8:	return &flash_lock_8;
		case 9:	return &flash_lock_9;
		case 10: return &flash_lock_10;
		case 11: return &flash_lock_11;
		case 12: return &flash_lock_12;
		case 13: return &flash_lock_13;
		case 14: return &flash_lock_14;
		case 15: return &flash_lock_15;
		case 16: return &flash_lock_16;
		case 17: return &flash_lock_17;
		case 18: return &flash_lock_18;
		case 19: return &flash_lock_19;
	}
}
