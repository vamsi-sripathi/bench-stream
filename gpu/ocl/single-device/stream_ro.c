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

static cl_mem xCreateBuffer(cl_context context, cl_mem_flags flags, size_t size, void *hostPtr, cl_int *errcodeRet)
{
#ifndef CACHEABLE
#error "Need CACHEABLE set to 0 or 1"
#endif

#if CACHEABLE == 0
  printf ("INFO: creating buffer using clCreateBuffer with CL_MEM_LOCALLY_UNCACHED_RESOURCE property\n"); fflush(0);
  INTELpfn_clCreateBufferWithPropertiesINTEL clCreateBufferWithPropertiesINTEL = (INTELpfn_clCreateBufferWithPropertiesINTEL) clGetExtensionFunctionAddressForPlatform(platform, clCreateBufferWithPropertiesINTELFunctionName);
  if(clCreateBufferWithPropertiesINTEL == NULL)
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "Can't find clCreateBufferWithPropertiesINTEL for this platform, kthxbye.\n");
    }
    exit(EXIT_FAILURE);
  }
  cl_mem_properties_intel props[] = {CL_MEM_FLAGS, flags, CL_MEM_FLAGS_INTEL, CL_MEM_LOCALLY_UNCACHED_RESOURCE, 0};
  return clCreateBufferWithPropertiesINTEL(context, props, 0, size, hostPtr, errcodeRet);
#elif CACHEABLE == 1
  return clCreateBuffer(context, flags, size, hostPtr, errcodeRet);
#else
#error "Need CACHEABLE set to 0 or 1"
#endif
}

static const char *usage_str = "USAGE:\t%s [at least one of -b <num> or -n <num>] [-t] [-q*] [-v*] [-h] [-V]\n";

static void usage(char *name)
{
  fprintf(stderr, usage_str, basename(name));
  exit(EXIT_FAILURE);
}

static void help(char *name)
{
  fprintf(stderr, usage_str, name);
  fprintf(stderr, "DESCRIPTION\n"
          "\t Run OpenCL STREAM Read-only kernel.\n\t DType is %s, cachable is %d\n", KERNEL_STREAM_DTYPE, CACHEABLE
         );
  fprintf(stderr, "OPTIONS\n"
          "\t-b,--bytes\n\t    run with given # of bytes\n"
          "\t-n,--num-elements\n\t    run with given # of elements\n"
          "\t-t,--times <n>\n\t    repeat benchmark n times (default is 10)\n"
          "\t-o,--ocl-profiling <flag>\n\t    use OCL events (don't use with AUB collection!)\n"
          "\t-y,--dump-ref <file>\n\t    dump host output buffer to <file>\n"
          "\t-v,--verbose\n\t    increase verbosity\n"
          "\t-q,--quiet\n\t    decrease verbosity\n"
          "\t-h,--help\n\t    print this help message\n"
          "\t-V,--version\n\t    print configuration information\n"
         );
}

static void print_version(FILE *fp)
{
  fprintf(fp, "%s\n", GIT_VERSION);
}

