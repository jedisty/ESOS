reset

in1 = "./io_latency_buffered.txt"
in2 = "./io_latency_direct.txt"
in3 = "./io_latency_fsync.txt"
in4 = "./io_latency_fdatasync.txt"

#set terminal wxt 0
set terminal postscript eps enhanced color "Helvetica" 48 size 12,5
set key outside reverse Left samplen 2 spacing 1.8

set ylabel "Fraction of I/Os (%)" font "Helvetica, 48"
#set ylabel "Victim Block Utilization(%)" font "Helvetica, 48"
set xlabel "I/O Latency (usec)" font "Helvetica, 48"

#set xrange[0:]
set yrange[0:1.1]

#set xtic (0, "0.4"0.0004, "0.8"0.0008, "1.2"0.0012, "1.6"0.0016, "2"0.002)
set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)
set logscale x

num1=system("wc -l io_latency_buffered.txt | awk '{print $1}'")
num2=system("wc -l io_latency_direct.txt | awk '{print $1}'")
num3=system("wc -l io_latency_fsync.txt | awk '{print $1}'")
num4=system("wc -l io_latency_fdatasync.txt | awk '{print $1}'")

set output "cdf.eps"
# set key samplen 3

plot in1 using ($2):(1./num1) s cumul t 'Buffered' with line linetype 1 linewidth 10,\
in2 using ($2):(1./num2) s cumul t 'O\_DIRECT' with line linetype 3 linewidth 10,\
in3 using ($2):(1./num3) s cumul t 'write+fsync()' with line linetype 0 linewidth 10,\
in4 using ($2):(1./num4) s cumul t 'write+fdatasync()' with line linetype 4 linewidth 10
