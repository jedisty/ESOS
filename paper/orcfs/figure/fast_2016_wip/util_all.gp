reset

in1 = "util_victim_1.txt"
in2 = "util_victim_5.txt"
in3 = "util_victim_7.txt"
in4 = "util_victim_10.txt"
in5 = "util_victim_15.txt"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 7.5, 3.2
set output "util_victim_all.eps"

#set key outside opaque left top horizontal enhanced samplen 2 spacing 1
set key inside right top enhanced samplen 2
#set key at 1, 1.3
set xtics border in scale 0,0 nomirror 

set yrange [0 : 0.5]
set xrange [-0.05 : 1.05]

set xlabel "Victim Segment Utilization (%)"
set ylabel "Fraction of Segment"

set xtics autofreq 0.2
set xtics ("0"0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)
set ytics autofreq 0.2

set style fill pattern 4.00 border -1
bin(x, s) = s*int(x/s)

num1=system("wc -l util_victim_1.txt | awk '{print $1}'")
num2=system("wc -l util_victim_5.txt | awk '{print $1}'")
num3=system("wc -l util_victim_7.txt | awk '{print $1}'")
num4=system("wc -l util_victim_10.txt | awk '{print $1}'")
num5=system("wc -l util_victim_15.txt | awk '{print $1}'")

#set bars 1
set boxwidth 0.05 absolute

plot in1 using (bin($2, 0.05)):(1./num1) smooth frequency t 'Iteration 1' with linespoint ps 2 pt 5 lw 3,\
	in2 using (bin($2, 0.05)):(1./num2) smooth frequency t 'Iteration 2' with linespoint ps 3 pt 8 lw 3,\
	in5 using (bin($2, 0.05)):(1./num5) smooth frequency t 'Iteration 15' with linespoint ps 2 pt 2 lw 3
