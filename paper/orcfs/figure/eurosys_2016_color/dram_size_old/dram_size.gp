reset

in1 = "dram_size.data"

set terminal postscript eps enhanced monochrome "Helvetica" 40 size 7.5, 4

set key right bottom
#set key right bottom font "Helveticai, 38"
set grid y

set xlabel "SSD Capacity (Gbyte)" font "Helvetica, 44"
set ylabel "DRAM Size (Mbyte)" font "Helvetica, 44"

set xrange[0:1024]
set yrange[0:1280]

set xtic(0, 128, 256, 384, 512, 640, 768, 896, 1024, 1152, 1280)
set ytic(0, 256, 512, 768, 1024)

set output "dram_size.eps"
plot in1 u 1:2 t '2011' with point pt 2 ps 3, \
	in1 u 3:4 t '2012' with point pt 6 ps 3, \
	in1 u 5:6 t '2014\~2015' with point pt 9 ps 3
