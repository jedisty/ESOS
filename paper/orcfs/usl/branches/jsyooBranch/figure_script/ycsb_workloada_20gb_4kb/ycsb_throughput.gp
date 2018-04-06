reset

in1 = "ycsb_throughput.txt"

set terminal postscript eps enhanced "Helvetica, 48" monochrome solid size 5, 4 

set xrange [0:6]
set yrange [0:3.5]
#set key spacing 1
#set key outside horizontal top center
#set key bmargin -1 
set key off

set grid ytics lt 1 lc rgb "black"

set ytic(0, 1, 2, 3, 4, 5 ) nomirror
set xtic("EXT4"1, "F2FS"3, "OrcFS"5)  

set ylabel "Ops/sec (x10^3)" offset 1, 0, 0
#set xlabel "Filesystem" 

set output "ycsb_throughput.eps"

set style data histograms
#set style fill solid 1.00 border -1
set style fill pattern 0 border
set boxwidth 1

plot in1  u 1:($2/1024) title "EXT4" with boxes lt -1, \
		'' u 3:($4/1024) title "F2FS"  with boxes lt -1 fill pattern 4, \
		'' u 5:($6/1024) title "OrcFS" with boxes lc 0 lt -1 fill pattern 1
