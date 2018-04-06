reset

set key font "Helvetica, 38" 
set key left top horizontal

set xrange [0:7]
set yrange [0:70]
set y2range [0:7]

set ytic(0, 10, 20, 30, 40, 50, 60 ) font "Helvetica, 38" nomirror
set y2tic(0, 1, 2, 3, 4, 5, 6 ) font "Helvetica, 38" nomirror
set xtic("2_{F2FS}"1, "8_{F2FS}"2, "32_{F2FS}"3, "128_{F2FS}"4, "256_{F2FS}"5, "256_{USL}"6)  font "Helvetica, 28" rotate by -30

set ylabel "IOPS (K)" font "Helvetica, 38"
set y2label "WAF" font "Helvetica, 38"
set xlabel "Section size (Mbyte)" font "Helvetica, 38"

set terminal postscript enhanced color font "Helvetica, 38" size 14, 7
set output "segs_per_sec.eps"

set grid ytics
set style data histograms
#set style fill solid 1.00 border -1
set style fill pattern 1 border -1
set boxwidth 0.6

plot "segs_per_sec.txt"  u ($1)/1024 title "IOPS" with boxes lt -1, \
		'' u ($2) title "WAF" with linespoint lw 2 pt 9 ps 3 lc 1 axis x1y2 
