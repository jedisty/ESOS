// copyright(c)2014
//
// Hanyang University, Seoul, Korea
// Embedded Software Systems Lab. All right reserved

#ifndef _VSSIM_BENCH_H_
#define _VSSIM_BENCH_H_

void read_workload(char* workload_file);
void make_random_sector_nb(double* arr_random_sects, int iteration);
void print_result(void);
void re_init(void);
#endif
