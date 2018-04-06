reset

in1 = "seq_write.txt"

set terminal postscript enhanced color font "Helvetica, 48" size 10, 10

set xrange [0:6]
set yrange [0:550]
#set key spacing 1
#set key outside horizontal top center
#set key bmargin -1 
set key off

set grid ytics lt 1 lc rgb "black"

set ytic(0, 100, 200, 300, 400, 500 ) font "Helvetica, 48" nomirror
set xtic("EXT4"1, "F2FS"3, "OrcFS"5)  font "Helvetica, 44"

set ylabel "Bandwidth (MB/s)" font "Helvetica, 48"
#set xlabel "Filesystem" font "Helvetica, 48"

set output "seq_write.eps"

set style data histograms
#set style fill solid 1.00 border -1
set style fill pattern 0 border
set boxwidth 1

plot in1  u 1:2 title "EXT4" with boxes lt -1, \
		'' u 3:4 title "F2FS"  with boxes lt -1 fill pattern 4, \
		'' u 5:6 title "OrcFS" with boxes lc 0 lt -1 fill pattern 1 
