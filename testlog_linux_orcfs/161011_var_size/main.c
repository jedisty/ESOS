#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void)
{
	printf("long %lu\n", sizeof(long));
	printf("long long %lu\n", sizeof(long long));
	printf("timespec %ld\n", sizeof(struct timespec));
	return 0;
}
