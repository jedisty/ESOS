#ifndef _SSD_MT_IO_MANAGER_H
#define _SSD_MT_IO_MANAGER_H

/* Initialize SSD Module */
int SSD_MT_IO_INIT(void);

void *CH_THREAD_FUNCTION(void* arg);
void *FLASH_THREAD_FUNCTION(void* arg);

/* GET IO from FTL */
int SSD_PAGE_READ_MT(unsigned int flash_nb, unsigned int block_nb, unsigned int page_nb);
int SSD_PAGE_WRITE_MT(unsigned int flash_nb, unsigned int block_nb, unsigned int page_nb);

pthread_cond_t* find_ch_cond_var(int channel_nb);
pthread_mutex_t* find_ch_lock(int channel_nb);

pthread_cond_t* find_flash_cond_var(int flash_nb);
pthread_mutex_t* find_flash_lock(int flash_nb);

int* find_ch_cmd_var(int channel_nb);
int* find_flash_cmd_var(int flash_nb);

void SSD_CH_ENABLE_MT(int channel, int cmd);
void SSD_CH_SWITCH_DELAY_MT(int channel, int cmd);

extern int F_O_DIRECT_VSSIM_MT;
#endif
