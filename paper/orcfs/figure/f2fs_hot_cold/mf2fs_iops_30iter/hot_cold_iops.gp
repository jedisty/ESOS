set terminal postscript eps enhanced monochrome "Helvetica, 44" size 9, 4

in1 = "hot_cold_iops.txt"

#set key outside right top horizontal spacing 2
#set key right top horizontal
set key right top vertical
set grid ytics lt 1

set xlabel "Iteration"
set ylabel "IOPS (K)" offset 1, 0, 0

set yrange [0:110]
set xrange [0:31]

set xtics autofreq 2
set ytics autofreq 20

set output "hot_cold_iops.eps"

plot in1 using 1:($2/1024) t "F2FS(.mp4)" w linespoint lw 3 lt 2 ps 3, \
	"" using 1:($3/1024) t "M F2FS(N.E.)" w linespoint lw 3 lt 6 ps 3, \
	"" using 1:($4/1024) t "M F2FS(.mp4)" w linespoint lw 3 lt 9 ps 3
