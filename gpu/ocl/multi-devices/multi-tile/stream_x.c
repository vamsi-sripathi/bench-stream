#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <CL/cl.h>
#include "ocl_util.h"
#include <math.h>
#include <libgen.h>
#include <float.h>
#include <getopt.h>
#include <unistd.h>
#include <limits.h>
#include <sys/time.h>
#include <string.h>
#include "intel_extensions.h"

#if !defined (RO_KERNEL) && !defined (WO_KERNEL) && !defined (SCALE_KERNEL) && !defined (TRIAD_KERNEL)
#error "STREAM kernel is undefined! One of RO_KERNEL, WO_KERNEL, SCALE_KERNEL, TRIAD_KERNEL should be defined!"
#endif

int verbosity = RUN_AND_SYS;

static double get_wtime()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return 1.*tv.tv_sec + 1.e-6*tv.tv_usec;
}

static char *human_format(double in)
{
  static const char cf_chars[]              = {'g', 'm', 'k', 'b'};
  static const unsigned long long cf_vals[] = {
    1073741824,
    1048576,
    1024,
    0ULL,
  };
  static const int nsuffix = sizeof(cf_vals)/sizeof(cf_vals[0]);

  int i;
  for(i = 0; in < cf_vals[i]; ++i);

  double v = in;
  if(cf_vals[i] > 0)
  {
    v = in/cf_vals[i];
  }
  char buff[1024];
  snprintf(buff, 1023, "%.1lf%c", v, cf_chars[i]);
  return strdup(buff);
}

static long long suffixed_atoll(const char *nptr)
{
  char   *mod = strdup(nptr);
  size_t  s   = strlen(mod);
  long long res2_power = 0LL;
  long long res10_power = 0LL;
  int ischar = 1;
  int power_of_two_stage = 2;
  int p;
  for(p = s-1; p >= 0 && ischar; --p)
  {
    switch(mod[p])
    {
      case 'B':
      case 'b':
        power_of_two_stage = 1;
        break;
      case 'I':
      case 'i':
        if(power_of_two_stage == 1)
        {
          power_of_two_stage = 2;
        }
        else
        {
          power_of_two_stage = 0;
        }
        break;
      case 'k':
      case 'K':
        if(power_of_two_stage == 2)
        {
          res2_power += 10;
          power_of_two_stage = 0;
        }
        else
        {
          res10_power += 3;
        }
        break;
      case 'm':
      case 'M':
        if(power_of_two_stage == 2)
        {
          res2_power += 20;
          power_of_two_stage = 0;
        }
        else
        {
          res10_power += 6;
        }
        break;
      case 'g':
      case 'G':
        if(power_of_two_stage == 2)
        {
          res2_power += 30;
          power_of_two_stage = 0;
        }
        else
        {
          res10_power += 9;
        }
        break;
      default:
        ischar = 0;

    }
    mod[p+1] = 0;
  }
  long long res = atof(mod) * (1ULL << res2_power) * pow(10,res10_power);
  free(mod);
  return res;
}

static double l2(const BASE_DTYPE *l, const BASE_DTYPE *r, int n)
{
  double v = 0.0;
  for(int i = 0; i < DCOPIES*n; ++i)
  {
    const double diff = (double)l[i] - (double)r[i];
    v += diff*diff;
  }

  return sqrt(v);
}

static void CheckError (cl_int error)
{
  if (error != CL_SUCCESS) {
    if(verbosity > NO_OUTPUT)
    {
      printf("OpenCL call failed with error %d\n",error);
    }
    exit(EXIT_FAILURE);
  }
}

