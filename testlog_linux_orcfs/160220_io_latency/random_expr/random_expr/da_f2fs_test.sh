#!/bin/bash

DEV="/dev/sdb"
DEVP="sdb"
#OUTPUTDIR="./result"
MNT="/home/jedisty/f2fs_mnt"
NRUNS=1
FIRMDIR="/home/jedisty/F2FS/firmware"

downloadFirmware()
{
	case $FILESYSTEM in
		ext4)
			sudo hdparm --fwdownload ${FIRMDIR}/HYU_OriginalFW_240GB.bin --please-destroy-my-drive --yes-i-know-what-i-am-doing /dev/sdb
			;;
	esac
}

_mkfs()
{
	case $FILESYSTEM in
		ext4)
			mkfs -t ext4 -F $DEV
			echo "mkfs ext4"
			;;
	esac
}

_mount()
{
	echo 3 > /proc/sys/vm/drop_caches

	case $FILESYSTEM in
		ext4)
			mount -t ext4 $DEV $MNT
			;;
	esac

	echo "mount device $DEV at $MNT as $FILESYSTEM"
}

getTestFileSize()
{
	ToWriteSize_1K=2097152	# 170 GByte

	echo "$ToWriteSize_1K"
}

runCompGcTest()
{
        echo "Start Compound GC Test - Make Test Files..."

	# Create Test File
	# ./program  path  file_size_1k  record_size_1k  test_mode
	./comp_gc_test_${TEST_PROG} $MNT $ToWriteSize_1K 4 0

        STATDIR=${OUTPUTDIR}/${FILESYSTEM}

        echo "Start Iteration"

	for i in `seq 1 $NRUNS`
	do
		touch ${STATDIR}/blkresult
		blktrace -d /dev/sdb -o - | blkparse -i - -o ${STATDIR}/blkresult &
		# ./program  path  file_size_1k  record_size_1k  test_mode
		./comp_gc_test_${TEST_PROG} $MNT $ToWriteSize_1K 4 1 >> ${STATDIR}/iops

		echo "  Iteration ${i} complete"
		sleep 3
	done

}

main()
{
#	mkdir ${OUTPUTDIR}

	for TEST_PROG in fsync 
	do
		OUTPUTDIR="./result_${TEST_PROG}"
		mkdir ${OUTPUTDIR}

		echo "Start Test for $TEST_PROG"

		for FILESYSTEM in ext4
		do
		        echo "Start Intilization for $FILESYSTEM"

			umount ${MNT}
			mkdir ${OUTPUTDIR}/${FILESYSTEM}

# Initialize
			downloadFirmware
			_mkfs
			_mount
			getTestFileSize

	        	echo "  Intilization Complete"

# Run Test
			runCompGcTest

			echo "$FILESYSTEM test End"
		done
	done
}


case $1 in
        run)
		main
		;;
        clean)
		rm ${OUTPUTDIR}/*.txt
		;;
        *)      echo "usage: $0 {run|clean}" 
                echo "       run: run the test and save the result under $OUTPUTDIR"
                echo "       clean: erases result file before running the test"
		;;
esac
