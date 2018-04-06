reset

in1 = "./io_latency_candle.data"

set terminal postscript eps enhanced monochrome "Helvetica, 58" size 6, 5
set key off

set ylabel "Write Latency \n (msec, Log scale)" offset 2, 0, 0

set xrange[0.00000 : 3.0000] noreverse nowriteback
set yrange[0.5 : 5000000.0000 ] noreverse nowriteback

set xtic ("F2FS"1, "USL"2) font "Helvetica, 58"
set ytic ("0.001"1, "0.01"10, "0.1"100, "1"1000, "10"10000, "100"100000, "1000"1000000, "3000"3000000) font "Helvetica, 54"
#set ytic ("0"0.1, "1 us"1, "10 us"10, "100 us"100, "1 ms"1000, "10 ms"10000, "100 ms"100000, "1 s"1000000, "3 s"3000000) font "Helvetica, 50"

set output "io_latency_candle.eps"

set boxwidth 0.4 absolute
set logscale y

x=0.0

plot in1 using 1:2:3:4:5 with candlesticks lt 1 lw 2  whiskerbars,\
	"" using 1:6:6:6:6 with candlesticks lt 4 lw 5 notitle
