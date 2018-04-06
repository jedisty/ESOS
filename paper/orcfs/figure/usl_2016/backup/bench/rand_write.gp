reset

in1 = "rand_write.txt"

set terminal postscript enhanced color font "Helvetica, 48" size 10, 10

set xrange [0:6]
set yrange [0:120]
#set key spacing 1
#set key outside horizontal top center
#set key bmargin -1 
set key off

set ytic(0, 20, 40, 60, 80, 100 ) font "Helvetica, 48" nomirror
set xtic("EXT4"1, "F2FS"3, "OrcFS"5)  font "Helvetica, 44"

set ylabel "IOPS (K)" font "Helvetica, 48" offset 1, 0, 0
#set xlabel "Fileystem" font "Helvetica, 48"

set output "rand_write.eps"

set grid ytics
set style data histograms
#set style fill solid 1.00 border -1
set style fill pattern 0 border
set boxwidth 1

plot in1  u 1:2 title "EXT4" with boxes lt -1, \
		'' u 3:4 title "F2FS"  with boxes lt -1 fill pattern 4, \
		'' u 5:6 title "OrcFS" with boxes lt -1 lc 0 fill pattern 1 
