#!/bin/bash

DEV="/dev/sdb"
DEVP="sdb"
OUTPUTDIR="./result"
MNT="/home/jedisty/f2fs_mnt"
NRUNS=15
FIRMDIR="../F2FS/firmware"
KMODDIR="../F2FS"

downloadFirmware()
{
	case $FILESYSTEM in
		f2fs_ori)
			sudo hdparm --fwdownload ${FIRMDIR}/HYU_OriginalFW_240GB.bin --please-destroy-my-drive --yes-i-know-what-i-am-doing /dev/sdb
			;;
		f2fs_da)
			sudo hdparm --fwdownload ${FIRMDIR}/HYU_DA_FTL_FW_240GB.bin --please-destroy-my-drive --yes-i-know-what-i-am-doing /dev/sdb
			#sudo hdparm --fwdownload ${FIRMDIR}/HYU_OriginalFW_240GB.bin --please-destroy-my-drive --yes-i-know-what-i-am-doing /dev/sdb
			;;
		ext4)
			sudo hdparm --fwdownload ${FIRMDIR}/HYU_OriginalFW_240GB.bin --please-destroy-my-drive --yes-i-know-what-i-am-doing /dev/sdb
			;;
	esac
}

_insmod()
{
	case $FILESYSTEM in
		f2fs_ori)
			rmmod f2fs
			insmod ${KMODDIR}/f2fs_ori/f2fs.ko
			;;
		f2fs_da)
			rmmod f2fs
			insmod ${KMODDIR}/f2fs_da/f2fs.ko
			;;
		ext4)
			;;
	esac

}

_mkfs()
{
	case $FILESYSTEM in
		f2fs_ori)
			mkfs -t f2fs -s 128 $DEV
			echo "mkfs f2fs"
			;;
		f2fs_da)
			mkfs -t f2fs -s 128 -t 0 $DEV
			echo "mkfs f2fs da"
			;;
		ext4)
			mkfs -t ext4 $DEV
			echo "mkfs ext4"
			;;
	esac
}

_mount()
{
	echo 3 > /proc/sys/vm/drop_caches

	case $FILESYSTEM in
		f2fs_ori)
			mount -t f2fs $DEV $MNT -o discard
			;;
		f2fs_da)
			#mount -t f2fs $DEV $MNT
			mount -t f2fs -o background_gc=off $DEV $MNT
			;;
		ext4)
			mount -t ext4 $DEV $MNT
			;;
	esac

	echo "mount device $DEV at $MNT as $FILESYSTEM"
}

getTestFileSize()
{
	TotalSize_1K=`df | grep $DEV | awk '{print $4}'`
	#PreOccupySize_1K=`echo $TotalSize_1K*80/100 | bc`
	#ToWriteSize_1K=`echo $TotalSize_1K*15/100 | bc`
	PreOccupySize_1K=89128960	# 85 GByte
	ToWriteSize_1K=89128960		# 85 GByte
	echo "$ExceptSize_1K"
	echo "$TotalSize_1K"
	echo "$PreOccupySize_1K"
	echo "$ToWriteSize_1K"
}

runCompGcTest()
{
        echo "Start Compound GC Test - Make Test Files..."

	#case ${FILESYSTEM} in
	#ext4)
	#	mobibench -p ${MNT}/except -f ${ExceptSize_1K} -r 2048 -a 0 -y 0
	#	;;
	#esac

	mobibench -p ${MNT}/ready -f ${PreOccupySize_1K} -r 512 -a 0 -y 0
	mobibench -p ${MNT}/TEST -f ${ToWriteSize_1K} -r 512 -a 0 -y 0
	
	STATDIR=${OUTPUTDIR}/${FILESYSTEM}
	DEVWAFNAME=${STATDIR}/device_waf.txt
	FSWAFNAME=${STATDIR}/fs_waf.txt

	# Get SSD WAF Information
	DeviceWrite=`smartctl -A /dev/sdb | grep Wear_Leveling_Count | awk '{print $10}' | bc`
	FsToDevice=`smartctl -A /dev/sdb | grep Total_LBAs_Written | awk '{print $10}' | bc`
	DeviceWrite=`echo $DeviceWrite*8192 | bc`
	FsToDevice=`echo $FsToDevice*512 | bc`
	echo "0 $DeviceWrite $FsToDevice" >> $DEVWAFNAME

	# Get Filesystem Information
	case ${FILESYSTEM} in
	f2fs_ori)
		cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
		cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_init;
		cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_init;
		cat /proc/fs/f2fs/${DEVP}/block_copy_info > ${STATDIR}/block_copy_info_init;
		cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_init
		;;
	f2fs_da)
		cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
		cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_init;
		cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_init;
		cat /proc/fs/f2fs/${DEVP}/block_copy_info > ${STATDIR}/block_copy_info_init;
		cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_init
		;;
	esac
	

       echo "Start Iteration"

	for i in `seq 1 $NRUNS`
	do
		mobibench -p ${MNT}/TEST -f ${ToWriteSize_1K} -r 4 -a 1 -y 0 | grep TIME >> ${STATDIR}/IOPS_test

		# Get SSD WAF Information
		DeviceWrite=`smartctl -A /dev/sdb | grep Wear_Leveling_Count | awk '{print $10}' | bc`
		FsToDevice=`smartctl -A /dev/sdb | grep Total_LBAs_Written | awk '{print $10}' | bc`
		DeviceWrite=`echo $DeviceWrite*8192 | bc`
		FsToDevice=`echo $FsToDevice*512 | bc`
		echo "$i $DeviceWrite $FsToDevice" >> $DEVWAFNAME

		# Get Filesystem Information
		case ${FILESYSTEM} in
		f2fs_ori)
			cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
			cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_${i};
			cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_${i};
			cat /proc/fs/f2fs/${DEVP}/block_copy_info > ${STATDIR}/block_copy_info_${i};

			cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_${i}
			;;
		f2fs_da)
			cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
			cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_${i};
			cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_${i};
			cat /proc/fs/f2fs/${DEVP}/block_copy_info > ${STATDIR}/block_copy_info_${i};
			cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_${i}
			;;
		esac

		echo "  Iteration ${i} complete"
		sleep 3
#TEMP for DBG
		#if [ "$i" -eq 8 ]; then
		#	cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
		#fi
	done
}

main()
{
	mkdir ${OUTPUTDIR}

	for FILESYSTEM in f2fs_da 
	do
	        echo "Start Intilization for $FILESYSTEM"

		umount ${MNT}
		mkdir ${OUTPUTDIR}/${FILESYSTEM}

	# Initialize
		downloadFirmware
		_insmod
		_mkfs
		_mount
		getTestFileSize

	        echo "  Intilization Complete"

	# Run Test
		smartctl -son $DEV
		runCompGcTest

		echo "$FILESYSTEM test End"
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
