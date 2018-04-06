reset

in1 = "util_victim_1.txt"
#in2 = "util_victim_2.txt"
#in3 = "util_victim_3.txt"

set terminal postscript eps enhanced "Helvetica,42"  monochrome solid size 7.5, 4
set output "util_victim_early.eps"

#set key outside opaque left top horizontal enhanced samplen 2 spacing 1
set key inside right top enhanced samplen 2 font "Helvetica, 42"
#set key at 1, 1.3
set xtics border in scale 0,0 nomirror 

set yrange [0 : 0.5]
set xrange [-0.05 : 1.05]

set xlabel "Victim Segment Utilization (%)"
set ylabel "Fraction of Victims" offset character 1, 0, 0

set xtics autofreq 0.2
set xtics ("0"0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)
set ytics autofreq 0.1

set style fill pattern 4.00 border -1
bin(x, s) = s*int(x/s)

num1=system("wc -l util_victim_1.txt | awk '{print $1}'")
#num2=system("wc -l util_victim_2.txt | awk '{print $1}'")
#num3=system("wc -l util_victim_3.txt | awk '{print $1}'")

#set bars 1
set boxwidth 0.05 absolute

plot in1 using (bin($2, 0.05)):(1./num1) smooth frequency t 'Early' with linespoint ps 2 pt 5 lw 3
	#in2 using (bin($2, 0.05)):(1./num2) smooth frequency t 'Interim' with linespoint ps 3 pt 6 lw 3,\
	#in3 using (bin($2, 0.05)):(1./num3) smooth frequency t 'Late' with linespoint ps 2 pt 2 lw 3
