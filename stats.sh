#!/bin/bash

set -u

function check_logs()
{
  for arch in ${archs[*]};
  do
     for nps in 1 2 4;
     do
       for m in uma numa;
       do
         for aff_type in compact distribute
         do
           if [ ! -d  ${arch}/nps-${nps}/${m}/${aff_type} ]; then
             continue
           fi
           src_dir=${arch}/nps-${nps}/${m}/${aff_type}
           pushd ${src_dir} > /dev/null

           for stype in nt rfo
           do
              echo -n "Checking stream logs for ${src_dir}/${stype}...."
              pushd ${stype} > /dev/null

              for f in stream*.log
              do
                grep -i "fail" $f &> /dev/null
                if [ $? -eq 0 ]; then
                   echo -e "\nValidation failed for $arch/$stype/$f"
                   exit 1
                fi
                grep -i "OMP.*Warning" $f &> /dev/null
                if [ $? -eq 0 ]; then
                   echo -e "\nOMP affinity warnings encountered for $arch/$stype/$f"
                   exit 1
                fi

                # desired_t=$(basename $f .log | awk -F "_" '{print $NF}' | tr -d "t")
                desired_t=$(echo $f | grep -E "_[0-9]+t[_|\.]" -o | grep [[:digit:]] -o | tr -d "\n")
                found_t=$(grep "Number of Threads counted" $f | awk -F "=" '{print $NF}' | tr -d " ")
                if [ "${desired_t}" != "${found_t}" ]; then
                  echo -e "\nThreads mismatch for $arch/$stype/$f"
                  exit 1
                fi
              done
              echo "passed"

              popd > /dev/null
           done #stype

           popd > /dev/null
         done #aff_type
       done #m
     done #nps
  done #arch
}

function get_mem_bw_numa()
{
  tag=numa
  dst_dir=${results_root}/mem_bw_${tag}
  mkdir -p ${dst_dir}

  echo "Getting memory b/w stats for \"$tag\".."
  for arch in ${archs[*]};
  do
    echo -e "\t${arch}.."
    for nps in 1 2 4
    do
      for aff_type in compact
      do
        src_dir=${arch}/nps-${nps}/${tag}/${aff_type}
        if [ ! -d ${src_dir} ]; then
          continue
        fi
        pushd ${src_dir} > /dev/null

        for stype in nt rfo
        do
          pushd ${stype} > /dev/null

          num_cores_per_socket=$(awk -F "=" '/num_cores_per_socket/ {print $NF}' ./runinfo.log | tr -d [:blank:])
          num_numa_domains=$(awk -F "=" '/\ynum_numa_domains\y/ {print $NF}' ./runinfo.log | tr -d [:blank:])
          for t in 1 ${num_cores_per_socket};
          do
            outfile=${arch}_${tag}_nps${nps}_${aff_type}_${stype}_${t}t.stats
            echo -e "#${t}-threads\t\t\t\t" > ${outfile}
            echo -e "#NUMA-domain\tCopy\tTriad\tReduce\tFill" >> ${outfile}

            for m in $(seq 0 $((${num_numa_domains}-1)))
            do
              f=./stream_*_${t}t_m${m}.log
              awk -v numa=$m '/Copy:/   {printf("%d\t%.2f",numa,$2/1000)}' $f >> ${outfile} 
              awk            '/Triad:/  {printf("\t%.2f",$2/1000)}' $f >> ${outfile} 
              awk            '/Reduce:/ {printf("\t%.2f",$2/1000)}' $f >> ${outfile} 
              awk            '/Fill:/   {printf("\t%.2f\n",$2/1000)}' $f >> ${outfile} 
            done
            cp ${outfile} ${dst_dir}/

          done
          paste ${arch}_${tag}_nps${nps}_${aff_type}_${stype}_1t.stats ${arch}_${tag}_nps${nps}_${aff_type}_${stype}_${num_cores_per_socket}t.stats > ${arch}_${tag}_nps${nps}_${aff_type}_${stype}.stats
          cp ${arch}_${tag}_nps${nps}_${aff_type}_${stype}.stats ${dst_dir}/
          popd > /dev/null
        done
        popd > /dev/null
      done
    done
  done
}

