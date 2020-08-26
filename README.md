# Stream

STREAM benchmark for Intel CPU's

**Pre-requisites: Intel C Compiler**

**Compilation:  Run `make`**

Running 'make' with no options would compile the STREAM benchmark with 134217728 FP64 elements per array for following Intel CPU's:

```
        stream_avx.bin        => Targeted for Intel CPU's that support AVX ISA
        stream_avx2.bin       => Targeted for Intel CPU's that support AVX2 ISA
        stream_avx512.bin     => Targeted for Intel CPU's that support AVX512 ISA
```

The following options are supported:
```
        stream_array_size=<number_of_elements_per_array>

        cpu_target=<avx,avx2,avx512>
```

`make help` shows the following menu on command-line:
```
Running 'make' with no options would compile the STREAM benchmark with 134217728  FP64 elements per array for following Intel CPU's:

        stream_avx.bin        => Targeted for Intel CPU's that support AVX ISA
        stream_avx2.bin       => Targeted for Intel CPU's that support AVX2 ISA
        stream_avx512.bin     => Targeted for Intel CPU's that support AVX512 ISA

The following options are supported:
        stream_array_size=<number_of_elements_per_array>

        cpu_target=<avx,avx2,avx512>

Few examples:
To compile STREAM benchmark only for Intel AVX512 CPU's, do:
        make cpu_target=avx512

To compile STREAM benchmark for Intel AVX512 CPU's with each buffer containing 67108864 elements, do:
        make stream_array_size=67108864 cpu_target=avx512
```


**Running the benchmark: Execute ./bench.sh**

The benchmark script does the following --

1.  Use the appropriate stream benchmark binary produced from the compilation step, i.e picks the highest supported ISA on your target system
2.  Run with OMP_NUM_THREADS set to number of available physical cores and KMP_AFFINITY set to compact pinning. Ignores Hyper-threading cores even if enabled on system.
3.  Store the results to a log file and outputs summary performance of STREAM kernels (Copy, Scale, Add, Triad)
4.  Also, output relevant system info such as number of sockets, NUMA, memory sub-system etc. Running with sudo would result in more detailed info on memory sub-system as it parses output of `dmidecode`

Below is the sample output of compilation and execution steps on a Intel CLX test system --

```
[vsripath@ortce-cl2 ~/repos/stream]$ make

======= Generating STREAM benchmark with AVX ISA =======
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xAVX -DNTIMES=100 -DOFFSET=0 -DSTREAM_TYPE=double -DSTREAM_ARRAY_SIZE=134217728  -c stream.c -o stream_avx.o
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xAVX stream_avx.o -o stream_avx.bin

======= Generating STREAM benchmark with AVX2 ISA =======
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xCORE-AVX2 -DNTIMES=100 -DOFFSET=0 -DSTREAM_TYPE=double -DSTREAM_ARRAY_SIZE=134217728  -c stream.c -o stream_avx2.o
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xCORE-AVX2 stream_avx2.o -o stream_avx2.bin

======= Generating STREAM benchmark with AVX512 ISA =======
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xCORE-AVX512 -qopt-zmm-usage=high -DNTIMES=100 -DOFFSET=0 -DSTREAM_TYPE=double -DSTREAM_ARRAY_SIZE=134217728  -c stream.c -o stream_avx512.o
icc -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always -xCORE-AVX512 -qopt-zmm-usage=high stream_avx512.o -o stream_avx512.bin
[vsripath@ortce-cl2 ~/repos/stream]$ ./bench.sh

CPU Model =  Intel(R) Xeon(R) Platinum 8260L CPU @ 2.40GHz

Sockets/Cores/Threads:
        num_sockets          = 2
        num_cores_total      = 48
        num_cores_per_socket = 24
        num_threads_per_core = 2
        Hyper-Threading      = true

NUMA:
        num_numa_domains            = 2
        num_numa_domains_per_socket = 1
        num_cores_per_numa_domain   = 24

Memory = 196.73 GB

CPU Caches:
        L1_cache = 32K (8-way)
        L2_cache = 1024K (16-way)
        L3_cache = 36608K (11-way)
        L3_cache_per_sock = 36600 KB
        L3_cache_per_core = 1525 KB

OS:
Operating System       = Fedora 27 (Server Edition)
Kernel version         = 4.18.16-100.fc27.x86_64
CPU Turbo Boost        = disabled
CPU Scaling Governor   = performance
Transparent Huge Pages = enabled

Target ISA  = avx512f
ICC version = icc (ICC) 19.1.1.217 20200306

Running stream_avx512.bin with 48 threads, output log will be saved in stream_avx512_48t.log
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:          205381.2     0.010506     0.010456     0.010738
Scale:         204407.1     0.010552     0.010506     0.010731
Add:           216988.7     0.014908     0.014845     0.015125
Triad:         216293.9     0.014960     0.014893     0.015130
-------------------------------------------------------------
Solution Validates: avg error less than 1.000000e-13 on all three arrays
-------------------------------------------------------------
```
