#!/bin/bash

for kernel in Triad #Copy Reduce Fill
do
	if [ "$kernel" == "Copy" ]; then
	col_id=2
	elif [ "$kernel" == "Triad" ]; then
	col_id=3
	fi

out_file=ia_mb_${kernel}_nt_rfo
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key out horizontal
set key bottom center
set output "./${out_file}.png"
set title "STREAM: ${kernel^^} scaling, Speed-up of NT over RFO stores"
set xlabel "Number of Threads"
set ylabel "Speed-up"
plot "./HSW_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "HSW",\
		 "./BDW_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "BDW",\
		 "./SKX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "SKX",\
		 "./CLX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "CLX",\
		 "./ICX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "ICX",\
		 "./SPR-B0_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "SPR-B0",\
     "./ia_${kernel}_ideal.stats" using 1 with lines lw 3 title "Ideal Speed-up"
EOF

out_file=others_mb_${kernel}_nt_rfo
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${kernel^^} scaling, Speed-up of NT over RFO stores"
set xlabel "Number of Threads"
set ylabel "Speed-up"
plot "./Rome_mb_scale_distribute_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "Rome",\
		 "./Milan_mb_scale_distribute_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "Milan",\
		 "./A64FX_mb_scale_distribute_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "A64FX",\
     "./others_${kernel}_ideal.stats" using 1 with lines lw 3 title "Ideal Speed-up"
EOF


done
