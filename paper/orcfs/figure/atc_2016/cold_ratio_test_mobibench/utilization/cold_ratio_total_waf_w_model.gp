set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5, 4

in1 = "cold_ratio_total_waf.txt"

set grid ytics

set xlabel "Cold Ratio (%)"
set ylabel "WAF" offset 1, 0, 0

set yrange [0:3.0]
set xrange [50:95]

set output "cold_ratio_total_waf_w_model.eps"

plot in1 using 1:2 t "EXT4" w linespoint lw 3 lt 2 ps 3, \
	"" using 1:3 t "F2FS" w linespoint lw 3 lt 6 ps 3, \
	"" using 1:4 t "USL" w linespoint lw 3 lt 9 ps 3
