#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define PAGE_NB 256

long long get_current_utime(void)
{
        struct timeval current;

        gettimeofday(&current,NULL);

        return (current.tv_sec*1000000 + current.tv_usec);
}


int main (int argc, char* argv[])
{
	FILE *i_fp;
	FILE *o_fp;

	int i, j, k;
	int ret;

	/* trace data */
	double sector;
	double size;

	/* performance variable */
	double total_count=0;
	double read_count=0;
	double total_size=0;

	long long io_time_start, io_time_end;

	/* input, output file open */
	i_fp = fopen(argv[1],"r");

	if(i_fp == NULL){
		printf("fopen error ! \n");
	}

	/* read input file and calculate */
	i = 0;
	io_time_start = get_current_utime();	

	while(1){

		/* read data from file */
		ret = fscanf(i_fp, "%lf\t%lf\n",&sector, &size);
		
		/* if end of file */
		if(ret < 0)
			break;

		total_count++;
		total_size += size;
	}

	printf("**************************\n");
	printf("Total operation count	: %lf \n", total_count);
	printf("Total size		: %lf KB\n", (total_size*512)/1024);
	printf("**************************\n");

	/* input, output file close */
	fclose(i_fp);

	return 0;
}
