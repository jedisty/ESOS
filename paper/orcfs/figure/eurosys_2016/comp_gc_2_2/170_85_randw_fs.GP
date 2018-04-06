reset

in1 = "170_85_randw_fs.data"

set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5,3.5

set key right bottom horizontal
set grid y

set xlabel "Iteration" font "Helvetica, 44"
set ylabel "WAF" font "Helvetica, 44"

set xrange[0:16]
set yrange[0:]

set xtic(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)

set output "170_85_randw_fs.eps"
plot in1 u ($1) t 'F2FS' with linespoint lw 3 lt 4 ps 3, \
	in1 u ($3) t 'USL' with linespoint lw 3 lt 9 ps 4
