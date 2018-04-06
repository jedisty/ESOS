in1 = "./normal_result"
in2 = "./qpsc_result"

#set terminal wxt 0
set key left top

set ylabel "Percentage" font "Helvetica, 48"
set xlabel "Latency (usec)" font "Helvetica, 48"

#set xrange[0:]
#set yrange[0:1.1]

#set xtic (0, "10"10*1024*1024/4, "20"20*1024*1024/4, "30"30*1024*1024/4, "40"40*1024*1024/4, "50"50*1024*1024/4, "60"60*1024*1024/4, "70"70*1024*1024/4)
#set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l normal_result | awk '{print $1}'")
num2=system("wc -l qpsc_result | awk '{print $1}'")

set output "victim_util_cdf.eps"
set terminal postscript eps enhanced color "Helvetica" 48 size 14,9

set logscale x 

plot in1 using ($2):(1./num1) s cumul t 'USL' with line linetype 1 linewidth 10,\
in2 using ($2):(1./num2) s cumul t 'USL_{QPSC}' with line linetype 7 linewidth 10
