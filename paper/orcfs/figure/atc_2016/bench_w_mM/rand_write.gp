reset

in1 = "rand_write.txt"

set terminal postscript enhanced color font "Helvetica, 48" size 10, 10

#set xrange [0:6]
set yrange [0:120]
#set key spacing 1
#set key outside horizontal top center
#set key bmargin -1 
set key off

set ytic(0, 20, 40, 60, 80, 100 ) font "Helvetica, 48" nomirror
set xtic("EXT4"1, "F2FS"3, "USL"5)  font "Helvetica, 44"

set ylabel "IOPS (K)" font "Helvetica, 48" offset 1, 0, 0
#set xlabel "Fileystem" font "Helvetica, 48"

set output "rand_write.eps"

set grid ytics
set style histogram errorbars gap 1
#set style data histogram errorbars
#set style fill solid 1.00 border -1
set style fill pattern 0 border
set boxwidth 0.7

plot in1  u 2:3:4 title "EXT4" with hist lt -1, \
		'' u 6:7:8 title "F2FS"  with hist lt -1 fill pattern 4, \
		'' u 10:11:12 title "USL" with hist lt -1 lc 0 fill solid 
