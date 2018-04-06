reset

in1 = "./io_distribution.data"

set terminal postscript eps enhanced "Helvetica, 48"  monochrome solid size 9, 4
set output "io_distribution.eps"

set key outside reverse Left samplen 2 spacing 1.8
#set key outside top vertical reverse Left samplen 4 spacing 2 width 2
#set key opaque right top enhanced spacing 1
#set rmargin 0.2

set xtics border in scale 0,0 nomirror

set yrange [0 : 170]
set xrange [-0.5 : 1.5]

#set xlabel "System"
set ylabel "Write Volume (Gbyte)" offset 1, 0, 0

set xtics ("F2FS" 0, "OrcFS"1)
set ytics autofreq 40 nomirror 

set style data histogram
set style histogram rowstacked
set style fill pattern 0 border -1

set bars 1
set grid ytics
set boxwidth 0.5 absolute

plot in1 using 2 t "User data", \
                 "" u 3 t "FS SC" fill solid, \
                "" u 4 t "SSD GC" fill pattern 4, \
		"" u 5 t "Meta Data" fill pattern 1
