set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5, 4

in1 = "hot_ratio_total_waf.txt"

set grid ytics

set xlabel "Hot Ratio (%)"
set ylabel "WAF" offset 1, 0, 0

set yrange [0:2.5]
set xrange [50:5]

set output "hot_ratio_total_waf.eps"

plot in1 using 1:2 t "EXT4" w linespoint lw 3 lt 2 ps 3, \
	"" using 1:3 t "F2FS" w linespoint lw 3 lt 6 ps 3, \
	"" using 1:4 t "USL" w linespoint lw 3 lt 9 ps 3
