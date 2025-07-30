#!/bin/bash

function mach_info()
{
   num_socks=$(lscpu | grep "Socket(s):" | awk '{print $NF}')
   num_cores_per_sock=$(lscpu | grep "Core(s) per socket:" | awk '{print $NF}')
   num_threads_per_core=$(lscpu | grep "Thread(s) per core:" | awk '{print $NF}')
   ht_enabled=$( [ "$num_threads_per_core" -gt 1 ] && echo "true" || echo "false")
   num_cores_total=$(($num_socks*$num_cores_per_sock))

   num_numa_domains=$(numactl -H | grep "available:" | awk '{print $2}')
   num_numa_domains_per_sock=$(($num_numa_domains/$num_socks))
   num_cores_per_numa_domain=$(numactl -H | grep "node 0 cpus:" | awk -F ":" '{print $NF}' | awk '{print NF}')
   num_cores_per_numa_domain=$(($num_cores_per_numa_domain/$num_threads_per_core))

   l1_cache_size=$(cat /sys/devices/system/cpu/cpu0/cache/index0/size)
   l1_cache_ways=$(cat /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity)
   l2_cache_size=$([ -f "/sys/devices/system/cpu/cpu0/cache/index2/size" ] && cat /sys/devices/system/cpu/cpu0/cache/index2/size)
   l2_cache_ways=$([ -f "/sys/devices/system/cpu/cpu0/cache/index2/ways_of_associativity" ] && cat /sys/devices/system/cpu/cpu0/cache/index2/ways_of_associativity)
   if [ -f "/sys/devices/system/cpu/cpu0/cache/index3/size" ]; then
     l3_cache_size=$(cat /sys/devices/system/cpu/cpu0/cache/index3/size)
     l3_cache_ways=$(cat /sys/devices/system/cpu/cpu0/cache/index3/ways_of_associativity)
     # todo: make this robust
     l3_cache_shared_cpu_count=$(cat /sys/devices/system/cpu/cpu0/cache/index3/shared_cpu_list  | awk -F "," '{print $1}' | awk -F "-" '{print $NF}')
     l3_cache_shared_cpu_count=$(($l3_cache_shared_cpu_count+1))
     l3_cache_size_per_core=$(($(echo $l3_cache_size | tr -d "[:alpha:]")/$l3_cache_shared_cpu_count))
     l3_cache_size_per_sock=$(($l3_cache_size_per_core*$num_cores_per_sock))
     l3_cache_size_per_core+=" KB"
     l3_cache_size_per_sock+=" KB"
   fi
   
   if [ "$EUID" -eq 0 ]; then
     #todo: iterate over all md to verify homogenous config
     md_size=$(dmidecode -t memory | grep "Memory Device" -A21  | grep -E 'Size:[[:space:]]*[[:digit:]]+' -m1)
     md_type=$(dmidecode -t memory | grep "Memory Device" -A21  | grep -E 'Type:[[:space:]]*DDR' -m1 | awk '{print $NF}')
     md_speed=$(dmidecode -t memory | grep "Memory Device" -A21  | grep -v "Configured Memory Speed"  | grep -E 'Speed:[[:space:]]*[[:digit:]]+' -m1 | awk -F ":" '{print $NF}')
     mem_vendor=$(dmidecode -t memory | grep "${md_size}" -m1 -A21 | grep -m1 "Manufacturer:" | awk '{print $NF}')
     # md_configured_speed=$(dmidecode -t memory | grep "Memory Device" -A21 -m1 | grep -E 'Configured Clock Speed:' | awk -F ":" '{print $NF}')
     # todo: add 'Locator' info and num_mem_channels_per_sock
     num_mem_channels=$(dmidecode -t memory | grep -c "${md_size}")
     size=$(echo $md_size | awk '{print $2}')
     mem_size_total=$(($size*$num_mem_channels))
     mem_size_total+=$(echo " $(echo $md_size | awk '{print $NF}')")
     mem_speed=$md_speed
     mem_type=$md_type
     mem_size_per_dimm=$(echo $md_size | awk -F ":" '{print $NF}')
     peak_mem_bw_system=$(echo "scale=2; (8 * $num_mem_channels * $(echo $mem_speed | tr -d "[:alpha:]|[:punct:]" ) / 1000)" | bc -l)
     peak_mem_bw_per_sock=$(echo "scale=2; ($peak_mem_bw_system / $num_socks)" | bc -l)
   else
     memory_size_total=$(cat /proc/meminfo | grep "MemTotal" | awk '{printf ("%.2f GB",$2/1000/1000)}')
   fi


   model_name=$(lscpu | grep "Model name:" | awk -F ":" '{print $NF}' | tr -s "[:space:]")
   os_name=$(cat /etc/os-release  | grep "PRETTY_NAME" | awk -F "=" '{print $NF}' | tr -d "\"")
   kernel_release=$(uname -r)
   hostname=$(hostname -f)
   thp=$( [ "$(grep -o "\[always\]" /sys/kernel/mm/*transparent_hugepage/enabled)" == "[always]" ] && echo "enabled" || echo "disabled")
   icc_version=$(icc --version | head -n1)

   if [ -f /sys/devices/system/cpu/intel_pstate/no_turbo ]; then
      cpu_turbo=$( [ "$(cat /sys/devices/system/cpu/intel_pstate/no_turbo)" == 1 ] && echo "disabled" || echo "enabled")
   fi
   cpu_scaling_governor=$(cat /sys/devices/system/cpu/cpu[0-9]*/cpufreq/scaling_governor | sort -u)
   
   lscpu_flags=$(lscpu | grep "Flags:")
   
   for isa in avx512f avx2 avx
   do
       echo ${lscpu_flags} | grep -w ${isa} &> /dev/null
        if [ $? -eq 0 ]; then
          target_cpu=${isa}
          break
         fi
   done
}

