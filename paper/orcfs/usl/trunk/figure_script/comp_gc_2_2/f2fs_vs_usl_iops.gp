reset

in1 = "./f2fs_vs_usl_iops.data"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 3.5,4
set output "f2fs_vs_usl_iops.eps"

set key opaque left top horizontal enhanced samplen 2 spacing 1
set key off

set xtics border in scale 0,0 nomirror offset  character 0.7, 0.2, 0

set yrange [0 : 80]
set xrange [0.2 : 2.8]

#set xlabel "File size" offset character 2, 0, 0
set ylabel "IOPS (x10^3)" offset character 1, 0, 0

set xtics ("F2FS"0.7, "OrcFS"2) font "Helvetica,40"
set ytics autofreq 15

#set style data histogram
#set style histogram cluster gap 1
set style fill pattern 1.00 
set style line 2 lt 1 lw 2

set bars 1
set grid ytics lt 3
set boxwidth 0.8 absolute

plot in1 using ($1):($2)/1024 t "F2FS" with boxes fill pattern 0, \
               "" u ($3):($4)/1024 t "OrcFS" with boxes fill pattern 1 ls 2

