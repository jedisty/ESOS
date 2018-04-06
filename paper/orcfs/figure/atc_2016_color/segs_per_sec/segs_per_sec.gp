reset

set terminal postscript enhanced color font "Helvetica, 25" size 9, 4
set output "segs_per_sec.eps"

set key font "Helvetica, 20"
set key right top horizontal samplen 2
set rmargin 6.8

set xrange [0:10]
set yrange [0:100]
set y2range [0:2]

set ytic(0, 20, 40, 60, 80) nomirror font "Helvetica, 20"
set y2tic autofreq 0.4  nomirror font "Helvetica, 20" 
set xtic("2"1, "8"2, "32"3, "128\nF2FS"4, "256"5, "512"6, "1024"7, "256\nUSL"9)  font "Helvetica, 20" 

set ylabel "IOPS (K)" offset 1, 0, 0 #font "Helvetica, 28"
set y2label "WAF" offset -1.8, 0, 0 #font "Helvetica, 28"
set xlabel "Section size (Mbyte)" offset 0, -0.3, 0 #font "Helvetica, 28"


set grid ytics
set style data histograms
#set style fill solid 1.00 border -1
set style fill pattern 1 border -1
set boxwidth 0.6

plot "segs_per_sec.txt"  u ($1)/1024 title "IOPS" with boxes lt -1, \
		'' u ($2) title "WAF" with points lw 2 pt 9 ps 3 lc 1 axis x1y2
