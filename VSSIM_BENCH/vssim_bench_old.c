#include "vssim_bench.h"
#include "common.h"

enum{
	SEQ_W = 1,
	SEQ_R,
	RAN_W,
	RAN_R
};

int io_type;
double file_size;	// KByte
double record_size;	// KByte

int64_t start_time, end_time;

int main(int argc, char* argv[])
{
#ifdef VSSIM_BENCH_MULTI_THREAD
	FTL_MT_INIT();
#else
	FTL_INIT();	
#endif
	double remain_sectors;
	double sects;
	double sector_nb = 0;
	double* arr_random_sects;

	double temp_remain_sectors;
	double temp_sects;

	int i;
	int iteration;

	// determine workload from file
	read_workload(argv[1]);

	remain_sectors = (file_size * 1024)/SECTOR_SIZE;
	sects = (record_size * 1024)/SECTOR_SIZE;

	if(remain_sectors > SECTOR_NB){
		printf("The workload exceed SSD capacity!\n");
		return 0;
	}

	printf("Workload:\n");
	printf("\t File size: %.0lf KB\n", file_size);
	printf("\t Record size: %.0lf KB\n", record_size);

	if(io_type == RAN_R || io_type == RAN_W){
		iteration = (int)(remain_sectors / sects);
		if((sects * iteration) != remain_sectors){
			iteration += 1;
		}
		arr_random_sects = malloc(iteration * sizeof(double));
		if(arr_random_sects == NULL){
			printf("Error[%s] alloc arr_random_sets fail\n",__FUNCTION__);
		}

		make_random_sector_nb(arr_random_sects, iteration);

		for(i=0;i<iteration;i++){
			arr_random_sects[i] = arr_random_sects[i] * sects;
		}
	}

	switch(io_type){
		case SEQ_W:
			printf("Sequential Write Start! ... ");

			start_time = get_usec();
			while(remain_sectors > 0){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_WRITE(sector_nb, sects);
#else
				FTL_WRITE(sector_nb, sects);
#endif			
				// Update variables
				remain_sectors -= sects;
				sector_nb += sects;
			}
			end_time = get_usec();

			break;
		case SEQ_R:
			// Write Data befor Read
			temp_remain_sectors = remain_sectors;
			temp_sects = sects;
			while(remain_sectors > 0){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_WRITE(sector_nb, sects);
#else
				FTL_WRITE(sector_nb, sects);
#endif
				// Update variables
				remain_sectors -= sects;
				sector_nb += sects;
			}

			printf("Sequential Read Start! ... ");
		
			remain_sectors = temp_remain_sectors;
			sects = temp_sects;
			sector_nb = 0;

			start_time = get_usec();
			while(remain_sectors > 0){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_READ(sector_nb, sects);
#else
				FTL_READ(sector_nb, sects);
#endif
				// Update variables
				remain_sectors -= sects;
				sector_nb += sects;
			}
			end_time = get_usec();

			break;
		case RAN_W:
			printf("Random Write Start! ... ");
			i=0;

			start_time = get_usec();
			while(remain_sectors){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_WRITE(arr_random_sects[i], sects);
#else
				FTL_WRITE(arr_random_sects[i], sects);
#endif			
				// Update variables
				remain_sectors -= sects;
				i++;
			}
			end_time = get_usec();

			break;
		case RAN_R:
			// Write Data befor Read
			temp_remain_sectors = remain_sectors;
			temp_sects = sects;
			while(remain_sectors > 0){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_WRITE(sector_nb, sects);
#else
				FTL_WRITE(sector_nb, sects);
#endif			
				// Update variables
				remain_sectors -= sects;
				sector_nb += sects;
			}

			printf("Random Read Start! ... ");
			remain_sectors = temp_remain_sectors;
			sects = temp_sects;
			i=0;

			start_time = get_usec();
			while(remain_sectors > 0){
				if(remain_sectors < sects){
					sects = remain_sectors;
				}
				// Call FTL IO Function
#ifdef VSSIM_BENCH_MULTI_THREAD
				FTL_MT_READ(arr_random_sects[i], sects);
#else
				FTL_READ(arr_random_sects[i], sects);
#endif
				// Update variables
				remain_sectors -= sects;
				i++;
			}
			end_time = get_usec();
			break;
		default:
			printf("Wrong Workload!\n");
			break;
	}
	printf("End!\n\n");

	print_result();

	return 0;
}
void read_workload(char* workload_file)
{
	FILE* fp;
	int ret;

	char type[3];	// sequenatial, random
	char io;	// read, write

	fp = fopen(workload_file, "r");
	if(fp == NULL){
		printf("Open workload fail!\n");
	}

	while(1){
		ret = fscanf(fp, "%s\t%c\t%lf\t%lf", type, &io, &file_size, &record_size);
		if(ret < 0)
			break;

		if(!strcmp(type, "SEQ")){
			if(io == 'R' || io == 'r')
				io_type = SEQ_R;
			else if(io == 'W' || io == 'w')
				io_type = SEQ_W;
			else
				printf("Wrong workload!\n");
		}
		else if(!strcmp(type, "RAN")){
			if(io == 'R' || io == 'r')
				io_type = RAN_R;
			else if(io == 'W' || io == 'w')
				io_type = RAN_W;
			else
				printf("Wrong workload!\n");
		}
	}

	fclose(fp);
}

void make_random_sector_nb(double* arr_random_sects, int iteration)
{
	int i, j;
	int ret;
	int check = 0;
	int random_num;
	double temp_num;
	FILE* fp;

	fp = fopen("./random_data.txt", "r");
	if(fp == NULL){

		fp = fopen("./random_data.txt", "w");
		srand((unsigned)time(NULL));

		for(i=0;i<iteration;i++){
			arr_random_sects[i] = -1;
		}

		for(i=0;i<iteration;i++){
		
			random_num = rand()%iteration;
			for(j=0;j<i;j++){
				if(arr_random_sects[j] == random_num)
					check = 1;
			}

			if(check == 1){
				i--;
				check = 0;
			}
			else{
				arr_random_sects[i]=random_num;
				fprintf(fp, "%d\n", random_num);
				printf("%d \n", i);
			}
		}

	}
	else{
		i = 0;

		while(1){
			ret = fscanf(fp, "%lf", &temp_num);
			if(ret < 0)
				break;
			arr_random_sects[i] = temp_num;
			i++;
		}
	}
	fclose(fp);
}

void print_result(void)
{
	double total_time_usec = (double)(end_time - start_time);
	double total_time_sec = total_time_usec / 1000000;
	double bandwidth_kbs = file_size/total_time_sec;
	double bandwidth_mbs = bandwidth_kbs / 1024;
	double iops = bandwidth_kbs/((double)PAGE_SIZE/1024);

	printf("[Benchmakr result]\n");
	printf(" - Total Run-time :\t%lf sec\n", total_time_sec);
	if(io_type == SEQ_R || io_type == SEQ_W){
		printf(" - Bandwidth[KB/s]:\t%.3lf KB/s\n", bandwidth_kbs);
		printf(" - Bandwidth[MB/s]:\t%.3lf MB/s\n", bandwidth_mbs);
	}
	else{
		printf(" - IOPS:\t%.3lf IOPS\n", iops);
	}
}
