set terminal postscript eps enhanced monochrome "Helvetica" 48 size 9, 4

#set title "Non Preemptive SC Latency"

in = "normal_result"

set key inside top left font ",40"
set key samplen 2

set grid ytics lt 3

set xtics ("0" 0,"10" 10*1024*1024/4,"20" 20*1024*1024/4,"30" 30*1024*1024/4,"40" 40*1024*1024/4,"50" 50*1024*1024/4,"60" 60*1024*1024/4,"70" 70*1024*1024/4,"80" 80*1024*1024/4,"90" 90*1024*1024/4, "100" 100*1024*1024/4)
#set ytics autofreq 400

set xlabel "Random Write Volume (Gbyte)"
set ylabel "Latency (sec)" offset 1, 0, 0

set output "normal_latency.eps"

plot in using ($2/1000000) title "OrcFS" with lines lw 1 lt 1
