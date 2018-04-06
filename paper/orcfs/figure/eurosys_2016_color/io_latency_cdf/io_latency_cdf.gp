reset

in1 = "./io_latency_f2fs.txt"
in2 = "./io_latency_da.txt"

set terminal wxt 0
set key right bottom

set ylabel "Cumulative Percent (%)" font "Helvetica, 48"
set xlabel "Write Latency (usec)" font "Helvetica, 48"

#set xrange[0:]
set yrange[0:1.1]

set xtic (1, 10, "10^2"100, "10^3"1000, "10^4"10000, "10^5"100000, "10^6"1000000)
set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l io_latency_f2fs.txt | awk '{print $1}'")
num2=system("wc -l io_latency_da.txt | awk '{print $1}'")

set output "io_latency_cdf.eps"
set terminal postscript eps enhanced color "Helvetica" 48 size 7,5
set logscale x

# set key samplen 3

plot in1 using ($2):(1./num1) s cumul t 'F2FS' with line linetype 1 linewidth 10,\
in2 using ($2):(1./num2) s cumul t 'USL' with line linetype 7 linewidth 10

