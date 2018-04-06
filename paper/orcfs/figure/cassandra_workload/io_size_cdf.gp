reset

in1 = "result_w.p"
in2 = "result_r.p"

set terminal postscript eps enhanced color "Helvetica,42"  size 9, 4
set output "all_cdf.eps"

#set key outside opaque left top horizontal enhanced samplen 2 spacing 1
set key right bottom vertical enhanced samplen 2
#set xtics border in scale 0,0 nomirror 

set yrange [0 : 1.1]
#set xrange [-0.05 : 1.05]

set xlabel "IO Size (KByte)"
set ylabel "Cumulative Probability"

#set xtics autofreq 0.2
set xtics ("0"0, "64"128, "128"256, "192"384, "256"512, "320"640, "384"768, "448"896,"512"1024)
set ytics autofreq 0.2

#set logscale x

num1=system("wc -l result_w.p | awk '{print $1}'")
num2=system("wc -l result_r.p | awk '{print $1}'")

#set bars 1
#set boxwidth 0.05 absolute

plot in1 using 5:(1./num1) smooth cumulative t 'write request' lt 1 lw 5,\
	in2 using 5:(1./num2) smooth cumulative t 'read request' lt 2 lw 5,\
