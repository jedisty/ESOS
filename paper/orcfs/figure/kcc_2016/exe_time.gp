reset

in1 = "./exe_time.txt"

set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 7.5,5
set output "exe_time.eps"

set key opaque left top enhanced samplen 2 spacing 1

set xtics border in scale 0,0 nomirror # offset  character 0.7, 0.2, 0

set yrange [0 : 11]
#set xrange [-0.5 : 4.5]

set xlabel "Hot Ratio (%)" # offset character 2, 0, 0
set ylabel "Run Time (sec)" offset character 1, 0, 0

set xtics ("10" 0, "20"1, "30"2, "40"3, "50"4)
set ytics autofreq 2

set style data histogram
set style histogram cluster gap 1
set style fill pattern 1.00 border -1

set bars 1
set grid ytics
set boxwidth 0.8 absolute

plot in1 using 2 t "SC", \
                 "" u 3 t "FIFO", \
                 "" u 4 t "CFLRU", \
		"" u 5 t "UFLRU"