static cl_mem xCreateBuffer (cl_context context, 
                             cl_device_id sub_dev_id,
                             cl_mem_flags flags,
                             size_t size, void *hostPtr, cl_int *errcodeRet)
{
  INTELpfn_clCreateBufferWithPropertiesINTEL clCreateBufferWithPropertiesINTEL = (INTELpfn_clCreateBufferWithPropertiesINTEL)
                                                                                 clGetExtensionFunctionAddressForPlatform(platform, clCreateBufferWithPropertiesINTELFunctionName);
  if (clCreateBufferWithPropertiesINTEL == NULL) {
    if (verbosity > NO_OUTPUT) {
      fprintf(stderr, "Can't find clCreateBufferWithPropertiesINTEL for this platform, kthxbye.\n");
    }
    exit(EXIT_FAILURE);
  }

  cl_mem_properties_intel props[] = {CL_MEM_FLAGS, flags, CL_MEM_DEVICE_ID_INTEL, (cl_mem_properties_intel) sub_dev_id, 0};
  return clCreateBufferWithPropertiesINTEL(context, props, 0, size, hostPtr, errcodeRet);
}

static const char *usage_str = "USAGE:\t%s [at least one of -b <num> or -n <num>] [-t] [-d] [-q*] [-v*] [-h]\n";

static void usage(char *name)
{
  fprintf(stderr, usage_str, basename(name));
  exit(EXIT_FAILURE);
}

static void help(char *name)
{
  fprintf(stderr, usage_str, name);
  fprintf(stderr, "DESCRIPTION\n"
          "\t Run OpenCL STREAM Kernels.\n\t Data-Type is %s\n", KERNEL_STREAM_DTYPE);
  fprintf(stderr, "OPTIONS\n"
          "\t-b,--bytes\n\t    run with given # of bytes\n"
          "\t-n,--num-elements\n\t    run with given # of elements\n"
          "\t-d,--num-subDevices\n\t    run with given # of sub devices\n"
          "\t-t,--times <n>\n\t    repeat benchmark n times\n"
          "\t-o,--ocl-profiling <flag>\n\t    use OCL events (don't use with AUB collection!)\n"
          "\t-v,--verbose\n\t    increase verbosity\n"
          "\t-q,--quiet\n\t    decrease verbosity\n"
          "\t-h,--help\n\t    print this help message\n"
         );
}


