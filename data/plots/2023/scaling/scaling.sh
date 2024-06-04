#!/bin/bash

stype=nt
out_file=mb_scale_triad_${stype}

gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid
set key bottom right
set output "./x86_${out_file}.png"
set title "STREAM: Triad NT stores scaling"
set xlabel "Number of Threads"
set ylabel "GB/sec"
set pointsize 0.75
plot "./HSW_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "HSW",\
     "./BDW_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "BDW",\
     "./SKX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "SKX",\
     "./CLX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "CLX",\
     "./ICX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "ICX",\
     "./SPR-B0_mb_scale_compact_nt.stats"    using 1:3 with linespoints lw 3 title "SPR-B0",\
     "./Rome_mb_scale_distribute_nt.stats"   using 1:3 with linespoints lw 3 title "Rome",\
     "./Milan_mb_scale_distribute_nt.stats"  using 1:3 with linespoints lw 3 title "Milan"
EOF

gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid
set key bottom right
set output "./others_${out_file}.png"
set title "STREAM: Triad NT stores scaling"
set xlabel "Number of Threads"
set ylabel "GB/sec"
set pointsize 0.75
plot "./Power9_mb_scale_compact_nt.stats"    using 1:3 with linespoints lw 3 title "Power9",\
     "./ThunderX2_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "ThunderX2",\
     "./Graviton2_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "Graviton2"
EOF
#     "./A64FX_mb_scale_distribute_nt.stats" using 1:3 with linespoints lw 3 title "A64FX"

gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid
set key bottom right
set output "./a64fx_${out_file}.png"
set title "STREAM: Triad NT stores scaling"
set xlabel "Number of Threads"
set ylabel "GB/sec"
set pointsize 0.75
plot      "./A64FX_mb_scale_distribute_nt.stats" using 1:3 with linespoints lw 3 title "A64FX"
EOF

stype=nt
out_file=mb_scale_triad_${stype}_skx-clx-icx

gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid lw 2
set key bottom right
set output "./x86_${out_file}.png"
set title "STREAM Triad (NT stores)\n1-Socket Scaling"
set xlabel "Number of Threads"
set ylabel "GB/sec"
set pointsize 0.75
plot "./SKX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "SKX",\
     "./CLX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "CLX",\
     "./ICX_mb_scale_compact_nt.stats" using 1:3 with linespoints lw 3 title "ICX"
EOF


