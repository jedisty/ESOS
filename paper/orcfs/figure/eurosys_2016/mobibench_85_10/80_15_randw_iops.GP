reset

in1 = "85_10_randw_iops.data"

set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5, 3.5

set key right bottom horizontal
set grid y

set xlabel "Iteration" font "Helvetica, 44"
set ylabel "IOPS (K)" font "Helvetica, 44"

set xrange[0:16]
set yrange[0:90]

set xtic(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)
set ytic(0, 15, 30, 45, 60, 75, 90)

set output "mobibench_randw_iops.eps"
plot in1 u ($1)/1024 t 'EXT4' with linespoint lw 3 lt 4 ps 2, \
	in1 u ($2)/1024 t 'F2FS' with linespoint lw 3 lt 2 ps 3, \
	in1 u ($3)/1024 t 'USL' with linespoint lw 3 lt 9 ps 2
