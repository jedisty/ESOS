reset
#set terminal postscript eps enhanced monochrome "Helvetica" 48 size 15, 6
set terminal postscript eps enhanced "Helvetica,48"  monochrome solid size 10, 3

in1 = "result"

set key top right
#set key tmargin center height 1
#set key inside top right
#set grid ytics lt 3

set yrange [0:9]

#set xtics ("15" 0,"25" 10*1024*1024/4,"35" 20*1024*1024/4,"45" 30*1024*1024/4,"55" 40*1024*1024/4,"65" 50*1024*1024/4,"75" 60*1024*1024/4)
#set xtics ("20" 1024*1024/4,"30" 11*1024*1024/4,"40" 21*1024*1024/4,"50" 31*1024*1024/4,"60" 41*1024*1024/4,"70" 51*1024*1024/4,"60" 61*1024*1024/4)
set xtics ("0"0,"10"10*1024*1024/4,"20"20*1024*1024/4,"30"30*1024*1024/4,"40"40*1024*1024/4, "50"50*1024*1024/4,"60"60*1024*1024/4,"70"70*1024*1024/4)
set ytics autofreq 3

set xlabel "Write Volume (GB)"
set ylabel "Latency (sec)" offset 1, 0, 0

set output "qpsc_latency.eps"

plot in1 using ($2/1000/1000) title "OrcFS" with lines lw 3 lt 1
