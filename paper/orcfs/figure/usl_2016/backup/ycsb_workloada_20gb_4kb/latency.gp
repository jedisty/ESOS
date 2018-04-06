reset

in1 = "./latency.txt"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 9, 4
set output "latency.eps"

set key opaque right top horizontal enhanced samplen 2 spacing 1

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0

#set yrange [0 : 1.5]
set xrange [-0.5 : 1.5]

#set xlabel "Workload" offset character 2, 0, 0
set ylabel "Normalized Latency" offset character 2, 0, 0

set xtics ("Insert" 0, "Update"1)
#set ytics autofreq 0.3

set logscale y

set style data histogram
set style fill pattern 1.00 border -1
set style histogram errorbars gap 1 linewidth 7

set bars 1
set grid ytics
set boxwidth 0.7 absolute
#set style fill pattern 0 border

plot in1 using 2:3:4 t "Ext4" w hist fill pattern 0, \
                 "" u 5:6:7 t "F2FS" w hist fill pattern 4, \
                "" u 8:9:10 t "OrcFS" w hist fill pattern 1
