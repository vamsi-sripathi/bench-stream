#!/bin/bash
 
for d in 1 2
do
  for k in triad scale ro wo
  do
     dir=${k}_logs
     mkdir -p ${dir}
     cd ${dir}
     rm -f ${k}_d${d}.dat
     for sz in {1024..4096..512}
     do
        echo "Running $k kernel with ${sz}m buffer on $d device(s).."
        ../stream-${k} -d${d} -b${sz}m -v &> ${d}d_${sz}m.log
        grep "Average Bandwidth (GB/sec) as measured by host" ${d}d_${sz}m.log | awk -v b=$sz '{print b"\t"$NF}' >> ${k}_d${d}.dat
     done
     cd - &> /dev/null
  done
done
 
for k in triad scale ro wo
do
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "${k}.png"
set xtics 1024,512,4096
set title "STREAM: ${k} bandwidth"
set xlabel "Buffer size (Scale: x2, Triad: x3)"
set ylabel "GB/sec"
plot "./${k}_logs/${k}_d1.dat" using 1:2 with linespoints lw 3 title "1 GPU", \
     "./${k}_logs/${k}_d2.dat" using 1:2 with linespoints lw 3 title "2 GPU"
EOF
done
