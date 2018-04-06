#!/bin/bash

DEV="/dev/sdb"
DEVP="sdb"
OUTPUTDIR="./result"
MNT="/home/jedisty/f2fs_mnt"
NRUNS=2
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
			mount -t f2fs $DEV $MNT
			;;
		f2fs_da)
			mount -t f2fs $DEV $MNT
			;;
		ext4)
			mount -t ext4 $DEV $MNT
			;;
	esac

	echo "mount device $DEV at $MNT as $FILESYSTEM"
}

getTestFileSize()
{
	#PreOccupySize_1K=`echo $TotalSize_1K*80/100 | bc`
	#ToWriteSize_1K=`echo $TotalSize_1K*15/100 | bc`

	TotalSize_1K=178257920		# 170 GByte
	PreOccupySize_1K=`echo $TotalSize_1K*${COLD_RATIO}/100 | bc`
	HOT_RATIO=`echo 100-${COLD_RATIO} | bc`
	ToWriteSize_1K=`echo $TotalSize_1K*${HOT_RATIO}/100 | bc`

	echo "$PreOccupySize_1K"
	echo "$ToWriteSize_1K"
}

runCompGcTest()
{
        echo "Start Compound GC Test - Make Test Files..."

	mobibench -p ${MNT}/ready -f ${PreOccupySize_1K} -r 512 -a 0 -y 0
	mobibench -p ${MNT}/TEST -f ${ToWriteSize_1K} -r 512 -a 0 -y 0
	
	STATDIR=${OUTPUTDIR}/${FILESYSTEM}/${COLD_RATIO}
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
		cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_init
		;;
	f2fs_da)
		cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
		cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_init;
		cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_init;
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
			cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_${i}
			;;
		f2fs_da)
			cat /proc/fs/f2fs/${DEVP}/waf_info >> $FSWAFNAME;
			cat /proc/fs/f2fs/${DEVP}/segment_info > ${STATDIR}/segment_info_${i};
			cat /proc/fs/f2fs/${DEVP}/valid_blocks_info > ${STATDIR}/valid_blocks_info_${i};
			cat /sys/kernel/debug/f2fs/status > ${STATDIR}/status_${i}
			;;
		esac

		echo "  Iteration ${i} complete"
		sleep 3
	done
}

main()
{
	mkdir ${OUTPUTDIR}

	for FILESYSTEM in f2fs_ori f2fs_da ext4 
	do
		mkdir ${OUTPUTDIR}/${FILESYSTEM}

		for COLD_RATIO in 50 55 60 65 70 75 80 85 90 95
		do
		        echo "Start Intilization for $FILESYSTEM with cold Ratio $COLD_RATIO"

			mkdir ${OUTPUTDIR}/${FILESYSTEM}/${COLD_RATIO}

			# Initialize
			umount ${MNT}
			downloadFirmware
			_insmod
			_mkfs
			_mount
			getTestFileSize
		        echo "  Intilization Complete"

			# Run Test
			smartctl -son $DEV
			runCompGcTest
			echo "$FILESYSTEM cold ratio $COLD_RATIO test End"
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
