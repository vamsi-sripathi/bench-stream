#!/bin/bash

rm -f *.dat

for t in {1..64};
do
  grep "Copy:" stream_avx2_${t}t.log  | awk -v thr=$t '{print thr"\t"$2/1000}' >> copy.dat
  grep "Scale:" stream_avx2_${t}t.log  | awk -v thr=$t '{print thr"\t"$2/1000}' >> scale.dat
  grep "Add:" stream_avx2_${t}t.log | awk -v thr=$t '{print thr"\t"$2/1000}' >> add.dat
  grep "Triad:" stream_avx2_${t}t.log | awk -v thr=$t '{print thr"\t"$2/1000}' >> triad.dat
  echo -e "$t\t204.8" >> 1s-peak.dat
done

out_file=thp_off_offset_zero_copy_scale
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set xtics 4
set ytics 10
set xrange [1:]
set yrange [0:]
set output "./${out_file}.png"
set title "STREAM: 1R+1W traffic. THP OFF, NT stores"
set xlabel "Number of threads"
set ylabel "GB/sec"
plot "./copy.dat" using 1:2 with linespoints lw 3 title "COPY",\
     "./scale.dat" using 1:2 with linespoints lw 3 title "SCALE",\
     "./1s-peak.dat" using 1:2 with linespoints lw 3 title "1S Theor. Peak"

EOF

out_file=thp_off_offset_zero_add_triad
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set xtics 4
set ytics 10
set xrange [1:]
set yrange [0:]
set output "./${out_file}.png"
set title "STREAM: 2R+1W traffic. THP OFF, NT stores"
set xlabel "Number of threads"
set ylabel "GB/sec"
plot "./triad.dat" using 1:2 with linespoints lw 3 title "TRIAD",\
     "./add.dat" using 1:2 with linespoints lw 3 title "ADD",\
     "./1s-peak.dat" using 1:2 with linespoints lw 3 title "1S Theor. Peak"

EOF