function get_mem_bw()
{
  if [[ "$1" != "core" && "$1" != "socket" && "$1" != "node" ]]; then
    echo "unknown option -- $1 .. quitting"
    exit 1
  fi

  tag=$1
  dst_dir=${results_root}/mem_bw_${tag}
  mkdir -p ${dst_dir}

  echo "Getting memory b/w stats for \"$tag\".."
  for arch in ${archs[*]};
  do
    pushd ${arch}/${l1_dir}/${l2_dir}/${l3_dir} > /dev/null
    echo -e "\t${arch}.."

    if [ "$1" == "core" ]; then
      tid=1
    elif [ "$1" == "socket" ]; then
      tid_nt=$(awk -F "=" '/num_cores_per_socket/ {print $NF}' ./nt/runinfo.log | tr -d [:blank:])
      tid_rfo=$(awk -F "=" '/num_cores_per_socket/ {print $NF}' ./rfo/runinfo.log | tr -d [:blank:])
      if [ "${tid_nt}" != "${tid_rfo}" ]; then
        echo "mismatch config between nt and rfo logs..quitting!"
        exit 1
      else
        tid=${tid_nt}
      fi
    elif [ "$1" == "node" ]; then
      tid_nt=$(awk -F "=" '/num_cores_total/ {print $NF}' ./nt/runinfo.log | tr -d [:blank:]) 
      tid_rfo=$(awk -F "=" '/num_cores_total/ {print $NF}' ./rfo/runinfo.log | tr -d [:blank:]) 
      if [ "${tid_nt}" != "${tid_rfo}" ]; then
        echo "mismatch config between nt and rfo logs..quitting!"
        exit 1
      else
        tid=${tid_nt}
      fi
    fi

    for stype in nt rfo
    do
      echo -e "#Arch\tCopy\tTriad\tReduce\tFill" > ./mb_${tag}_${stype}.stats
      if [[ "${stype}" == "rfo" && $scale_rfo -eq 1 ]]; then
        awk -v arch=${arch} -v sf=${sf_copy}  '/Copy:/  {printf ("%s\t%.2f",arch,($2*sf)/1000)}' ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                 -v sf=${sf_triad} '/Triad:/ {printf ("\t%.2f",($2*sf)/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                                   '/Reduce:/ {printf ("\t%.2f",($2)/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                 -v sf=${sf_fill} '/Fill:/ {printf ("\t%.2f\n",($2*sf)/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
      else
        awk -v arch=${arch} '/Copy:/  {printf ("%s\t%.2f",arch,$2/1000)}' ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                 '/Triad:/ {printf ("\t%.2f",$2/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                 '/Reduce:/ {printf ("\t%.2f",$2/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
        awk                 '/Fill:/ {printf ("\t%.2f\n",$2/1000)}'   ${stype}/stream_*_${tid}t.log >> ./mb_${tag}_${stype}.stats
      fi

    done
    popd > /dev/null
  done


  rm -f mb_${tag}_nt.stats
  rm -f mb_${tag}_rfo.stats

  for arch in ${archs[*]};
  do
    src_dir=${arch}/${l1_dir}/${l2_dir}/${l3_dir}
    for stype in nt rfo
    do
      cat ${src_dir}/mb_${tag}_${stype}.stats >> mb_${tag}_${stype}.stats
      awk '!/#/ {print "Copy\t"$2"\nTriad\t"$3"\nReduce\t"$4"\nFill\t"$5}' ${src_dir}/mb_${tag}_${stype}.stats > tmp_${arch}_${stype}
    done

    echo -e "#Kernal\tNT\tKernel\tRFO\tNT/RFO" > ${arch}_${tag}_nt_rfo.stats
    paste tmp_${arch}_nt tmp_${arch}_rfo >> ${arch}_${tag}_nt_rfo.stats

    cp ${arch}_${tag}_nt_rfo.stats tmp_nt_rfo.stats
    awk '{if (NR>1) printf("%s\t%.2f\n",$0,$2/$NF); else print $0}' tmp_nt_rfo.stats > ${arch}_${tag}_nt_rfo.stats

    rm tmp_*
    mv ${arch}_${tag}_nt_rfo.stats ${dst_dir}/
  done

  mv mb_${tag}_nt.stats ${dst_dir}/
  mv mb_${tag}_rfo.stats ${dst_dir}/

  pushd ${dst_dir} > /dev/null
  flist=""
  for arch in ${archs[*]};
  do
    flist+=$(echo "${arch}_${tag}_nt_rfo.stats ")
  done

  ofile=$(echo ${archs[*]} | sed 's/ /_/g')_${tag}_nt_rfo.stats
  paste ${flist[*]} > ${ofile}

  infile=${ofile}
  for stype in nt rfo
  do
    if [ "$stype" == "nt" ]; then
      col=2
    else
      col=4
    fi
    ofile=$(echo ${archs[*]} | sed 's/ /_/g')_${tag}_${stype}.stats
    echo "Kernel&$(echo ${archs[*]} | sed 's/ \+/\&/g')" > ${ofile}
    awk -v col=$col '{if (NR>1) { printf("%s ",$1); for(i=col;i<NF;i+=5) {printf("& %.2f ", $i)} printf("\n");} }' ${infile} >> ${ofile}

    tex_ofile=$(basename ${ofile} .stats).tex
    echo "\begin{tabular}{|c|c|c|c|c|c|c|}  \hline" > ${tex_ofile}
    awk -F "&" '{if (NR>1) {printf("%s & %.2f & %.2f & %.2f \\\\ \\hline \n",$0,$NF/$2,$(NF-1)/$2,$NF/$(NF-1))} else {printf("%s & %s/%s & %s/%s & %s/%s \\\\ \\hline \n", $0, $NF,$2, $(NF-1),$2, $NF,$(NF-1))}}' $ofile >> ${tex_ofile}
    echo "\end{tabular}" >> ${tex_ofile}
  done


  echo -e "results in ${dst_dir}"
  popd > /dev/null
}


function get_mem_bw_scale()
{
  tag=scale
  dst_dir=${results_root}/mem_bw_${tag}
  mkdir -p ${dst_dir}
  echo "Getting memory b/w stats for \"$tag\".."

  for arch in ${archs[*]};
  do
    for aff_type in compact distribute
    do
      src_dir=${arch}/${l1_dir}/${l2_dir}/${aff_type}
      if [ ! -d ${src_dir} ]; then
        continue
      fi
    pushd ${src_dir} > /dev/null
    echo -e "\t${arch} with ${aff_type} affinity.."

    for stype in nt rfo
    do
      echo -e "#${arch}\t\t\t\t" > ./mb_${tag}_${aff_type}_${stype}.stats
      echo -e "#Threads\tCopy\tTriad\tReduce\tFill" >> ./mb_${tag}_${aff_type}_${stype}.stats

      num_cores_per_socket=$(awk -F "=" '/num_cores_per_socket/ {print $NF}' ./${stype}/runinfo.log | tr -d [:blank:])
      if [[ "${stype}" == "rfo" && $scale_rfo -eq 1 ]]; then
        # for f in $(ls -1 ${stype}/stream_*.log | sort -g -t "_" -k3)
        for t in $(seq 1 ${num_cores_per_socket})
        do
          f=${stype}/stream_*_${t}t.log
          awk '/Number of Threads counted/ {printf ("%d",$NF)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk -v sf=${sf_copy} '/Copy:/  {printf ("\t%.2f",($2*sf)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk -v sf=${sf_triad} '/Triad:/ {printf ("\t%.2f",($2*sf)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk                   '/Reduce:/ {printf ("\t%.2f",($2)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk -v sf=${sf_fill} '/Fill:/ {printf ("\t%.2f\n",($2*sf)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
        done
      else
        # for f in $(ls -1 ${stype}/stream_*.log | sort -g -t "_" -k3)
        for t in $(seq 1 ${num_cores_per_socket})
        do
          f=${stype}/stream_*_${t}t.log
          awk '/Number of Threads counted/ {printf ("%d",$NF)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk '/Copy:/  {printf ("\t%.2f",($2)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk '/Triad:/ {printf ("\t%.2f",($2)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk '/Reduce:/ {printf ("\t%.2f",($2)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
          awk '/Fill:/ {printf ("\t%.2f\n",($2)/1000)}' $f >> ./mb_${tag}_${aff_type}_${stype}.stats
        done
      fi

    done
    cp mb_${tag}_${aff_type}_nt.stats ${dst_dir}/${arch}_mb_${tag}_${aff_type}_nt.stats
    cp mb_${tag}_${aff_type}_rfo.stats ${dst_dir}/${arch}_mb_${tag}_${aff_type}_rfo.stats

    echo "#Speed-up of NT over RFO" > ${dst_dir}/${arch}_mb_${tag}_${aff_type}_nt_rfo.stats
    echo -e "#Threads\tCopy\tTriad\tReduce\tFill" >> ${dst_dir}/${arch}_mb_${tag}_${aff_type}_nt_rfo.stats
    paste mb_${tag}_${aff_type}_nt.stats mb_${tag}_${aff_type}_rfo.stats > tmp_nt_rfo.stats
    awk '{if (NR>2) printf("%d\t%.2f\t%.2f\t%.2f\t%.2f\n", $1, $2/$7, $3/$8, $4/$9, $5/$10);}' tmp_nt_rfo.stats >> ${dst_dir}/${arch}_mb_${tag}_${aff_type}_nt_rfo.stats

    popd > /dev/null
  done
  done

  echo -e "results in ${dst_dir}"
}

function gen_numa_plot()
{
  tag=numa
  pushd mem_bw_${tag} > /dev/null

  echo "Generating plots for \"${tag}\".."

  for arch in ${archs[*]};
  do
    for nps in 1 2 4
    do
      for aff_type in compact
      do
        for stype in nt rfo
        do
          in_file=${arch}_${tag}_nps${nps}_${aff_type}_${stype}.stats
            if [ ! -f ${in_file} ]; then
              continue
            fi
            # num_thrs=$(echo ${in_file} | grep -E "_[0-9]+t[_|\.]" -o | grep [[:digit:]] -o | tr -d "\n")
            out_file=$(basename ${in_file} .stats)
gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid
set key box outside bottom center horizontal
set output "./${out_file}.png"
set title "STREAM: ${arch} 1-Core, 1-Socket - NUMA bandwidth with ${stype}"
set xlabel "NUMA domain ID"
set ylabel "GB/sec"
plot "./${in_file}" using 2:xticlabels(1) with linespoints lw 3 lt 1 lc 1 title "COPY - 1-core",\
     ''             using 3:xticlabels(1) with linespoints lw 3 lt 1 lc 2 title "TRIAD - 1-core",\
     ''             using 4:xticlabels(1) with linespoints lw 3 lt 1 lc 3 title "REDUCE - 1-core",\
     ''             using 5:xticlabels(1) with linespoints lw 3 lt 1 lc 4 title "FILL - 1-core",\
     ''             using 7:xticlabels(1) with linespoints lw 3 lt 2 lc 1 title "COPY - 1-socket",\
     ''             using 8:xticlabels(1) with linespoints lw 3 lt 2 lc 2 title "TRIAD - 1-socket",\
     ''             using 9:xticlabels(1) with linespoints lw 3 lt 2 lc 3 title "REDUCE - 1-socket",\
     ''             using 10:xticlabels(1) with linespoints lw 3 lt 2 lc 4 title "FILL - 1-socket"
EOF
        done
      done
    done
  done

popd > /dev/null

}

function gen_plot()
{
  if [[ "$1" != "core" && "$1" != "socket" && "$1" != "node" ]]; then
    echo "unknown option -- $1 .. quitting"
    exit 1
  fi

  tag=$1
  pushd mem_bw_${tag} > /dev/null
  echo "Generating plots for \"${tag}\".."

  for f in mb_${tag}_{rfo,nt}.stats;
  do
    if [[ "$f" =~ "_rfo" ]]; then
       stype=RFO
    elif [[ "$f" =~ "_nt" ]]; then
       stype=NT
    else
      stype=""
    fi
    out_file=$(basename $f .stats)

gnuplot <<- EOF
# set terminal postscript enhanced color "Helvetica" 20
# set terminal postscript enhanced color
# set terminal png enhanced
set terminal pngcairo enhanced
# set key left top
# set key right bottom
# set key outside
# set key horizontal
# set output "./test.eps"
set key box outside bottom center horizontal
set output "./${out_file}.png"
set grid
# set title "STREAM: ${tag}" font "Helvetica,16"
set title "STREAM: 1-${tag}"
# set xlabel "Size"
set ylabel "GB/sec"
# plot "./mem_bw_core/mb_core_rfo.stats" using 2:xticlabels(1) with linespoints lt 1 lc 1 lw 5 title "COPY-RFO", "./mem_bw_core/mb_core_rfo.stats" using 3 with linespoints lt 2 lc 2 lw 5 title "TRIAD-RFO"
plot "./${f}" using 2:xticlabels(1) with linespoints lw 3 title "COPY-${stype}", '' u 3 with linespoints lw 3 title "TRIAD-${stype}", '' u 4 with linespoints lw 3 title "REDUCE-${stype}", '' u 5 with linespoints lw 3 title "FILL-${stype}


EOF
# convert -density 300 -flatten -rotate 90 test.eps test.png
  done

  for arch in ${archs[*]}
  do
    in_file=${arch}_${tag}_nt_rfo.stats
    out_file=$(basename ${in_file} .stats)

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key center left
set output "./${out_file}.png"
set title "STREAM: ${arch} 1-${tag}"
set ylabel "GB/sec"
plot "./${in_file}" using 2:xticlabels(1) with linespoints lw 3 title "NT", '' using 4:xticlabels(1) with linespoints lw 3 title "RFO"
EOF

  done

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
# set key top right
set key box outside bottom center horizontal
set output "./mb_${tag}_nt_rfo.png"
set title "STREAM: 1-${tag} Speed-up of NT stores over RFO"
set ylabel "Speed-up"
plot "./${archs[0]}_${tag}_nt_rfo.stats" using 5:xticlabels(1) with linespoints lw 3 title "${archs[0]}",\
     "./${archs[1]}_${tag}_nt_rfo.stats" using 5:xticlabels(1) with linespoints lw 3 title "${archs[1]}",\
     "./${archs[2]}_${tag}_nt_rfo.stats" using 5:xticlabels(1) with linespoints lw 3 title "${archs[2]}"
EOF


  

popd > /dev/null
}

function gen_scaling_plot()
{
  tag=scale
  pushd mem_bw_${tag} > /dev/null
  echo "Generating plots for \"${tag}\".."

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

  for aff_type in compact
  do
  for stype in nt rfo
  do
    out_file=mb_${tag}_${aff_type}_${kernel}_${stype}

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${kernel} ${stype} stores scaling with ${aff_type} affinity"
set xlabel "Number of Threads"
set ylabel "GB/sec"
plot "./${archs[0]}_mb_${tag}_${aff_type}_${stype}.stats" using 1:${col_id} with linespoints lw 3 title "${archs[0]}",\
     "./${archs[1]}_mb_${tag}_${aff_type}_${stype}.stats" using 1:${col_id} with linespoints lw 3 title "${archs[1]}",\
     "./${archs[2]}_mb_${tag}_${aff_type}_${stype}.stats" using 1:${col_id} with linespoints lw 3 title "${archs[2]}"
EOF
  done

  out_file=mb_${tag}_${aff_type}_${kernel}_nt_rfo
gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${kernel} scaling, Speed-up of NT over RFO stores"
set xlabel "Number of Threads"
set ylabel "Speed-up"
plot "./${archs[0]}_mb_${tag}_${aff_type}_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "${archs[0]}",\
     "./${archs[1]}_mb_${tag}_${aff_type}_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "${archs[1]}",\
     "./${archs[2]}_mb_${tag}_${aff_type}_nt_rfo.stats" using 1:${col_id} with linespoints lw 3 title "${archs[2]}"
EOF

  done
  done



  for arch in ${archs[*]}
  do
    for aff_type in compact #distribute
    do
      in_file1=${arch}_mb_${tag}_${aff_type}_nt.stats
      in_file2=${arch}_mb_${tag}_${aff_type}_rfo.stats
      
      if [[ ! -f ${in_file1} || ! -f ${in_file2} ]]; then
        continue
      fi

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
        out_file=${arch}_${tag}_${aff_type}_${kernel}_nt_rfo

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${arch} ${kernel} scaling with ${aff_type} affinity"
set xlabel "Number of Threads"
set ylabel "GB/sec"
plot "./${in_file1}" using 1:${col_id} with linespoints lw 3 title "NT",\
     "./${in_file2}" using 1:${col_id} with linespoints lw 3 title "RFO"
EOF
  done
  done
  done

  for arch in ${archs[*]}
  do
    for stype in nt rfo
    do
      in_file1=${arch}_mb_${tag}_compact_${stype}.stats
      in_file2=${arch}_mb_${tag}_distribute_${stype}.stats
      
      if [[ ! -f ${in_file1} || ! -f ${in_file2} ]]; then
        continue
      fi

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
        out_file=${arch}_${tag}_${kernel}_affinity_${stype}

gnuplot <<- EOF
set terminal pngcairo enhanced
set grid
set key bottom right
set output "./${out_file}.png"
set title "STREAM: ${arch} ${kernel} ${stype} stores scaling Compact vs Distributed Affinity"
set xlabel "Number of Threads"
set ylabel "GB/sec"
plot "./${in_file1}" using 1:${col_id} with linespoints lw 3 title "Compact",\
     "./${in_file2}" using 1:${col_id} with linespoints lw 3 title "Distributed"
EOF
  done

    out_file=${arch}_${tag}_affinity_${stype}

# newer version of gnuplot has dash-type (dt "-")
gnuplot <<- EOF
set terminal pngcairo dashed enhanced
set grid
# set key top right
set key box outside bottom center horizontal
set output "./${out_file}.png"
set title "STREAM: ${arch} ${stype} stores scaling Compact vs Distributed Affinity"
set xlabel "Number of Threads"
set ylabel "GB/sec"
plot "./${in_file1}" using 1:2 with lines lw 3 lt 1 lc 1 title "COPY - Compact",\
     "./${in_file1}" using 1:3 with lines lw 3 lt 1 lc 2 title "TRIAD - Compact",\
     "./${in_file1}" using 1:4 with lines lw 3 lt 1 lc 3 title "REDUCE - Compact",\
     "./${in_file1}" using 1:5 with lines lw 3 lt 1 lc 4 title "FILL - Compact",\
     "./${in_file2}" using 1:2 with lines lw 3 lt 2 lc 1 title "COPY - Distributed",\
     "./${in_file2}" using 1:3 with lines lw 3 lt 2 lc 2 title "TRIAD - Distributed",\
     "./${in_file2}" using 1:4 with lines lw 3 lt 2 lc 3 title "REDUCE - Distributed",\
     "./${in_file2}" using 1:5 with lines lw 3 lt 2 lc 4 title "FILL - Distributed"
EOF


  done
  done


  popd > /dev/null

}

# Globals
archs=("Rome" "CLX" "ICX")
l1_dir="nps*"
l2_dir="uma"
l3_dir="compact"
results_root="$(pwd)"


# scaling rfo not done in NUMA
scale_rfo=0
if [ $scale_rfo -eq 1 ]; then
  sf_copy=1.5
  sf_triad=1.33
  sf_fill=2.0
else
  sf_copy=1.0
  sf_triad=1.0
  sf_fill=1.0
fi


# check_logs

set -e
get_mem_bw core
gen_plot core

get_mem_bw socket
gen_plot socket

get_mem_bw node
gen_plot node

get_mem_bw_scale
gen_scaling_plot

get_mem_bw_numa
gen_numa_plot

