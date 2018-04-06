reset

in1 = "./usl.txt"
in2 = "./usl_qpsc.txt"

set terminal postscript eps enhanced "Helvetica,48"  monochrome size 9, 4
set termoption dash
set output "usl_qpsc_cdf.eps"

set key right bottom height 1.2

#set ylabel "Cumulative Probabillity" font "Helvetica, 48"
set ylabel "Cumulative Percent(%)"
set xlabel "Segment Cleaning Time (sec)" 

set xrange[0:]
set yrange[0:1.1]

set style line 1 lt 3 lc rgb "red" lw 10

set grid ytics
set xtic ("1"1000000, "2"2000000, "3"3000000, "4"4000000, "5"5000000, "6"6000000, "7"7000000)
set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l usl.txt | awk '{print $1}'")
num2=system("wc -l usl_qpsc.txt | awk '{print $1}'")


plot in1 using ($8):(1./num1) s cumul t 'OrcFS' with line linetype 1 linewidth 10,\
	in2 using ($8):(1./num2) s cumul t 'OrcFS_{QPSC}' ls 1
