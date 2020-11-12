#!/bin/bash

pushd mem_bw_scale
tag=scale
archs=("SNC-1" "SNC-2")

  for kernel in Copy Triad Reduce Fill
  do
  if [ "$kernel" == "Copy" ]; then
    col_id=2
  elif [ "$kernel" == "Triad" ]; then
    col_id=3
  elif [ "$kernel" == "Reduce" ]; then
    col_id=4
  elif [ "$kernel" == "Fill" ]; then
    col_id=5
  fi

  for stype in rfo #nt
  do
    out_file=mb_${tag}_special_${kernel}_${stype}

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${kernel^^} ${stype^^} stores scaling"
set xlabel "Number of Threads"
set ylabel "GB/sec"
plot "./${archs[0]}_mb_${tag}_compact_${stype}.stats" using 1:${col_id} with linespoints lw 3 title "${archs[0]} - Compact",\
     "./${archs[1]}_mb_${tag}_distribute_${stype}.stats" using 1:${col_id} with linespoints lw 3 title "${archs[1]} - Distributed"
EOF
  done
  done

popd