int main(int argc, char *argv[]) {
  int do_ocl_events = 1;
  long long ntimes = 100;
  int num_sub_devices = -1;
  long long array_n = -1;
  long long array_bytes = -1;
  double mintime, maxtime, totaltime;
  BASE_DTYPE scalar;
  long long total_bytes;
  int bw_factor;
  int opt;
  const struct option opts[] =
  {
    {"help",            no_argument,       0, 'h'},
    {"quiet",           no_argument,       0, 'q'},
    {"verbose",         no_argument,       0, 'v'},
    {"times",           required_argument, 0, 't'},
    {"num-elements",    required_argument, 0, 'n'},
    {"bytes",           required_argument, 0, 'b'},
    {"ocl-profiling",   required_argument, 0, 'o'},
    {"num-subDevices",  required_argument, 0, 'd'},
    {0, 0, 0, 0},
  };

  do {
    opt = getopt_long(argc, argv, "hqvt:n:b:o:d:", opts, 0);
    switch(opt) {
      case 0:
        break;
      case 'o':
        do_ocl_events = atoll(optarg);
        break;
      case 'n':
        array_n = suffixed_atoll(optarg);
        break;
      case 'b':
        array_bytes = suffixed_atoll(optarg);
        break;
      case 't':
        ntimes = suffixed_atoll(optarg);
        break;
      case 'd':
        num_sub_devices = atoll(optarg);
        break;
      case 'q':
        --verbosity;
        break;
      case 'v':
        ++verbosity;
        break;
      case 'h':
        help(argv[0]);
        exit(EXIT_FAILURE);
      default:
        usage(argv[0]);
      case -1:
        break;
    };
  } while (opt != -1);
  

  if (optind != argc) {
    usage(argv[0]);
  }

  if (ntimes < 0) {
    if (verbosity > NO_OUTPUT) {
      fprintf (stderr, "Number of kernel executions should be greater than 0! (got %lld.)", ntimes);
    }
    exit (EXIT_FAILURE);
  }

  if (array_n != -1) {
    long long need_size = array_n * sizeof(BASE_DTYPE)*DCOPIES;
    if (array_bytes != -1 && array_bytes != need_size) {
      if (verbosity > NO_OUTPUT) {
        fprintf(stderr, "if both num-elements and bytes are specified, they need to match up!"
                "(num-elements is %lld, bytes is %lld, but computed %lld)", array_n, array_bytes, need_size);
      }
      exit(EXIT_FAILURE);
    }
    array_bytes = need_size;
  } else if (array_bytes != -1) {
    array_n = (array_bytes + sizeof(BASE_DTYPE)*DCOPIES -1) / (sizeof(BASE_DTYPE)*DCOPIES);
    array_bytes = array_n * sizeof(BASE_DTYPE)*DCOPIES;
  } else {
    if (verbosity > NO_OUTPUT) {
      fprintf(stderr, "need at least one of num-elements and bytes specified!\n");
    }
    exit(EXIT_FAILURE);
  }

  assert(sizeof(C_STREAM_DTYPE) == DCOPIES * sizeof(BASE_DTYPE));

  setup_OCL_dev();

  if (verbosity > RUN_AND_SYS) {
    printf("Finished Initialization\n");
  }

#if defined (RO_KERNEL)
  bw_factor = 1;
#elif defined (WO_KERNEL)
  bw_factor = 1;
#elif defined (SCALE_KERNEL)
  bw_factor = 2;
#elif defined (TRIAD_KERNEL)
  bw_factor = 3;
#endif
  total_bytes = bw_factor * array_bytes;

  if (num_sub_devices == -1) {
    num_sub_devices = maxSubDevices;
  }

  if (num_sub_devices > maxSubDevices) {
    printf ("\n Requested number of sub devices (%d) cannot be greater than available sub devices (%d)\n",
            num_sub_devices, maxSubDevices);
    return 1;
  }

  size_t chunk_bytes = array_bytes/num_sub_devices;
  size_t chunk_n = array_n/num_sub_devices;
  const size_t globalWorkSize [] = {chunk_n, 0, 0};

  if (verbosity > NO_OUTPUT) {
#if defined (RO_KERNEL)
    printf ("STREAM Read-Only kernel\n");
#elif defined (WO_KERNEL)
    printf ("STREAM Write-Only kernel\n");
#elif defined (SCALE_KERNEL)
    printf ("STREAM Scale kernel\n");
#elif defined (TRIAD_KERNEL)
    printf ("STREAM Triad kernel\n");
#endif
    printf ("Number of trials kernel is executed       = %lld\n", ntimes);
    printf ("Number of elements per array              = %lld\n", array_n);
    printf ("Memory footprint per array                = %s\n", human_format(array_bytes));
    printf ("Total bytes processed by kernel           = %lld (%s)\n\n", total_bytes, human_format(total_bytes));

    printf ("Number of tiles/sub-devices used          = %d\n", num_sub_devices);
    printf ("Elements processed by each sub-device     = %ld\n", chunk_n);
    printf ("Total bytes processed by each sub-device  = %lld (%s)\n\n", (long long)bw_factor*chunk_bytes, human_format(bw_factor*chunk_bytes));
  }

  srand48(42);

  cl_event **event = (cl_event **)malloc(sizeof(cl_event *)*num_sub_devices);
  for (int i=0; i<num_sub_devices; i++) {
    event[i] = (cl_event *) malloc(sizeof(cl_event)*ntimes);
  }

  kernels = (cl_kernel *) malloc(sizeof(cl_kernel)*num_sub_devices);

  BASE_DTYPE *a = NULL, *b = NULL, *c_device = NULL, *c_host = NULL;
  cl_mem *a_bufs = NULL, *b_bufs = NULL, *c_bufs = NULL;

  // Result buffer on host and device, common for all kernels
  c_host   = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);
  c_device = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);

