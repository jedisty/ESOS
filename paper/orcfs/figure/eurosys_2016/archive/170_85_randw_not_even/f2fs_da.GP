reset

in1 = "f2fs_da.data"

set terminal postscript eps enhanced color "Helvetica" 25 size 7.5,3.5

set key left top

set xlabel "Iteration" font "Helvetica, 28"
set ylabel "WAF" font "Helvetica, 28"

set xrange[0:6]
set yrange[0:]

set xtic(0, 1, 2, 3, 4, 5)

set output "f2fs_da.eps"
plot in1 u ($1) t 'filesystem waf' with linespoint lw 2, \
	in1 u ($2) t 'device waf' with linespoint, \
	in1 u ($3) t 'total waf' with linespoint
