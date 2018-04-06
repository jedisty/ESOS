reset

in1 = "./f2fs_vs_usl_iops.data"

set terminal postscript eps enhanced "Helvetica,42"  color solid size 3,3.5
set output "f2fs_vs_usl_iops.eps"

set key opaque left top horizontal enhanced samplen 2 spacing 1
set key off

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0

set yrange [0 : 70]
set xrange [0.2 : 2.8]

#set xlabel "File size" offset character 2, 0, 0
set ylabel "IOPS (K)" offset character 1, 0, 0

set xtics ("F2FS"0.7, "USL"2.1)
set ytics autofreq 15

#set style data histogram
#set style histogram cluster gap 1
set style fill pattern 1.00 border -1

set bars 1
set grid ytics
set boxwidth 0.8 absolute

plot in1 using ($1):($2)/1024 t "F2FS" with boxes fill pattern 0, \
               "" u ($3):($4)/1024 t "USL" with boxes fill pattern 1 lc 1

