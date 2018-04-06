reset

in1 = "dram_size_2.data"

set terminal postscript eps enhanced monochrome "Helvetica" 48 size 9, 4.5

set key inside top horizontal center samplen 2 height -4.5
set grid y

set xlabel "Year" font "Helvetica, 48"
set ylabel "Percentage (%)" font "Helvetica, 48"

set xrange[-0.5:2.5]
set yrange[0:]

#set xtic("2011"0, "2012"1, "2013~"2)
set ytic autofreq 20
set x2tics nomirror font "Helvetica, 48" offset 0, -0.5, 0

set style data histogram
set style histogram rowstacked
set style fill pattern 1.00 border -1

set bars 1
set grid ytics
set boxwidth 0.7 absolute

set output "dram_size.eps"
plot in1 using (100.*$2/($2+$3+$4)):xtic(5):x2tic(sprintf("%d",($2+$3+$4))) t column(2), \
	"" u (100.*$3/($2+$3+$4)) t column(3) fill solid ,\
	"" u (100.*$4/($2+$3+$4)) t column(4) fill pattern 4
