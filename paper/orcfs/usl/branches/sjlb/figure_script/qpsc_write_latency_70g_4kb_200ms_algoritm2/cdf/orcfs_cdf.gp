reset

in1 = "../OrcFS_Normal/result"
in2 = "../OrcFS_QPSC/result"

set terminal postscript eps enhanced "Helvetica,48"  dash dashlength 1.1 color size 9, 4
set output "orcfs_qpsc_cdf.eps"

set key inside right bottom height 1.2

set ylabel "Cumulative\nPercent (%)" font "Helvetica, 48"
set xlabel "Write Latency (usec)" font "Helvetica, 48"

set xrange[0.1:]
set yrange[0: 1.1]

set logscale x

set style line 1 lt 3 lc rgb "red" lw 10

set grid ytics

set xtic ("0.1" 0.1, "1"1, "10" 10, "10^2" 100, "10^3" 1000, "10^4" 10000, "10^5" 100000, "10^6" 1000000, "10^7" 10000000)
set ytic ("0" 0 , "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l ../OrcFS_Normal/result | awk '{print $1}'")
num2=system("wc -l ../OrcFS_QPSC/result | awk '{print $1}'")


plot in1 using ($2):(1./num1) s cumul t 'OrcFS' with line linetype 3 linewidth 8 linecolor "black", \
     in2 using ($2):(1./num2) s cumul t 'OrcFS_{QPSC}' with line linetype 1 linewidth 6 lc "red"

