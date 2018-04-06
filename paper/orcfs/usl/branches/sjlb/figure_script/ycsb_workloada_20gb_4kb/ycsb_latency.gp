reset

in1 = "./ycsb_latency.txt"

set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 5,4 
set output "ycsb_latency.eps"

#set key opaque right top horizontal enhanced samplen 2 spacing 1

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0
set key off

#set yrange [0 : 1.5]
set yrange [0.1 : 1000]
set xrange [-0.4 : 0.4]

#set xlabel "Workload" offset character 2, 0, 0
set ylabel "Normalized Latency" offset 2, 0, 0 

set xtics ("Ext4"-0.3, "F2FS"0, "OrcFS"0.3)
set ytics (0, "  1"1, "  10^{1}"10, "  10^{2}"100, "  10^{3}"1000, "  10^{4}"10000)

set logscale y

set style data histogram
set style fill pattern 1.00 border -1
set style histogram errorbars gap 1 linewidth 7

set bars 1
set grid ytics lt 3
set boxwidth 0.7 absolute
#set style fill pattern 0 border

# max
set label at -0.31, 700  "435"
set label at -0.06, 690  "433"
set label at  0.2, 500  "332"

# mean 
set label at -0.31, 1.3  ""
set label at 0.03, 1.3  "0.82"
set label at  0.27, 1.3  "0.76"

# min
set label at -0.31, 0.3  "0.47"
set label at -0.06, 0.3  "0.46"
set label at  0.2, 0.3  "0.44"

plot in1 using 2:3:4 t "Ext4" w hist fill pattern 0, \
                 "" u 5:6:7 t "F2FS" w hist fill pattern 4, \
                "" u 8:9:10 t "OrcFS" w hist fill pattern 1

