reset

in1 = "comp_gc_3.data"

set terminal postscript eps enhanced monochrome "Helvetica" 32 size 7.5,3.5

set key right bottom horizontal
set grid y

set xlabel "Iteration" font "Helvetica, 38"
set ylabel "WAF" font "Helvetica, 38"

set xrange[0:8]
set yrange[0:2.25]

set xtic(0, 1, 2, 3, 4, 5, 6, 7, 8)
set ytic(0, 0.5, 1, 1.5, 2, 2.5)

set output "comp_gc_3_waf.eps"
plot in1 u ($1) t 'USL' with linespoint lw 3 lt 4 ps 2, \
	in1 u ($2) t 'F2FS' with linespoint lw 3 lt 9 ps 2
