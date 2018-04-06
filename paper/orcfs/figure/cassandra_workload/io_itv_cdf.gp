reset

in1 = "time_itv_t.txt"

set terminal postscript eps enhanced color "Helvetica,42"  size 9, 4
set output "time_itv_cdf_t.eps"

set yrange [0 : 1.1]
set xlabel "IO Issue Time Interval"
set ylabel "Cumulative Probability"

set xtics ("1 nsec"1, "100 nsec"100, "10 usec"10000, "1 msec"1000000, "100 msec"100000000, "10 sec"10000000000)
set ytics autofreq 0.2
unset key 

set logscale x

num1=system("wc -l time_itv_t.txt | awk '{print $1}'")

plot in1 using 1:(1./num1) smooth cumulative t 'IO request' lt 1 lw 5


