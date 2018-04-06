reset
#set terminal postscript eps enhanced monochrome "Helvetica" 48 size 15, 6
set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 9, 4

in1 = "block_copy_info_init"

#set key bmargin center horizontal Left width -5 font ",30"
set key inside top right
#set grid ytics lt 3

#set xtics ("0" 0,"10" 10*1024*1024/4,"20" 20*1024*1024/4,"30" 30*1024*1024/4,"40" 40*1024*1024/4,"50" 50*1024*1024/4,"60" 60*1024*1024/4,"70" 70*1024*1024/4,"80" 80*1024*1024/4,"90" 90*1024*1024/4, "100" 100*1024*1024/4)
#set ytics autofreq 400

set xlabel "Sequence of Segment Cleanings"
set ylabel "Latency (sec)" offset 1, 0, 0

set xrange [0:635]

set output "normal_total_latency.eps"

plot in1 using ($8/1000/1000) title "OrcFS" with lines lw 1 lt 1
