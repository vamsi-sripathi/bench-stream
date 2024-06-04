gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key box outside bottom center horizontal
set output "./ia_numa.png"
set title "STREAM: 1-Socket - NUMA bandwidth with Triad"
set xlabel "NUMA domain ID"
set ylabel "GB/sec"
plot "./HSW_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "HSW",\
     "./BDW_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "BDW",\
     "./SKX_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "SKX",\
     "./CLX_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "CLX",\
     "./ICX_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "ICX",\
     "./SPR-B0_numa_nps1_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "SPR-B0"
EOF

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key box outside bottom center horizontal
set output "./rome_numa.png"
set title "STREAM: 1-Socket - NUMA bandwidth with Triad"
set xlabel "NUMA domain ID"
set ylabel "GB/sec"
plot "./Rome_numa_nps4_compact_nt.stats" using 3:xticlabels(1) with linespoints lw 3 title "Rome"
EOF

