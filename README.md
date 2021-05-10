# Stream

STREAM benchmark for Intel CPU's. This branch contains the standard STREAM benchmark as written by Dr. McCalpin at https://www.cs.virginia.edu/stream/

In addition to the standard kernels (Copy, Scale, Add, Triad), two additional memory traffic kernels (100% Read aka Reduce, 100% Write aka Fill) were developed.

**Pre-requisites: Intel C Compiler**

**Compilation:  Run `make`**

Running make without any options would use the "un-modified" STREAM benchmark source to produce a binary optimized for Intel platforms.
Running make with "extra_kernels=1" would produce a binary that also includes the aforementioned additional kernels.
Compilation defaults/controls can be viewed by running, `make help` and result in following output:

```
Running 'make' with no options would compile the STREAM benchmark with 269000000 FP64 elements per array for following Intel CPU's:

        stream_avx.bin        => Targeted for Intel CPU's that support AVX ISA
        stream_avx2.bin       => Targeted for Intel CPU's that support AVX2 ISA
        stream_avx512.bin     => Targeted for Intel CPU's that support AVX512 ISA

The following options are supported:
        stream_array_size=<number_of_elements_per_array>

        cpu_target=<avx,avx2,avx512>
        
        amd=1 generates binary targeted for AMD processors

        use_rfo=1 forces to use regular stores instead of Non-temporal stores

        extra_kernels=1 include additonal kernels (reduce, fill) in generated binary

Few examples:
To compile STREAM benchmark only for Intel AVX512 CPU's, do:
        make cpu_target=avx512

To compile STREAM benchmark for Intel AVX512 CPU's with each buffer containing 67108864 elements, do:
        make stream_array_size=67108864 cpu_target=avx512

To compile STREAM with extra kernels that are not part of standard benchmark, do:
        make extra_kernels=1

To compile STREAM benchmark for AMD, do:
        make amd=1

To explicitly compile STREAM benchmark that uses regular stores/RFO's, do:
        make use_rfo=1
```

**Running the benchmark:**
1.  Execute bench.sh

The benchmark script does the following --
1.  Use the appropriate stream benchmark binary produced from the compilation step, i.e picks the highest supported ISA on your target system
2.  Runs with OMP_NUM_THREADS set to 1-thread, max. threads per 1-socket, all threads in a node. KMP_AFFINITY set to compact pinning. Ignores Hyper-threading cores even if enabled on system.
3.  Store the results under a directory named after the target CPU model name. Additional system info such as number of sockets, NUMA, memory sub-system etc are also dumped. Running with sudo would result in more detailed info on memory sub-system as it parses output of `dmidecode`
