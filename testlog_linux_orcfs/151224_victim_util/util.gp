reset

in1 = "./f2fs_victim.txt"
#in2 = "./inp.txt"

set terminal wxt 0
set key left top

set ylabel "Cumulative Percent (%)" font "Helvetica, 48"
set xlabel "# of valid pages" font "Helvetica, 48"

set xrange[0:70000]
set yrange[0:1.1]

set xtic (0, 20000, 40000, 60000)
set ytic (0, "20"0.2, "40"0.4, "60"0.6, "80"0.8, "100"1)

num1=system("wc -l f2fs_victim.txt | awk '{print $1}'")
#num2=system("wc -l inp.txt | awk '{print $1}'")

set output "victim_util_cdf_victim.eps"
set terminal postscript eps enhanced color "Helvetica" 48 size 7,5
# set key samplen 3

plot in1 using ($1):(1./num1) s cumul t 'f2fs w/o SSR' with line linetype 1 linewidth 10
#in2 using ($1):(1./num2) s cumul t 'inp' with line linetype 7 linewidth 10
