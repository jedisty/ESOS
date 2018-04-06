reset

in1 = "./orcfs.txt"
in2 = "./orcfs_qpsc.txt"

set terminal postscript eps enhanced "Helvetica,48"  monochrome size 9, 4
set termoption dash
set output "orcfs_qpsc_cdf.eps"

set key right bottom height 1.2

set ylabel "Cumulative Percent (%)" font "Helvetica, 48"
set xlabel "Write Latency (msec)" font "Helvetica, 48"

#set xrange[0:]
set yrange[0:1.1]

set logscale x

set style line 1 lt 3 lc rgb "red" lw 10

set grid ytics

#set xtic (0, "1" 1000, "2" 2000, "3" 3000, "4" 4000, "5" 5000, "6" 6000, "7" 7000, "8" 8000, "9" 9000)
set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l orcfs.txt | awk '{print $1}'")
num2=system("wc -l orcfs_qpsc.txt | awk '{print $1}'")


plot in1 using ($2):(1./num1) s cumul t 'USL' with line linetype 1 linewidth 10,\
in2 using ($2):(1./num2) s cumul t 'USL_{QPSC}' ls 1