#if defined (RO_KERNEL)
  a = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);

  for (int i = 0; i < DCOPIES*array_n; ++i) {
    a[i]  = (BASE_DTYPE)drand48();
    c_host[i] = c_device[i] = 0.;
  }

  a_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  c_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  for (int i=0; i<num_sub_devices; i++) {
    a_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_READ_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    c_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_WRITE_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    clEnqueueWriteBuffer(commands[i], a_bufs[i], CL_TRUE, 0, chunk_bytes, a+(i*chunk_n), 0, NULL, NULL);
    clEnqueueWriteBuffer(commands[i], c_bufs[i], CL_TRUE, 0, chunk_bytes, c_device+(i*chunk_n), 0, NULL, NULL);
  }

  for (int i=0; i<num_sub_devices; i++) {
    kernels[i] = clCreateKernel(programs[i], "stream_ro", &error);
    assert(error == CL_SUCCESS);

    clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &(a_bufs[i]));
    clSetKernelArg(kernels[i], 1, sizeof(cl_mem), &(c_bufs[i]));
  }
#elif defined (WO_KERNEL)
  scalar = (BASE_DTYPE) drand48();

  for (int i = 0; i < DCOPIES*array_n; ++i) {
    c_host[i] = scalar;
    c_device[i] = 0.;
  }

  c_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  for (int i=0; i<num_sub_devices; i++) {
    c_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_WRITE_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    clEnqueueWriteBuffer(commands[i], c_bufs[i], CL_TRUE, 0, chunk_bytes, c_device+(i*chunk_n), 0, NULL, NULL);
  }

  for (int i=0; i<num_sub_devices; i++) {
    kernels[i] = clCreateKernel(programs[i], "stream_wo", &error);
    assert(error == CL_SUCCESS);

    clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &(c_bufs[i]));
    clSetKernelArg(kernels[i], 1, sizeof(BASE_DTYPE), &scalar);
  }
#elif defined (SCALE_KERNEL)
  a = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);
  scalar = (BASE_DTYPE) drand48();

  for (int i = 0; i < DCOPIES*array_n; ++i) {
    a[i]      = (BASE_DTYPE)drand48();
    c_host[i] = a[i] * scalar;
    c_device[i] = 0.;
  }

  a_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  c_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  for (int i=0; i<num_sub_devices; i++) {
    a_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_READ_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    c_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_WRITE_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    clEnqueueWriteBuffer(commands[i], a_bufs[i], CL_TRUE, 0, chunk_bytes, a+(i*chunk_n), 0, NULL, NULL);
    clEnqueueWriteBuffer(commands[i], c_bufs[i], CL_TRUE, 0, chunk_bytes, c_device+(i*chunk_n), 0, NULL, NULL);
  }

  for (int i=0; i<num_sub_devices; i++) {
    kernels[i] = clCreateKernel(programs[i], "stream_scale" , &error);
    assert(error == CL_SUCCESS);

    clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &(a_bufs[i]));
    clSetKernelArg(kernels[i], 1, sizeof(cl_mem), &(c_bufs[i]));
    clSetKernelArg(kernels[i], 2, sizeof(BASE_DTYPE), &scalar);
  }
