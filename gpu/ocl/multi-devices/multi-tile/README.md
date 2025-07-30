This directory contains the OpenCL version of STREAM bandwidth kernels that can be run on 1 or more tiles of Intel GPU's. The following memory traffic kernels are available:

1.  Triad (2 read + 1 write)
2.  Scale/Copy (1 read + 1 write)
3.  100% read traffic
4.  100% write traffic

**Compilation:** <br>
Edit the Makefile to point to OpenCL runtime on your system. Running, "make" would produce the following 4 binaries corresponding to the above mentioned traffic patterns:

1.  stream-triad
2.  stream-scale
3.  stream-ro (read-only)
4.  stream-wo (write-only)

**Execution:** <br>
All the binaries support the following options. Among the options, -b is mandatory. To accurately measure memory bandwidth performance, it is necessary to use large sized buffers as input to -b flag. This will ensure that the data is not loaded/stored from/to EU caches. If you want to run only on a specific number of tiles of a multi-tile GPU card, use -d flag. The default is to use all available compute tiles of a GPU card.

```
USAGE:  ./stream-triad [at least one of -b <num> or -n <num>] [-t] [-d] [-q*] [-v*] [-h]
DESCRIPTION
         Run OpenCL STREAM Kernels.
         Data-Type is double
OPTIONS
        -b,--bytes
            run with given # of bytes
        -n,--num-elements
            run with given # of elements
        -d,--num-subDevices
            run with given # of sub devices
        -t,--times <n>
            repeat benchmark n times
        -o,--ocl-profiling <flag>
            use OCL events (don't use with AUB collection!)
        -v,--verbose
            increase verbosity
        -q,--quiet
            decrease verbosity
        -h,--help
            print this help message
```


**Examples**: <br>
Run Triad with 1 GB/buffer on all tiles: ./stream-triad -b1g <br>
Run Triad with 500 MB/buffer on 1-tile: ./stream-triad -b500m -d1 <br>

**Benchmark:** <br>
Running "bench.sh" would run all 4 kernels with 1 GB/buffer partitioned equally among all available compute tiles of a GPU card. Each kernel is repeated 100 times and average, best performance is reported.
