#!/bin/bash

out_file=mb_socket_bw
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key top left
set xtics rotate by -60
set output "./${out_file}.png"
set title "STREAM Triad (NT Stores): 1-Socket all-cores bandwidth"
set ylabel "GB/sec"
plot "mb_socket_nt.stats" using 3:xticlabels(1) with linespoints lw 3 notitle
EOF

out_file=mb_socket_bw_eff
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key top left
set xtics rotate by -45
set output "./${out_file}.png"
set title "STREAM Triad (NT Stores): 1-Socket B/W Efficiency"
set ylabel "Percentage of 1S b/w"
plot "mb_socket_triad_nt_eff.stats" using 4:xticlabels(1) with linespoints lw 3 notitle 
EOF

out_file=mb_socket_triad_nt_bpc
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key top left
set xtics rotate by -60
set output "./${out_file}.png"
set title "STREAM Triad (NT Stores): Memory Bandwidth Per Core"
set ylabel "GB/sec"
plot "mb_socket_triad_nt_bpc.stats" using 3:xticlabels(1) with linespoints lw 3 title "Loaded 1-Core Bandwidth",\
     "mb_socket_triad_nt_10c.stats" using 4:xticlabels(1) with linespoints lw 3 title "Loaded 10-Core Bandwidth",\
     "mb_core_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "Idle 1-Core Bandwidth"
EOF

out_file=mb_socket_triad_nt_rfo
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key top left
set xtics rotate by -60
set output "./${out_file}.png"
set title "STREAM: 1-Socket all-cores bandwidth"
set ylabel "GB/sec"
plot "mb_socket_nt_rfo.stats" using 3:xticlabels(1) with linespoints lw 3 title "Triad-NT",\
     "mb_socket_nt_rfo.stats" using 6:xticlabels(1) with linespoints lw 3 title "Triad-RFO"
EOF
