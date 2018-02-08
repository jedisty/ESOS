#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define __USE_GNU
#include <fcntl.h>
#include <time.h>
#include <string.h> 

#define MERSENNE

#define DEVICE "/dev/sdb"
#define FILENAME    "/dev/sdb"
#define FILENAMESIO "/dev/sdb"
#define FILENAMEDIO "/dev/sdb"
#define FILENAMESYDI "/dev/sdb"

#define FILE_POS 0x38A40000 //950272000

#define SIZE_100MB 104857600
#define SIZE_1MB 1048576
#define SIZE_4KB 4096
#define SIZE_512KB 524288
//#define WRITE_LAT_DEBUG


long long kilo64; //file size
long int counter=0 ; 
long long reclen_byte;

typedef enum
{
	MODE_WRITE,
	MODE_REWRITE,
	MODE_RND_WRITE,
	MODE_RND_READ,
	MODE_READ
} file_test_mode_t;

typedef enum
{
	NORMAL,
	OSYNC,
	FSYNC,
	ODIRECT,
	SYDI
} file_sync_mode_t;

void print_time(struct timeval T1, struct timeval T2)
{
	long sec,usec;
	double time;
	double rate;
	
	time_t t;
	
	if(T1.tv_usec > T2.tv_usec)
	{
		sec = T2.tv_sec - T1.tv_sec -1;
		usec = 1000000 + T2.tv_usec - T1.tv_usec;
	}
	else
	{
		sec = T2.tv_sec - T1.tv_sec;
		usec = T2.tv_usec - T1.tv_usec;
	}

	time = (double)sec + (double)usec/1000000;
	
	rate = kilo64*1024/time;
	
	printf("Elapsed Time : %8ld sec %4ldus. %.0f B/sec, %.2f KB/sec, %.2f MB/sec. \n\n",sec,usec, rate, rate/1024, rate/1024/1024);
	
}

double calc_time(struct timeval T1, struct timeval T2)
{
	long sec,usec;
	double time;
	double rate;
	
	time_t t;
	
	if(T1.tv_usec > T2.tv_usec)
	{
		sec = T2.tv_sec - T1.tv_sec -1;
		usec = 1000000 + T2.tv_usec - T1.tv_usec;
	}
	else
	{
		sec = T2.tv_sec - T1.tv_sec;
		usec = T2.tv_usec - T1.tv_usec;
	}

	time = (double)sec + (double)usec/1000000;

	return time;	
}

void print_latency(struct timeval T1, struct timeval T2)
{
	long sec,usec;
	double time;
	double rate;
	
	time_t t;
	
	if(T1.tv_usec > T2.tv_usec)
	{
		sec = T2.tv_sec - T1.tv_sec -1;
		usec = 1000000 + T2.tv_usec - T1.tv_usec;
	}
	else
	{
		sec = T2.tv_sec - T1.tv_sec;
		usec = T2.tv_usec - T1.tv_usec;
	}

	time = (double)sec + (double)usec/1000000;
	printf("%ld: %lf\tsec\n",counter++, time);
}

#define DEF_PROCSTAT_BUFFER_SIZE (1 << 10) /* 1 KBytes (512 + 512 ^^) */ 

#define START_CPU_CHECK 0
#define END_CPU_CHECK 1

unsigned long s_CPUTick[2][6]; 

