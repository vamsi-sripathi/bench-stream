#!/bin/bash

use_intel=1
if [ "${use_intel}" == "1" ]; then
  make clean && make extra_kernels=1 cpu_target=avx512           && ./bench.sh
  make clean && make extra_kernels=1 cpu_target=avx512 use_rfo=1 && ./bench.sh
else
  make clean && make extra_kernels=1 cpu_target=avx2 amd=1           && ./bench.sh
  make clean && make extra_kernels=1 cpu_target=avx2 amd=1 use_rfo=1 && ./bench.sh
fi