#elif defined (TRIAD_KERNEL)
  a = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);
  b = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);
  scalar = (BASE_DTYPE) drand48();

  for (int i = 0; i < DCOPIES*array_n; ++i) {
    a[i]      = (BASE_DTYPE)drand48();
    b[i]      = (BASE_DTYPE)drand48();
    c_host[i] = a[i] + b[i]*scalar;
    c_device[i] = 0.;
  }

  a_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  b_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  c_bufs = (cl_mem *) malloc(sizeof(cl_mem)*num_sub_devices);
  for (int i=0; i<num_sub_devices; i++) {
    a_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_READ_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    b_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_READ_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    c_bufs[i] = xCreateBuffer (contexts[i], sub_dev_ids[i], CL_MEM_WRITE_ONLY, chunk_bytes, NULL, &error);
    CheckError (error);

    clEnqueueWriteBuffer(commands[i], a_bufs[i], CL_TRUE, 0, chunk_bytes, a+(i*chunk_n), 0, NULL, NULL);
    clEnqueueWriteBuffer(commands[i], b_bufs[i], CL_TRUE, 0, chunk_bytes, b+(i*chunk_n), 0, NULL, NULL);
    clEnqueueWriteBuffer(commands[i], c_bufs[i], CL_TRUE, 0, chunk_bytes, c_device+(i*chunk_n), 0, NULL, NULL);
  }

  for (int i=0; i<num_sub_devices; i++) {
    kernels[i] = clCreateKernel(programs[i], "stream_triad" , &error);
    assert(error == CL_SUCCESS);

    clSetKernelArg(kernels[i], 0, sizeof(cl_mem), &(a_bufs[i]));
    clSetKernelArg(kernels[i], 1, sizeof(cl_mem), &(b_bufs[i]));
    clSetKernelArg(kernels[i], 2, sizeof(cl_mem), &(c_bufs[i]));
    clSetKernelArg(kernels[i], 3, sizeof(BASE_DTYPE), &scalar);
  }