void cpuUsage(int startEnd)
{
	const char *s_ProcStat = "/proc/stat"; 
	const char *s_CPUName = "cpu "; 
	int s_Handle, s_Check, s_Count; 
	char s_StatBuffer[ DEF_PROCSTAT_BUFFER_SIZE ]; 
	char *s_String; 
	float s_DiffTotal; 
	unsigned long active_tick, idle_tick, wait_tick;
	
	s_Handle = open(s_ProcStat, O_RDONLY); 
		
	if(s_Handle >= 0) 
	{ 
		s_Check = read(s_Handle, &s_StatBuffer[0], sizeof(s_StatBuffer) - 1); 
		
		s_StatBuffer[s_Check] = '\0'; 

		s_String = strstr(&s_StatBuffer[0], s_CPUName); /* Total CPU entry */ 
				
		//printf("s_String=%s\n", s_String);

		if(s_String) 
		{ 
			s_Check = sscanf(s_String, "cpu %lu %lu %lu %lu %lu", &s_CPUTick[startEnd][0], &s_CPUTick[startEnd][1], &s_CPUTick[startEnd][2], &s_CPUTick[startEnd][3], &s_CPUTick[startEnd][4]); 

			//printf("s_Check=%d\n", s_Check);

			if(s_Check == 5) 
			{ 
				for(s_Count = 0, s_CPUTick[startEnd][5] = 0lu; s_Count < 5;s_Count++)
					s_CPUTick[startEnd][5] += s_CPUTick[startEnd][s_Count]; 
			}
					
			//printf("[CPU] 0=%ld, 1=%ld, 2=%ld, 3=%ld, 4=%ld, \n", s_CPUTick[startEnd][0], s_CPUTick[startEnd][1], s_CPUTick[startEnd][2], s_CPUTick[startEnd][3], s_CPUTick[startEnd][4]);
		}
	}
		
	//printf("[CPU] 0=%ld, 1=%ld, 2=%ld, 3=%ld, 4=%ld, \n", s_CPUTick[startEnd][0], s_CPUTick[startEnd][1], s_CPUTick[startEnd][2], s_CPUTick[startEnd][3], s_CPUTick[startEnd][4]);
		
	if(startEnd == END_CPU_CHECK)
	{
		s_DiffTotal = (float)(s_CPUTick[END_CPU_CHECK][5] - s_CPUTick[START_CPU_CHECK][5]); 
		active_tick = (s_CPUTick[END_CPU_CHECK][0] - s_CPUTick[START_CPU_CHECK][0]) + (s_CPUTick[END_CPU_CHECK][1] - s_CPUTick[START_CPU_CHECK][1]) + (s_CPUTick[END_CPU_CHECK][2] - s_CPUTick[START_CPU_CHECK][2]);
		idle_tick = (s_CPUTick[END_CPU_CHECK][3] - s_CPUTick[START_CPU_CHECK][3]);
		wait_tick = (s_CPUTick[END_CPU_CHECK][4] - s_CPUTick[START_CPU_CHECK][4]);
						
		printf("[CPU TICK] Active=%ld, Idle=%ld, IoWait=%ld\n",active_tick, idle_tick, wait_tick);
		printf("[CPU] Active=%1.2f%%, Idle=%1.2f%%, IoWait=%1.2f%%\n",
						(float)( (float)(active_tick * 100lu) / s_DiffTotal ),
						(float)( (float)(idle_tick * 100lu) / s_DiffTotal ), 
						(float)( (float)(wait_tick * 100lu) / s_DiffTotal ) ); 
	}
		
	close(s_Handle);			
}

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
        if (mti == NN+1) 
            init_genrand64(5489ULL); 

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

void init_by_array64(unsigned long long init_key[],
		     unsigned long long key_length)
{
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i=1; j=0;
    k = (NN>key_length ? NN : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))
          + init_key[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=NN-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL))
          - i; /* non linear */
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }

    mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */ 
}

void MERSENNE_test()
{
	unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
	unsigned long long length=4;
	long long kilo64;
	long long reclen;
	long long numrecs64;
		
    long long *recnum= 0;
    long long i;
    unsigned long long big_rand;
    
    kilo64=1024;
    reclen=1024;
    
    numrecs64 = (kilo64*1024)/reclen;	
    
    init_by_array64(init, length);
    
    recnum = (long long *)malloc(sizeof(*recnum)*numrecs64);
    
    if (recnum){
             /* pre-compute random sequence based on 
		Fischer-Yates (Knuth) card shuffle */
            for(i = 0; i < numrecs64; i++){
                recnum[i] = i;
            }
            for(i = 0; i < numrecs64; i++) {
                long long tmp;

      	       big_rand=genrand64_int64();

               big_rand = big_rand%numrecs64;
               tmp = recnum[i];
               recnum[i] = recnum[big_rand];
               recnum[big_rand] = tmp;
            }
        }
	else
	{
		fprintf(stderr,"Random uniqueness fallback.\n");
	}

	#if 0
	for(i = 0; i < numrecs64; i++) {
		
		printf("%lld\n",	recnum[i]);
	}
	#endif

}

