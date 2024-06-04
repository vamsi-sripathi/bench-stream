#!/bin/bash

for kernel in Triad #Copy Reduce Fill
do
	if [ "$kernel" == "Copy" ]; then
	col_id=2
	elif [ "$kernel" == "Triad" ]; then
	col_id=3
	fi

out_file=ia_mb_${kernel}_nt_rfo_1
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key inside horizontal
set key top right
set output "./${out_file}.png"
set title "STREAM: ${kernel^^} scaling, Speed-up of NT over RFO stores"
set xlabel "Number of Threads"
set ylabel "Speed-up"
plot "./HSW_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "HSW",\
		 "./BDW_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "BDW",\
		 "./SKX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "SKX",\
		 "./CLX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "CLX",\
		 "./ICX_mb_scale_compact_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "ICX",\
     "./ia_${kernel}_ideal.stats" using 1 with lines lw 3 title "Ideal Speed-up"
EOF

out_file=ia_mb_${kernel}_nt_rfo_2
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key inside horizontal
set key top right
set output "./${out_file}.png"
set title "STREAM: ${kernel^^} scaling, Speed-up of NT over RFO stores"
set xlabel "Number of Threads"
set ylabel "Speed-up"
plot "./SPR_mb_scale_distribute_nt_rfo.stats"       using 1:${col_id} with linespoints lw 3 title "SPR-DDR",\
		 "./SPR-HBM_mb_scale_distribute_nt_rfo.stats"   using 1:${col_id} with linespoints lw 3 title "SPR-HBM",\
		 "./EMR_mb_scale_distribute_nt_rfo.stats"       using 1:${col_id} with linespoints lw 3 title "EMR",\
		 "./GNR-DDR_mb_scale_distribute_nt_rfo.stats"   using 1:${col_id} with linespoints lw 3 title "GNR-DDR",\
		 "./SRF_mb_scale_compact_nt_rfo.stats"          using 1:${col_id} with linespoints lw 3 title "SRF",\
     "./ia_${kernel}_ideal_144c.stats" using 1 with lines lw 3 title "Ideal Speed-up"
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
		 "./Genoa_mb_scale_distribute_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "Genoa",\
     "./others_${kernel}_ideal.stats" using 1 with lines lw 3 title "Ideal Speed-up"
EOF


done
