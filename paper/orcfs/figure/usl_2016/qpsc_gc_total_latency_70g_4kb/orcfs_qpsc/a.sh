NRUNS=`wc -l block_copy_info_init | awk '{print $1}'`

head -n 1 block_copy_info_init > block_copy_info_2

for i in `seq 2 $NRUNS`
do
	if [ `head -n $i block_copy_info_init | tail -n 1 | awk '{print $8}'` -ne "0" ]; then
		head -n $i block_copy_info_init | tail -n 1 >> block_copy_info_2
	fi
done