function show_mach_info()
{
  echo -e "\nCPU Model = $model_name"
  echo -e "\nSockets/Cores/Threads:"
  echo -e "\tnum_sockets          = $num_socks"
  echo -e "\tnum_cores_total      = $num_cores_total"
  echo -e "\tnum_cores_per_socket = $num_cores_per_sock"
  echo -e "\tnum_threads_per_core = $num_threads_per_core"
  echo -e "\tHyper-Threading      = $ht_enabled"
  
  echo -e "\nNUMA:"
  echo -e "\tnum_numa_domains            = $num_numa_domains"
  echo -e "\tnum_numa_domains_per_socket = $num_numa_domains_per_sock"
  echo -e "\tnum_cores_per_numa_domain   = $num_cores_per_numa_domain"

  if [ "$EUID" -eq 0 ]; then
    echo -e "\nMemory:"
    echo -e "\tmem_vendor           = $mem_vendor"
    echo -e "\tmem_speed            = $mem_speed"
    echo -e "\tmem_type             = $mem_type"
    echo -e "\tmem_size_total       = $mem_size_total"
    echo -e "\tmem_size_per_dimm    = $mem_size_per_dimm"
    echo -e "\tnum_mem_channels     = $num_mem_channels"
    echo -e "\tpeak_mem_bw_system   = $peak_mem_bw_system GB/sec"
    echo -e "\tpeak_mem_bw_per_sock = $peak_mem_bw_per_sock GB/sec"
  else
    echo -e "\nMemory = $memory_size_total"
  fi

  echo -e "\nCPU Caches:"
  echo -e "\tL1_cache = $l1_cache_size (${l1_cache_ways}-way)"
  echo -e "\tL2_cache = $l2_cache_size (${l2_cache_ways}-way)"
  echo -e "\tL3_cache = $l3_cache_size (${l3_cache_ways}-way)"
  echo -e "\tL3_cache_per_sock = $l3_cache_size_per_sock"
  echo -e "\tL3_cache_per_core = $l3_cache_size_per_core"

  echo -e "\nOS:"
  echo -e "Operating System       = $os_name"
  echo -e "Kernel version         = $kernel_release"
  echo -e "CPU Turbo Boost        = $cpu_turbo"
  echo -e "CPU Scaling Governor   = $cpu_scaling_governor"
  echo -e "Transparent Huge Pages = $thp"

  echo ""
  echo "Target ISA  = ${target_cpu}"
  echo "ICC version = ${icc_version}"
  echo ""

}

mach_info
show_mach_info

if [ "${target_cpu}" == "avx512f" ]; then
   binary=stream_avx512.bin
elif [ "${target_cpu}" == "avx2" ]; then
   binary=stream_avx2.bin
elif [ "${target_cpu}" == "avx" ]; then
   binary=stream_avx.bin
else
   echo "Unknown ISA, aborting.."
   exit 1
fi

if [ "${ht_enabled}" == "true" ]; then
   export KMP_AFFINITY=granularity=fine,compact,1,0
else
   export KMP_AFFINITY=compact
fi

for t in ${num_cores_total}
do
  export OMP_NUM_THREADS=$t
  res_file=$(basename ${binary} .bin)_${t}t.log
  echo "Running ${binary} with ${t} threads, output log will be saved in ${res_file}"
  # ./${binary} &> ${res_file}
  tail -n9 ${res_file}
done


