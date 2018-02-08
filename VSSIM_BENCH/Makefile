CC = gcc
TARGET = vssim_bench
CFLAGS = -lpthread -D_GNU_SOURCE -D_THREAD_SAFE

all: vssim_bench

ftl.o : ftl.c common.h
	$(CC) -c ftl.c
ftl_mt.o : ftl_mt.c common.h
	$(CC) -c ftl_mt.c

ftl_mapping_manager.o : ftl_mapping_manager.c common.h
	$(CC) -c ftl_mapping_manager.c

ftl_inverse_mapping_manager.o : ftl_inverse_mapping_manager.c common.h
	$(CC) -c ftl_inverse_mapping_manager.c

ftl_gc_manager.o : ftl_gc_manager.c common.h
	$(CC) -c ftl_gc_manager.c

ftl_cache.o : ftl_cache.c common.h
	$(CC) -c ftl_cache.c

ftl_perf_manager.o : ftl_perf_manager.c common.h
	$(CC) -c ftl_perf_manager.c


ssd_io_manager.o : ssd_io_manager.c common.h
	$(CC) -c ssd_io_manager.c

ssd_log_manager.o : ssd_log_manager.c common.h
	$(CC) -c ssd_log_manager.c

ssd_io_manager_mt.o : ssd_io_manager_mt.c common.h
	$(CC) -c ssd_io_manager_mt.c

firm_buffer_manager.o : firm_buffer_manager.c common.h 
	$(CC) -c $(CFLAGS) firm_buffer_manager.c

vssim_config_manager.o : vssim_config_manager.c common.h
	$(CC) -c vssim_config_manager.c

vssim_bench : vssim_bench.c ftl.o ftl_mt.o ftl_mapping_manager.o ftl_inverse_mapping_manager.o ftl_gc_manager.o ftl_cache.o ftl_perf_manager.o ssd_io_manager.o ssd_log_manager.o firm_buffer_manager.o vssim_config_manager.o ssd_io_manager_mt.o
	$(CC) $(CFLAGS) -o vssim_bench vssim_bench.c ftl.o ftl_mt.o ftl_mapping_manager.o ftl_inverse_mapping_manager.o ftl_gc_manager.o ftl_cache.o ftl_perf_manager.o ssd_io_manager.o ssd_log_manager.o firm_buffer_manager.o vssim_config_manager.o ssd_io_manager_mt.o

clean:
	rm -rf *.o $(TARGET)
