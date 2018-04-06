reset

in1 = "dram_size.data"
in2 = "trend.txt"

set terminal postscript eps enhanced color "Helvetica" 44 size 9, 4.5

set key outside top horizontal height 1 font "Helvetica, 36"
set key samplen 0.4
set grid y

set xlabel "Year" font "Helvetica, 44"
set ylabel "DRAM Size (Mbyte)" font "Helvetica, 44" offset 1, 0, 0

set xrange[0:6]
set yrange[0:1152]

set xtic("2011"1, "2012"2, "2013"3, "2014"4, "2015"5)
set ytic autofreq 256

set xtics nooffset
set xtics nomirror

set grid ytics

set output "dram_size.eps"
plot in1 using 1:2 title "SSD < 128GB" ps 3 pt 10 lc -1, \
	"" u 3:4 title "128GB {/Symbol \243} SSD < 256GB" ps 3 pt 12 lc -1, \
	"" u 5:6 title "256GB {/Symbol \243} SSD < 512GB" ps 3 pt 4 lc -1, \
	"" u 7:8 title "512GB {/Symbol \243} SSD < 1024GB" ps 3 pt 9 lc 3, \
	"" u 9:10 title "1024GB {/Symbol \243} SSD" ps 3 pt 13 lc 1, \
	in2 using 1:2 title "Trend" with line lt 2 lw 3 lc 4
