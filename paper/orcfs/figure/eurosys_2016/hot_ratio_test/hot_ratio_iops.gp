set terminal postscript eps enhanced monochrome "Helvetica" 48 size 9, 4

in1 = "hot_ratio_iops.txt"

set key right bottom
set grid ytics

set xlabel "Hot Ratio (%)"
set ylabel "IOPS (K)" offset 1, 0, 0

set yrange [0:110]
set xrange [50:5]

set output "hot_ratio_iops.eps"

plot in1 using 1:($2/1000) t "EXT4" w linespoint lw 3 lt 2 ps 4, \
	"" using 1:($3/1000) t "F2FS" w linespoint lw 3 lt 6 ps 4, \
	"" using 1:($4/1000) t "USL" w linespoint lw 3 lt 9 ps 4
