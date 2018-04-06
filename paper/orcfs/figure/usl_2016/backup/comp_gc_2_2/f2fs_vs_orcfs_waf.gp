reset

in1 = "./f2fs_vs_orcfs_waf.data"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 5.5,3.5
set output "f2fs_vs_orcfs_waf.eps"

set key opaque left top horizontal enhanced samplen 2 spacing 1

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0

set yrange [0 : 2.2]
set xrange [-0.5 : 2.5]

#set xlabel "File size" offset character 2, 0, 0
set ylabel "WAF" offset character 1, 0, 0

set xtics ("Filesystem" 0, "SSD"1, "Total"2)
set ytics autofreq 0.5

set style data histogram
set style histogram cluster gap 1
set style fill pattern 1.00 border -1

set bars 1
set grid ytics
set boxwidth 0.9 absolute

plot in1 using 2 t "F2FS" fill pattern 0, \
               "" u 3 t "OrcFS" fill pattern 1

