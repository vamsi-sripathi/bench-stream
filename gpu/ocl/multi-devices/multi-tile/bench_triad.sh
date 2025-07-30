#!/bin/bash

set -x

res_dir=$(hostname)
mkdir -p ${res_dir}

clinfo > ${res_dir}/clinfo.log
env > ${res_dir}/env.log

for sz in 32 64 128 256 512 1024;
do
  ./stream-triad -b${sz}m -d1 -t500 &> ${res_dir}/triad_1T_${sz}m.log

  grep "sub-device\[0\]-trial.*MTR" ${res_dir}/triad_1T_${sz}m.log | awk '{print $4}' > mtr.stats
  grep "Trial.*Bandwidth:"          ${res_dir}/triad_1T_${sz}m.log | awk '{print $3}' > bw.stats
  echo -e "#${sz}MB per buffer\n#Kernel-Only\tFull-Stack" > ${res_dir}/triad_1T_${sz}m.stats
  paste mtr.stats bw.stats >> ${res_dir}/triad_1T_${sz}m.stats
done

rm -f mtr.stats bw.stats