int main( int argc, char **argv)
{
	void *buf;
	int ret = 0;
	int fd, fd2, fd3, fd4, fd5;
	int ps = getpagesize();
	int time;
	int count;
	struct timeval T1, T2, Latency1, Latency2;
	struct timeval start, end;
	int vssim_ret;
	int num_4k;

	unsigned long long init[4]={0x12345ULL, 0x23456ULL, 0x34567ULL, 0x45678ULL};
	unsigned long long length=4;
	long long reclen;
	long long numrecs64;
		
	long long *recnum= 0;
	long long i;
	unsigned long long big_rand;
	long long offset;
	int access, sync;

	double vssim_time;

	if(argc!=5)
	{
	 		printf("Need 4 inputs : File size(KByte), reclen(KByte), AccessMode(0:WRITE, 1:REWRITE, 2:RANDOM_WRITE 3:RANDOM_READ 4:READ), Sync Mode(0:Normal, 1: O_SYNC,  2: fsync   3: O_DIRECT    4: Sync+direct IO)\n\n");
	 		return;
	}	
  
 	kilo64 = atoi(argv[1]);
 	reclen = atoi(argv[2]);
 	access = atoi(argv[3]);
 	sync = atoi(argv[4]);
    
	numrecs64 = kilo64/reclen;	
    
	init_by_array64(init, length);
    
	recnum = (long long *)malloc(sizeof(*recnum)*numrecs64);

#ifdef WRITE_LAT_DEBUG	
	FILE* fp_sync;
	FILE* fp_direct;
	FILE* fp_sync_rand;
	FILE* fp_direct_rand;
	FILE* fp_noop;
	FILE* fp_noop_rand;
	FILE* fp_sydi;
	FILE* fp_sydi_rand;
	FILE* fp_read;
	FILE* fp_read_rand;

	if(sync == OSYNC && access == MODE_WRITE) {
		fp_sync = fopen("./sync.txt", "a");
	}
	else if ( sync == ODIRECT && access == MODE_WRITE) {
		fp_direct = fopen("./direct.txt", "a");
	}
	else if ( sync == OSYNC && access == MODE_RND_WRITE ) {
		fp_sync_rand = fopen("./sync_rand.txt", "a");
	}
	else if ( sync == ODIRECT && access == MODE_RND_WRITE) {
		fp_direct_rand = fopen("./direct_rand.txt", "a");
	}
	else if ( sync == NORMAL && access == MODE_WRITE){
		fp_noop = fopen("./noop.txt", "a");
	}
	else if ( sync == NORMAL && access == MODE_RND_WRITE){
		fp_noop_rand = fopen("./noop_rand.txt", "a");
	}
	else if ( sync == SYDI && access == MODE_WRITE){
		fp_sydi = fopen("./sydi.txt", "a");
	}
	else if ( sync == SYDI && access == MODE_RND_WRITE){
		fp_sydi_rand = fopen("./sydi_rand.txt", "a");
	}
	else if ( access == MODE_READ){
		fp_read = fopen("./read.txt", "a");
	}
	else if ( access == MODE_RND_READ){
		fp_read_rand = fopen("./read_rand.txt", "a");
	}
#endif
	if (recnum){
		/* pre-compute random sequence based on 
		  Fischer-Yates (Knuth) card shuffle */
		for(i = 0; i < numrecs64; i++){
			recnum[i] = i;
		}
		for(i = 0; i < numrecs64; i++) {
			long long tmp;

			big_rand=genrand64_int64();

			big_rand = big_rand%numrecs64;
			tmp = recnum[i];
			recnum[i] = recnum[big_rand];
			recnum[big_rand] = tmp;
		}
	}
	else
	{
		fprintf(stderr,"Random uniqueness fallback.\n");
	}

 	if(sync == OSYNC) {
 		fd = open(FILENAMESIO,  O_RDWR | O_CREAT | O_SYNC);
                printf("\n");
                printf("Opened in Synchronous IO mode\n");
        }
        else if ( sync == ODIRECT ) {
                fd = open(FILENAMEDIO,  O_RDWR | O_CREAT | O_DIRECT);
                printf("\n");
                printf("Opened in direct IO mode\n");
        }
        else if ( sync == SYDI ) {
                fd = open(FILENAMESYDI, O_RDWR | O_CREAT | O_SYNC | O_DIRECT);
                printf("\n");
                printf("Opened in SYDI mode\n");
        }
	else if (sync == NORMAL) {
		fd = open(FILENAME,  O_RDWR | O_CREAT);
		printf("\n");
		printf("Opened in normal mode\n");
	}
	else {
		fd = open(FILENAME,  O_RDWR);
	}
	
	if(fd <0)
	{
		printf("Open failed\n");
		exit(ret);
	}
	reclen_byte = reclen * 1024;
	num_4k = reclen/4;

	if ( sync == ODIRECT || sync == SYDI ){
		if (( ret = posix_memalign( &buf, SIZE_4KB, SIZE_4KB*num_4k )))
		{
			printf("Memalign failed\n");
			exit(ret);
		} 
	}
	else {
		buf = malloc(reclen_byte);
	}

	memset(buf, 0xcafe, reclen_byte);
		
	printf("File size %lld KB, Reclen %lld KB, Write count %lld \n", kilo64, reclen, numrecs64);
	printf("Access Mode %d, Sync Mode %d\n", access, sync);
 
	if(access == MODE_READ || access == MODE_RND_READ){
		for(i=0; i<numrecs64; i++)
		{
			vssim_ret = write(fd, buf, reclen_byte);
			if(vssim_ret<0)
				printf("File write error!!!\n");
		}
		if(lseek(fd, 0, SEEK_SET)==-1){
			printf("lseek error!!!\n");
			exit(ret);
		}
		sleep(3);
	}

	/* Start */
	gettimeofday(&T1,NULL);
	cpuUsage(START_CPU_CHECK);
	
	if((access == MODE_WRITE || access == MODE_REWRITE) && sync==FSYNC)
	{
		printf("SEQ WRITE & FSYNC!!!\n");

		for(i=0; i<numrecs64; i++)
		 {
		 	if(write(fd, buf, reclen_byte)<0)
				printf("File write error!!!\n");
			
			fsync(fd); 	 	 		
		 }	
	}
	else if((access == MODE_WRITE || access == MODE_REWRITE))
	{
		printf("SEQ WRITE!!!\n");
		
		if(sync == OSYNC){
			for(i=0; i<numrecs64; i++)
			{
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end, NULL);
				if(vssim_ret<0)
					printf("File write error!!!\n");
				vssim_time = calc_time(start,end);
				fprintf(fp_sync,"%lf\n",vssim_time);
#else
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret<0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == ODIRECT){
			for(i=0; i<numrecs64; i++)
			{
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end, NULL);
				if(vssim_ret<0)
					printf("File write error!!!\n");
				vssim_time = calc_time(start,end);
				fprintf(fp_direct,"%lf\n",vssim_time);
#else
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret<0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == NORMAL){
			for(i=0; i<numrecs64; i++)
			{
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end, NULL);
				if(vssim_ret<0)
					printf("File write error!!!\n");
				vssim_time = calc_time(start,end);
				fprintf(fp_noop,"%lf\n",vssim_time);
#else			
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret<0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == SYDI){
			for(i=0; i<numrecs64; i++)
			{
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end, NULL);
				if(vssim_ret<0)
					printf("File write error!!!\n");
				vssim_time = calc_time(start,end);
				fprintf(fp_sydi,"%lf\n",vssim_time);
#else			
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret<0)
					printf("File write error!!!\n");
#endif
			}
		}
	}	
	else if((access == MODE_RND_WRITE) && sync==FSYNC)
	{
		printf("RANDOM WRITE & FSYNC!!!\n");
		
		for(i=0; i<numrecs64; i++)
		{
			offset = (long long)recnum[i]*1024*reclen;
		 	 
		 	if(lseek(fd, offset, SEEK_SET)==-1)
			{
				printf("lseek error!!!\n");
				exit(ret);
			}
					
			if(write(fd, buf, reclen_byte)<0)
				printf("File write error!!!\n");
			
			fsync(fd); 	 	 		
		}
	}
	else if((access == MODE_RND_WRITE))
	{
		printf("RANDOM WRITE!!!\n");
	        char MODENAME[]="RND_WRITE";	 
                counter = 0; 

		if(sync == OSYNC){
			for(i=0; i<numrecs64; i++)
			{
				offset = (long long)recnum[i]*1024*reclen;

				if(lseek(fd, offset, SEEK_SET)==-1)
				{
					printf("lseek error!!!\n");
					exit(ret);
				}
#ifdef WRITE_LAT_DEBUG		
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end,NULL);

				if(vssim_ret < 0)
					printf("File write error!!!\n");

				vssim_time = calc_time(start,end);
				fprintf(fp_sync_rand,"%lf\n",vssim_time);
#else
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret < 0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == ODIRECT){

			for(i=0; i<numrecs64; i++)
			{
				offset = (long long)recnum[i]*1024*reclen;
		 	 
		 		if(lseek(fd, offset, SEEK_SET)==-1)
		 		{
					printf("lseek error!!!\n");
					exit(ret);
				}
#ifdef WRITE_LAT_DEBUG	
                	        gettimeofday(&start,NULL);
		 		vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end,NULL);

				if(vssim_ret < 0)
					printf("File write error!!!\n");

				vssim_time = calc_time(start,end);
				fprintf(fp_direct_rand,"%lf\n",vssim_time);
#else
		 		vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret < 0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == NORMAL){

			for(i=0; i<numrecs64; i++)
			{
				offset = (long long)recnum[i]*1024*reclen;

				if(lseek(fd, offset, SEEK_SET)==-1)
				{
					printf("lseek error!!!\n");
					exit(ret);
				}
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end,NULL);

				if(vssim_ret < 0)
					printf("File write error!!!\n");

				vssim_time = calc_time(start,end);
				fprintf(fp_noop_rand,"%lf\n",vssim_time);
#else
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret < 0)
					printf("File write error!!!\n");
#endif
			}
		}
		else if(sync == SYDI){

			for(i=0; i<numrecs64; i++)
			{
				offset = (long long)recnum[i]*1024*reclen;

				if(lseek(fd, offset, SEEK_SET)==-1)
				{
					printf("lseek error!!!\n");
					exit(ret);
				}
#ifdef WRITE_LAT_DEBUG
				gettimeofday(&start,NULL);
				vssim_ret = write(fd, buf, reclen_byte);
				gettimeofday(&end,NULL);

				if(vssim_ret < 0)
					printf("File write error!!!\n");

				vssim_time = calc_time(start,end);
				fprintf(fp_sydi_rand,"%lf\n",vssim_time);
#else
				vssim_ret = write(fd, buf, reclen_byte);
				if(vssim_ret < 0)
					printf("File write error!!!\n");
#endif
			}
		}
	}
	else if((access == MODE_READ))
	{
		printf("Seq READ!!!\n");
		for(i=0; i<numrecs64; i++)
		{
#ifdef WRITE_LAT_DEBUG
			gettimeofday(&start,NULL);
			vssim_ret = read(fd, buf, reclen_byte);
			gettimeofday(&end, NULL);

			if(vssim_ret<0)
				printf("File read error!!!\n");

			vssim_time = calc_time(start,end);
			fprintf(fp_read,"%lf\n",vssim_time);
#else
			vssim_ret = read(fd, buf, reclen_byte);
			if(vssim_ret<0)
				printf("File write error!!!\n");
#endif
		}
	}
	else if((access == MODE_RND_READ))
	{
		printf("RANDOM READ!!!\n");
		char MODENAME[]="RND_READ";	 
		counter = 0; 
		
		for(i=0; i<numrecs64; i++)
		{
			offset = (long long)recnum[i]*1024*reclen;
			if(lseek(fd, offset, SEEK_SET)==-1)
			{
				printf("lseek error!!!\n");
				exit(ret);
			}
#ifdef WRITE_LAT_DEBUG
			gettimeofday(&start,NULL); 
			vssim_ret = read(fd, buf, reclen_byte);
			gettimeofday(&end,NULL);

			if(vssim_ret < 0)
				printf("File rand read error!!!\n");

			vssim_time = calc_time(start,end);
			fprintf(fp_read_rand,"%lf\n",vssim_time);	
#else
			vssim_ret = read(fd, buf, reclen_byte);
			if(vssim_ret < 0)
				printf("File rand read error!!!\n");
#endif
		}
	}	
	else{
		printf("Bad combination!!!\n");
		exit(ret);
	}

	gettimeofday(&T2,NULL);
	cpuUsage(END_CPU_CHECK);
	print_time(T1, T2);
 
	close(fd);
	
	free(buf);
 
 	if(recnum)
		free(recnum);

}
