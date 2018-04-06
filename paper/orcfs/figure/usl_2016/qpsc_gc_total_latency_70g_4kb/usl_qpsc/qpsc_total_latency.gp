set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 5, 4

in1 = "block_copy_info_2"

#set key bmargin center horizontal Left width -5 font ",30"
set key inside top right
#set grid ytics lt 3

#set xtics ("0" 0,"10" 10*1024*1024/4,"20" 20*1024*1024/4,"30" 30*1024*1024/4,"40" 40*1024*1024/4,"50" 50*1024*1024/4,"60" 60*1024*1024/4,"70" 70*1024*1024/4,"80" 80*1024*1024/4,"90" 90*1024*1024/4, "100" 100*1024*1024/4)
set xtics autofreq 100
#unset ytics

set xlabel "Sequence of SC"
#set xlabel "Random Write Volume (Gbyte)"
set ylabel "SC Time (sec)" offset 1, 0, 0

set yrange [0:7]
set xrange [0:300]

set output "qpsc_total_latency.eps"

plot in1 using ($8/1000/1000) title "OrcFS_{QPSC}" with lines lw 3 lt 1