int main(int argc, char *argv[]) {
  int do_ocl_events = 1;
  long long ntimes = 10;
  long long array_n = -1;
  long long array_bytes = -1;
  char *dump_file = NULL;
  const struct option opts[] =
  {
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {"quiet",           no_argument,       0, 'q'},
    {"verbose",         no_argument,       0, 'v'},
    {"times",           required_argument, 0, 't'},
    {"num-elements",    required_argument, 0, 'n'},
    {"bytes",           required_argument, 0, 'b'},
    {"dump-ref",        required_argument, 0, 'y'},
    {"ocl-profiling",   required_argument, 0, 'o'},
    {0, 0, 0, 0},
  };

  int opt;
  do
  {
    opt = getopt_long(argc, argv, "hVqvt:n:b:y:o:", opts, 0);
    switch(opt)
    {
      case 0:
        break;
      case 'o':
        do_ocl_events = atoll(optarg);
        break;
      case 'y':
        dump_file = strdup(optarg);
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
      case 'q':
        --verbosity;
        break;
      case 'v':
        ++verbosity;
        break;
      case 'h':
        help(argv[0]);
        exit(EXIT_FAILURE);
      case 'V':
        print_version(stdout);
        exit(EXIT_FAILURE);
      default:
        usage(argv[0]);
      case -1:
        break;
    };
  }
  while (opt != -1);

  if (optind != argc)
  {
    usage(argv[0]);
  }

  if(ntimes < 0)
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "Need times > 0! (got %lld.)", ntimes);
    }
    exit(EXIT_FAILURE);
  }

  if(array_n != -1)
  {
    long long need_size = array_n * sizeof(BASE_DTYPE)*DCOPIES;
    if(array_bytes != -1 && array_bytes != need_size)
    {
      if(verbosity > NO_OUTPUT)
      {
        fprintf(stderr, "if both num-elements and bytes are specified, they need to match up! (num-elements is %lld, bytes is %lld, but computed %lld).", array_n, array_bytes, need_size);
      }
      exit(EXIT_FAILURE);
    }
    array_bytes = need_size;
  }
  else if(array_bytes != -1)
  {
    array_n = (array_bytes + sizeof(BASE_DTYPE)*DCOPIES -1) / (sizeof(BASE_DTYPE)*DCOPIES);
    array_bytes = array_n * sizeof(BASE_DTYPE)*DCOPIES;
  }
  else
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "need at least one of num-elements and bytes specified!\n");
    }
    exit(EXIT_FAILURE);
  }

  setup_OCL_dev();

  // Prepare some test data
  if(verbosity > NO_OUTPUT)
  {
    printf("ntimes = %lld\n", ntimes);
  }
  char *array_bytes_str = human_format(array_bytes);
  if(verbosity > NO_OUTPUT)
  {
    printf ("The value of array_n is %lld and footprint is %s\n",array_n,array_bytes_str);
  }
  free(array_bytes_str);
  assert(sizeof(C_STREAM_DTYPE) == DCOPIES * sizeof(BASE_DTYPE));

  BASE_DTYPE sum_host = 0.;
  BASE_DTYPE sum_device = 0.;

  const double bytes = (double)1 * array_bytes;
  double mintime = FLT_MAX;
  double maxtime = FLT_MIN;
  double totaltime = 0.0;

  BASE_DTYPE *a = (BASE_DTYPE *)_mm_malloc(array_bytes, 64);
  BASE_DTYPE *psum = (BASE_DTYPE *)_mm_malloc(sizeof(BASE_DTYPE)*(array_n), 64);

  srand48(42);
  for (int i = 0; i < DCOPIES*array_n; ++i) {
    /* a[i]      = rand()/((double) RAND_MAX - 0.5); */
    a[i]      = (BASE_DTYPE)drand48();
    psum[i]   = 1.0;
    /* printf ("[%d] = %.2f\n", i, a[i]); fflush(0); */
    sum_host += a[i];
  }

  if(dump_file)
  {
    FILE *fp = fopen(dump_file, "wb");
    if(!fp)
    {
      if(verbosity > NO_OUTPUT)
      {
        fprintf(stderr, "Can't open %s for dumping reference.\n", dump_file);
      }
      exit(EXIT_FAILURE);
    }
    fwrite(&sum_host, sizeof(BASE_DTYPE), 1, fp);
    fclose(fp);
    if(verbosity > NO_OUTPUT)
    {
      printf("Dumped reference file to %s\n", dump_file);
    }
    free(dump_file);
  }

  if(verbosity > RUN_AND_SYS)
  {
    printf("Finished Initialization\n");
  }
  //MEMORY ALLOCATION FOR THE BUFFERS
  cl_mem aBuffer = xCreateBuffer (context, CL_MEM_READ_ONLY, array_bytes, NULL, &error);
  CheckError (error);

  cl_mem psumBuffer = xCreateBuffer (context, CL_MEM_WRITE_ONLY, sizeof(BASE_DTYPE)*(array_n), NULL, &error);
  CheckError (error);

  if(verbosity > RUN_AND_SYS)
  {
    printf("Finished Memory Allocation\n");
  }

  clEnqueueWriteBuffer(commands, aBuffer, CL_TRUE, 0, array_bytes, a, 0, NULL, NULL);
  clEnqueueWriteBuffer(commands, psumBuffer, CL_TRUE, 0, sizeof(BASE_DTYPE)*(array_n), psum, 0, NULL, NULL);

  //Converting parallel loop in NDRange (loop indices converted to NDRange dimensions).
  const size_t globalWorkSize [] = { array_n, 0, 0 };

  //Creating kernel (kernel function code generated in .cl file from parallel loop's body).
  kernel_cl1 = clCreateKernel(program, "stream_ro" , &error);
  assert(error == CL_SUCCESS);

  //Setting kernel arguments (based on parallel step's input, output grids).
  clSetKernelArg(kernel_cl1, 0, sizeof(cl_mem), &aBuffer);
  clSetKernelArg(kernel_cl1, 1, sizeof(cl_mem), &psumBuffer);
  /* clSetKernelArg(kernel_cl1, 1, sizeof(sum_device), &sum_device); */

  if(verbosity > RUN_AND_SYS)
  {
    printf("Finished kernel Setup\n");
  }
  //Enqueuing OpenCL kernel for execution.
  cl_event event[ntimes];
  double cpu_start = get_wtime();
  for (int i=0; i<ntimes; i++)
  {
    if(do_ocl_events)
    {
      clEnqueueNDRangeKernel(commands, kernel_cl1, 1, NULL, globalWorkSize, NULL, 0, NULL, &event[i]);
    }
    else
    {
      clEnqueueNDRangeKernel(commands, kernel_cl1, 1, NULL, globalWorkSize, NULL, 0, NULL, NULL);
    }
  }


  clFinish(commands);
  double cpu_end = get_wtime();
  if(verbosity > RUN_AND_SYS)
  {
    printf("Finished kernel execution %lld times\n",ntimes);
  }
  if(verbosity > NO_OUTPUT)
  {
    printf("Total host-side wall time for %lld executions: %le s\n",ntimes, cpu_end-cpu_start);
    printf("Average host-side wall time: %le s\n", (cpu_end-cpu_start)/ntimes);
    printf("OpenCl Read-Only Bytes is: %0.3f\n",bytes);
  }

  //TIMING KERNEL
  if(do_ocl_events)
  {
    cl_ulong time_start[ntimes];
    cl_ulong time_end[ntimes];
    double nanoSeconds[ntimes];

    if(verbosity > RUN_ONLY) {
      printf ("Bandwidth for each test trial/iteration:\n");
    }
    for (int i=0; i<ntimes; i++)
    {
      clGetEventProfilingInfo(event[i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start[i], NULL);
      clGetEventProfilingInfo(event[i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end[i], NULL);

      nanoSeconds[i] = time_end[i]-time_start[i];
      if(verbosity > RUN_ONLY) {
        printf("\ttrial[%d] Bandwidth is: %0.3f GB/s; Time(ns) is: %.2f\n", i, (1.0E-09* bytes/(nanoSeconds[i]*1.0E-09)), nanoSeconds[i]);
      }

      if(nanoSeconds[i] < mintime)
      {
        mintime = nanoSeconds[i];
      }
      if (nanoSeconds[i] > maxtime) {
        maxtime = nanoSeconds[i];
      }
      totaltime += nanoSeconds[i];
    }
  }

  if(verbosity > NO_OUTPUT)
  {
    if(do_ocl_events)
    {
      printf("Performance summary for %lld trials:\n", ntimes);
      /* printf("Best-case: OpenCl Read-Only Execution time (MIN) is: %0.3f micro-seconds \n",mintime * 1.0E-3); */
      printf("Best:    OpenCl Read-Only Bandwidth is: %0.3f GB/s; time(ns) is: %.2f\n",(1.0E-09* bytes/(mintime*1.0E-09)), mintime);
      printf("Average: OpenCl Read-Only Bandwidth is: %0.3f GB/s; time(ns) is: %.2f\n",(1.0E-09* bytes/((totaltime/ntimes)*1.0E-09)), totaltime/ntimes);
      printf("Worst:   OpenCl Read-Only Bandwidth is: %0.3f GB/s; time(ns) is: %.2f\n",(1.0E-09* bytes/(maxtime*1.0E-09)), maxtime);
    }
  }

  clEnqueueReadBuffer(commands, psumBuffer, CL_TRUE, 0, sizeof(BASE_DTYPE)*(array_n), psum, 0, NULL, NULL);


  int res;
  /* printf ("sum_device = %.2f\n", psum[0]); */
  /* printf ("sum_host = %.2f\n", sum_host); */
#if 0
  if(verbosity > NO_OUTPUT)
  {
    printf("L2 norm is %le\n", l2_val);
  }


  if(l2_val > 1e-12)
  {
    if(verbosity > NO_OUTPUT)
    {
      printf("Failed!\n");
    }
    res = EXIT_FAILURE;
  }
  else
  {
    if(verbosity > NO_OUTPUT)
    {
      printf("Success!\n");
    }
    res = EXIT_SUCCESS;
  }
#else
  res = EXIT_SUCCESS;
#endif

  //Freeing up memory buffers.
  clReleaseMemObject(aBuffer);
  clReleaseMemObject(psumBuffer);
  clReleaseKernel (kernel_cl1);
  finalize_OCL_dev();
  return res;
}
