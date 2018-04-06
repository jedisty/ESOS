#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define MAX_IO_LENGTH 524288

int n_files;
long long file_size_1k;
long long record_size_1k;
int fd;
char* path;
char file_name[10] = "test_file";
char cold_file_name[10] = "cold_file";

FILE* fp;

void init_test_files(void);
void open_test_files(void);
void create_test_files(void);
double overwrite_test_files(void);

#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */

/* The array for the state vector */
static unsigned long long mt[NN];

/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1;

void init_genrand64(unsigned long long seed)
{
	mt[0] = seed;
	for (mti=1; mti<NN; mti++)
		mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

unsigned long long genrand64_int64(void)
{
	int i;
	unsigned long long x;
	static unsigned long long mag01[2]={0ULL, MATRIX_A};

	if (mti >= NN) { /* generate NN words at one time */

		/* if init_genrand64() has not been called, */
		/* a default initial seed is used     */
		if (mti == NN+1){
			srand((unsigned)time(NULL));
			init_genrand64((unsigned long long)rand());	
		}

		for (i=0;i<NN-MM;i++) {
			x = (mt[i]&UM)|(mt[i+1]&LM);
			mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
		}
		for (;i<NN-1;i++) {
			x = (mt[i]&UM)|(mt[i+1]&LM);
			mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
		}
		x = (mt[NN-1]&UM)|(mt[0]&LM);
		mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

		mti = 0;
	}

	x = mt[mti++];

	x ^= (x >> 29) & 0x5555555555555555ULL;
	x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
	x ^= (x << 37) & 0xFFF7EEE000000000ULL;
	x ^= (x >> 43);

	return x;
}


long long get_current_utime(void)
{
        struct timeval current;

        gettimeofday(&current,NULL);

        return (current.tv_sec*1000000 + current.tv_usec);
}

/*
 * ./program  path  file_size_1k record_size_1k test_mode
 *
 */

int main(int argc, char* argv[])
{
	long long total_file_size_1k;
	int test_mode;
	double iops = 0;

	path 			= argv[1];		// file path
	file_size_1k 		= atoi(argv[2]);	// file size in Kbyte
	record_size_1k		= atoi(argv[3]);	// record size in Kbyte
	test_mode		= atoi(argv[4]);

	if(test_mode < 0 || test_mode > 3){
		printf("[%s] test mode should be 0 or 1 (current: %d)\n", __FUNCTION__, test_mode);
	}

	if(test_mode == 0){
		create_test_files();
	}
	else if(test_mode == 1){
		open_test_files();
		iops = overwrite_test_files();
	}
	else{
		printf("[%s] Wrong input: %d\n", __FUNCTION__, test_mode);
	}

	printf("%lf\n", iops);

	return 0;
}

void create_test_files(void)
{
	int i;
	long long n_remains;
	long long to_write;
	long long file_size = file_size_1k * 1024;
	char buf[MAX_IO_LENGTH];

	memset(buf, 0xcafe, MAX_IO_LENGTH);
	/* Change directory to path */
	if(chdir(path) == -1){
		printf("[%s] Change directory to %s fail\n", __FUNCTION__, path);
		return;
	}

	/* Open a file */
	fd = open(file_name, O_RDWR | O_CREAT, 0644);
	if(fd == -1){
		printf("[%s] file %s create fail\n", __FUNCTION__, file_name);
		return;
	}

	i = fallocate(fd, 0, 0, file_size); 

	if (i != 0){
		printf("[%s] file %s fallocate is fail (%d)\n", __FUNCTION__, file_name, i);
	}

	/*n_remains = file_size;
	while(n_remains > 0){

		if(n_remains >= MAX_IO_LENGTH)
			to_write = MAX_IO_LENGTH;
		else
			to_write = n_remains;

		if(write(fd, buf, to_write) != to_write){
			printf("[%s] File write fail\n", __FUNCTION__);
			break;
		}

		n_remains -= to_write;
	}*/

	/* synchronize the file */
	fsync(fd);
}

void open_test_files(void)
{
	fp = fopen("io_latency.txt","w");

	/* Change directory to path */
	if(chdir(path) == -1){
		printf("[%s] Change directory to %s fail\n", __FUNCTION__, path);
		return;
	}

	/* Open a file */
	fd = open(file_name, O_RDWR, 0644);
	if(fd == -1){
		printf("[%s] file %s create fail\n", __FUNCTION__, file_name);
		return;
	}
}

double overwrite_test_files(void)
{
	int i;
	long long file_size = file_size_1k * 1024;
	long long record_size = record_size_1k * 1024;
	long long num_recs = file_size_1k / record_size_1k;
	char buf[record_size];
	long long offset;

	long long io_time_start, io_time_end;
	long long start, end;

	memset(buf, 0xefac, record_size);

	io_time_start = get_current_utime();

	for(i = 0; i<num_recs; i++){

		start = get_current_utime();

		if(write(fd, buf, record_size) != record_size){
			printf("[%s] File write fail\n", __FUNCTION__);
			break;
		}

//		fsync(fd);

		end = get_current_utime();

		fprintf(fp,"%d\t%lld\n", i, end-start);
	}

	fclose(fp);

	/* synchronize the file */
	fsync(fd);

	io_time_end = get_current_utime();

	return (double)(num_recs * 1000000)/(io_time_end - io_time_start);
}