#endif

  if (verbosity > RUN_AND_SYS) {
    printf("Finished kernel Setup\n");
  }

  double iter_start[ntimes], iter_elapsed[ntimes];

  for (int i=0; i<ntimes; i++) {
    iter_start[i] = get_wtime();
    for (int j=0; j<num_sub_devices; j++) {
      if (do_ocl_events) {
        clEnqueueNDRangeKernel(commands[j], kernels[j], 1, NULL, globalWorkSize, NULL, 0, NULL, &event[j][i]);
      } else {
        clEnqueueNDRangeKernel(commands[j], kernels[j], 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
      }
      clFlush(commands[j]);
    }
    for (int j=0; j<num_sub_devices; j++) {
      clFinish(commands[j]);
    }
    iter_elapsed[i] = get_wtime() - iter_start[i];
  }

  if (verbosity > RUN_AND_SYS) {
    printf ("Finished kernel execution %lld times\n", ntimes);
  }

  if (do_ocl_events) {
    printf ("OpenCL profiling with CL_PROFILING_COMMAND_{START/END} is active!\n");
    cl_ulong time_start[ntimes];
    cl_ulong time_end[ntimes];
    double nanoSeconds[ntimes];
    long long dev_bytes = bw_factor * chunk_bytes;

    for (int j=0; j<num_sub_devices; j++) {
      mintime = FLT_MAX;
      maxtime = FLT_MIN;
      totaltime = 0.0;

      if (verbosity > RUN_ONLY) {
        printf ("Memory transfer rate (MTR) for each kernel trial on sub-device-%d:\n",j);
      }

      for (int i=0; i<ntimes; i++) {
        clGetEventProfilingInfo(event[j][i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &time_start[i], NULL);
        clGetEventProfilingInfo(event[j][i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &time_end[i], NULL);

        nanoSeconds[i] = time_end[i]-time_start[i];

        if(verbosity > RUN_ONLY) {
          printf("\tsub-device[%d]-trial[%d] MTR is: %0.3f GB/s; Time(ns) is: %.2f\n", j, i,
                (1.0E-09*dev_bytes)/(nanoSeconds[i]*1.0E-09), nanoSeconds[i]);
        }

        // skip 1st trial for summary stats
        if (i > 0) {
          if(nanoSeconds[i] < mintime) {
            mintime = nanoSeconds[i];
          }
          if (nanoSeconds[i] > maxtime) {
            maxtime = nanoSeconds[i];
          }
          totaltime += nanoSeconds[i];
        }
      }

      if (verbosity > NO_OUTPUT) {
          printf("Performance summary (Memory Transfer Rate[MTR]) of sub-device-%d for %lld trials (excluding 1st trial):\n", j, ntimes);
          printf("\tWorst   MTR: %0.3f GB/s; Time(ns): %.2f\n", (1.0E-09*dev_bytes)/(maxtime*1.0E-09), maxtime);
          printf("\tAverage MTR: %0.3f GB/s; Time(ns): %.2f\n", (1.0E-09*dev_bytes)/((totaltime/(ntimes-1))*1.0E-09), totaltime/(ntimes-1));
          printf("\tBest    MTR: %0.3f GB/s; Time(ns): %.2f\n", (1.0E-09*dev_bytes)/(mintime*1.0E-09), mintime);
          printf("\nWarning: MTR does not represent actual observed bandwidth since it does not include command enqueue and submission costs\n");
      }
      printf ("========================================================\n");
    }
    printf("End OpenCL profiling stats\n");
  }

  if (verbosity > NO_OUTPUT) {
      mintime = FLT_MAX;
      maxtime = FLT_MIN;
      totaltime = 0.0;

      for (int i=0; i<ntimes; i++) {
        if (verbosity > RUN_ONLY) {
          printf("Trial[%d] Bandwidth: %.3f GB/s; Time(ns): %.2f\n", i, (1.E-09*total_bytes)/iter_elapsed[i], iter_elapsed[i]*1.E09);
        }

        // skip 1st trial for summary stats
        if (i > 0) {
          if (mintime > iter_elapsed[i]) {
            mintime = iter_elapsed[i];
          }
          if (iter_elapsed[i] > maxtime) {
            maxtime = iter_elapsed[i];
          }
          totaltime += iter_elapsed[i];
        }
      }

      printf("\nPerformance summary for %lld trials (excluding 1st trial):\n", ntimes);
      printf("\tWorst   Bandwidth: %.3f GB/s; Time(ns): %.2f\n", (1.E-09*total_bytes)/(maxtime), maxtime*1.E09);
      printf("\tAverage Bandwidth: %.3f GB/s; Time(ns): %.2f\n", (1.E-09*total_bytes)/(totaltime/(ntimes-1)), (totaltime/(ntimes-1))*1.E09);
      printf("\tBest    Bandwidth: %.3f GB/s; Time(ns): %.2f\n", (1.E-09*total_bytes)/(mintime), mintime*1.E09);
  }

  double l2_val;
  int num_fail = 0;
  for (int i=0; i<num_sub_devices; i++) {
    clEnqueueReadBuffer(commands[i], c_bufs[i], CL_TRUE, 0, chunk_bytes, c_device+(i*chunk_n), 0, NULL, NULL);

    l2_val = l2(c_device+(i*chunk_n), c_host+(i*chunk_n), chunk_n);

    if (l2_val > 1e-12) {
      num_fail++;
      if (verbosity > NO_OUTPUT) {
        printf("L2 norm for sub-device-%d is %le\n", i, l2_val);
      }
    }
  }

  if (num_fail) {
    printf ("Validation failed!\n");
  } else {
    printf ("Validation passed!\n");
  }

  for (int i=0; i<num_sub_devices; i++) {
    clReleaseKernel(kernels[i]);
    if (a_bufs != NULL) {
      clReleaseMemObject(a_bufs[i]);
    }
    if (b_bufs != NULL) {
      clReleaseMemObject(b_bufs[i]);
    }
    if (c_bufs != NULL) {
      clReleaseMemObject(c_bufs[i]);
    }
  }

  if (a != NULL) {
    _mm_free (a);
  }
  if (b != NULL) {
    _mm_free (b);
  }
  if (c_host != NULL) {
    _mm_free (c_host);
  }
  if (c_device != NULL) {
    _mm_free (c_device);
  }

  finalize_OCL_dev();

  return num_fail;
}
