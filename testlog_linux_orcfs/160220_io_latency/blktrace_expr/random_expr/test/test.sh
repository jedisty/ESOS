blktrace -d /dev/sda -o - | blkparse -i - -o blkresult.txt &
blk_pid=`ps -ef | grep "blktrace" | awk '{print $2}'`
echo $blk_pid
#kill $blk_pid
