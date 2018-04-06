reset

in1 = "./io_distribution.data"

set terminal postscript eps enhanced "Helvetica, 58" color solid size 8, 6
set output "io_distribution.eps"

set key right top font "Helvetica, 50" 
#set key opaque right top enhanced spacing 1
set key samplen 1.2
set rmargin 0.2

set xtics border in scale 0,0 nomirror

set yrange [0 : 190]
set xrange [-0.5 : 1.5]

#set xlabel "System"
set ylabel "Write Volume (Gbyte)" offset 1, 0, 0

set xtics ("F2FS" 0, "USL"1)
set ytics autofreq 50

set style data histogram
set style histogram rowstacked
set style fill pattern 0 border -1

set bars 1
set grid ytics
set boxwidth 0.5 absolute

plot in1 using 2 t "User data", \
                 "" u 3 t "FS GC" fill pattern 1 lc -1, \
                "" u 4 t "SSD GC" fill solid lc 1, \
		"" u 5 t "Meta Data" fill solid lc -1
