reset

in1 = "util_victim_1.txt"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 7.5, 3.2
set output "util_victim_1.eps"

#set key outside opaque left top horizontal enhanced samplen 2 spacing 1
set key outside top horizontal enhanced samplen 2
set key at 1, 1.3
set xtics border in scale 0,0 nomirror 

set yrange [0 : 1.1]
set xrange [-0.05 : 1.05]

set xlabel "Victim Segment Utilization (%)"
set ylabel "Fraction of Segment"

set xtics autofreq 0.2
set xtics ("0"0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)
set ytics autofreq 0.2

set style fill pattern 4.00 border -1
bin(x, s) = s*int(x/s)

num1=system("wc -l util_victim_1.txt | awk '{print $1}'")

#set bars 1
set boxwidth 0.05 absolute

plot in1 using (bin($2, 0.05)):(1./num1) smooth frequency t 'Frequency' with boxes,\
	"" using 2:(1./num1) smooth cumulative t 'Cumulative'
