set terminal postscript eps enhanced monochrome "Helvetica, 48" size 9, 4

in1 = "cold_ratio_iops.txt"

set key right bottom horizontal spacing 2
set grid ytics lt 3

set xlabel "Cold Ratio (%)"
set ylabel "IOPS (K)" offset 1, 0, 0

set yrange [0:150]
set xrange [50:95]

set ytics autofreq 30

set output "cold_ratio_iops.eps"

plot in1 using 1:($2/1024) t "EXT4" w linespoint lw 3 lt 2 ps 4, \
	"" using 1:($3/1024) t "F2FS" w linespoint lw 3 lt 6 ps 4, \
	"" using 1:($4/1024) t "USL" w linespoint lw 3 lt 9 ps 4
