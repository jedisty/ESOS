reset

in1 = "170_85_randw_total.data"

set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5, 4

set key right bottom maxrows 2 width -6
set grid y

set xlabel "Iteration" font "Helvetica, 44"
set ylabel "WAF" font "Helvetica, 44"

set xrange[0:16]
set yrange[0:]

set xtic(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)

set output "170_85_randw_total.eps"
plot in1 u ($1) t 'f2fs ori' with linespoint lw 3 lt 4 ps 2, \
	in1 u ($2) t 'f2fs w/o ssr' with linespoint lw 3 lt 6 ps 2, \
	in1 u ($3) t 'f2fs da' with linespoint lw 3 lt 9 ps 2
