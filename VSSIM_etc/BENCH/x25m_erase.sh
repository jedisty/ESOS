sudo hdparm --user-master u --security-set-pass test /dev/sdb
sudo hdparm --user-master u --security-erase test /dev/sdb

sudo mkfs.ext4 /dev/sdb
mkdir ssd
sudo mount -t ext4 /dev/sdb ./ssd
sudo chmod 0777 ssd
