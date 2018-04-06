reset

in1 = "./throughput.txt"

set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 9,4
set output "throughput.eps"

set key opaque right top horizontal enhanced samplen 2 spacing 1

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0

set yrange [0 : 5]
set xrange [-0.5 : 1.5]

#set xlabel "Workload" offset character 2, 0, 0
set ylabel "Ops/sec (K)" offset character 0, 0, 0

set xtics ("Insert" 0, "Read / Update" 1)
set ytics autofreq 1

set style data histogram
set style histogram cluster gap 1
set style fill pattern 1.00 border -1

set bars 1
set grid ytics
set boxwidth 0.7 absolute
set style fill pattern 0 border

plot in1 using ($2/1000) t "Ext4", \
                 "" u ($3/1000) t "F2FS" fill pattern 4, \
                 "" u ($4/1000) t "OrcFS"  fill pattern 1 lc 0
