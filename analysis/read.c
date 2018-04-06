#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PAGE_NB 256

int main (int argc, char* argv[])
{
	FILE *i_fp;
	FILE *o_fp;

	int i, j, k;
	int ret;

	/* trace data */
	int core_id;
	int64_t page_nb;
	int64_t addr;
	int	flash_nb;
	int	plane_nb;
	int	block_nb;
	int	offset;

	/* performance variable */
	double read_count=0;

	/* input, output file open */
	i_fp = fopen(argv[1],"r");

	if(i_fp == NULL){
		printf("fopen error ! \n");
	}

	/* read input file and calculate */
	i = 0;
	while(1){
		/* read data from file */
		ret = fscanf(i_fp, "%d\t%ld\t%ld\t%d\t%d\t%d\t%d\n",
				&core_id,
				&page_nb,
				&addr,
				&flash_nb,
				&plane_nb,
				&block_nb,
				&offset);
		
		/* if end of file */
		if(ret < 0)
			break;

		if(addr != -1){
			if(core_id == 0
				&& !(flash_nb == 0 || flash_nb == 3 || flash_nb == 4
					|| flash_nb == 7 || flash_nb == 8 || flash_nb == 11
					|| flash_nb == 12 || flash_nb == 15))
			{
				printf("%lf\t%d\t%ld\t%ld\t%d\t%d\t%d\t%d\n", read_count,
					core_id, page_nb, addr, flash_nb, plane_nb, block_nb, offset);
			}
			else if(core_id == 1 && !(flash_nb == 1 || flash_nb == 5 
					|| flash_nb == 9 || flash_nb == 13)){
				printf("%lf\t%d\t%ld\t%ld\t%d\t%d\t%d\t%d\n", read_count,
					core_id, page_nb, addr, flash_nb, plane_nb, block_nb, offset);

			}
			else if(core_id == 2 && !(flash_nb == 2 || flash_nb == 6 
					|| flash_nb == 10 || flash_nb == 14)){
				printf("%lf\t%d\t%ld\t%ld\t%d\t%d\t%d\t%d\n", read_count,
					core_id, page_nb, addr, flash_nb, plane_nb, block_nb, offset);
			}
		}

		read_count++;
	}


	/* input, output file close */
	fclose(i_fp);

	return 0;
}
