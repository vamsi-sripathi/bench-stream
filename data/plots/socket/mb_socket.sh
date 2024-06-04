#!/bin/bash

out_file=mb_socket_bw
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid lt 3 lw 1
set key center left
set xtics rotate by -60
set rmargin 5
set ytics 50
set pointsize 2
set output "./${out_file}.png"
set title "STREAM: 1-Socket all-cores bandwidth"
set ylabel "GB/sec"
plot "mb_socket_nt.stats" using 3:xticlabels(1) with linespoints lw 3 pt 7 title "Triad (NT)", \
     "mb_socket_nt.stats" using 4:xticlabels(1) with linespoints lw 3 pt 6 title "All-Reads"
EOF

out_file=mb_socket_bw_eff
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid lt 3 lw 1
set key center left
set xtics rotate by -60
set rmargin 5
set pointsize 2
set output "./${out_file}.png"
set title "STREAM: 1-Socket B/W Efficiency"
set ylabel "Percentage of 1S B/W"
plot "mb_socket_triad_nt_eff.stats" using 4:xticlabels(1) with linespoints lw 3 pt 7 title "Triad (NT)", \
     "mb_socket_allreads_eff.stats" using 4:xticlabels(1) with linespoints lw 3 pt 6 title "All-Reads"
EOF


out_file=mb_socket_triad_nt_bpc
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid lt 3 lw 1
set xtics rotate by -60
set rmargin 5
set pointsize 2
set output "./${out_file}.png"
set title "STREAM Triad (NT Stores): 1-core Memory Bandwidth"
set ylabel "GB/sec"
plot "mb_core_nt.stats" using 3:xticlabels(1) with linespoints lw 3 pt 7 notitle
EOF

out_file=mb_socket_triad_nt_rfo
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid lt 3 lw 1
set rmargin 5
set pointsize 2
set key top left
set xtics rotate by -60
set output "./${out_file}.png"
set title "STREAM: 1-Socket Triad all-cores bandwidth"
set ylabel "GB/sec"
plot "mb_socket_nt_rfo.stats" using 3:xticlabels(1) with linespoints lw 3 title "NT",\
     "mb_socket_nt_rfo.stats" using 6:xticlabels(1) with linespoints lw 3 title "RFO"
EOF
