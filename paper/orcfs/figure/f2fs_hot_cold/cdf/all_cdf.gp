reset

in1 = "f2fs_ne"
in2 = "f2fs_mp4"
in3 = "m_f2fs_ne"
in4 = "m_f2fs_mp4"

set terminal postscript eps enhanced color "Helvetica,42"  size 9, 4
set output "all_cdf.eps"

#set key outside opaque left top horizontal enhanced samplen 2 spacing 1
set key right bottom vertical enhanced samplen 2
#set xtics border in scale 0,0 nomirror 

set yrange [0 : 1.1]
set xrange [-0.05 : 1.05]

set xlabel "Victim Section Utilization (%)"
set ylabel "Cumulative Probability"

set xtics autofreq 0.2
set xtics ("0"0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)
set ytics autofreq 0.2

#set style fill pattern 4.00 border -1

num1=system("wc -l f2fs_ne | awk '{print $1}'")
num2=system("wc -l f2fs_mp4 | awk '{print $1}'")
num3=system("wc -l m_f2fs_ne | awk '{print $1}'")
num4=system("wc -l m_f2fs_mp4 | awk '{print $1}'")

#set bars 1
#set boxwidth 0.05 absolute

plot in1 using 11:(1./num1) smooth cumulative t 'F2FS (N.E.)' lt 1 lw 5,\
	in2 using 11:(1./num2) smooth cumulative t 'F2FS (.mp4)' lt 2 lw 5,\
	in3 using 11:(1./num3) smooth cumulative t 'M F2FS (N.E.)' lt 3 lw 5,\
	in4 using 11:(1./num4) smooth cumulative t 'M F2FS (.mp4)' lt 4 lw 5
