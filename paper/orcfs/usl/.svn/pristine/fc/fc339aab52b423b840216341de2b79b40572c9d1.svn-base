set terminal postscript eps enhanced monochrome "Helvetica, 48" size 9, 4

in1 = "cold_ratio_waf.txt"

set key right bottom horizontal spacing 2 
set grid ytics

set xlabel "Cold Ratio (%)"
set ylabel "WAF" offset 1, 0, 0

set yrange [0:2]
set xrange [50:95]

set ytics autofreq 0.4

set output "cold_ratio_waf.eps"

plot in1 using 1:2 t "EXT4" w linespoint lw 3 lt 2 ps 3, \
	"" using 1:3 t "F2FS" w linespoint lw 3 lt 6 ps 3, \
	"" using 1:4 t "OrcFS" w linespoint lw 3 lt 9 ps 3
