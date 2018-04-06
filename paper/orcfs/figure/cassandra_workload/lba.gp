reset

in1 = "result_w.p"

set terminal postscript eps enhanced color "Helvetica,42" size 9, 4
set output "lba_w.eps"
#set termoption enhanced

set ylabel "LBA (10^6)"
set xlabel "Sequence of Write Request"

set xtics autofreq 4000
set ytics autofreq 100
#set xtics (0, "4000"4000000, "8000"8000000, "12000"12000000, "16000"16000000)

unset key

set style data points

plot in1 using (($4)/1000000)
