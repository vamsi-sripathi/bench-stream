// OpenCL boilerplate code.
// Konstantinos Krommydas (konstantinos.krommyda@intel.com)
// 06/2018
//
// Parts of the code below for looping through the available OpenCL platforms
// and devices to discover available GPUs are derived from: 
// https://gist.github.com/courtneyfaulkner/7919509


#include <CL/cl.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

enum { NO_OUTPUT = -2, RUN_ONLY = 0, RUN_AND_SYS = 1, EFFUSIVE_SYS=2, EVERYTHING=3};
extern int verbosity;

#define xstr(s) str(s)
#define str(s) #s

#if defined(TYPE_FLOAT4)
#define KERNEL_STREAM_DTYPE "float4"
#define BASE_DTYPE float
#define C_STREAM_DTYPE cl_float4
#define DCOPIES 4
#elif defined(TYPE_FLOAT)
#define KERNEL_STREAM_DTYPE "float"
#define BASE_DTYPE float
#define C_STREAM_DTYPE cl_float
#define DCOPIES 1
#elif defined(TYPE_DOUBLE)
#define KERNEL_STREAM_DTYPE "double"
#define BASE_DTYPE double
#define C_STREAM_DTYPE cl_double
#define DCOPIES 1
#elif defined(TYPE_INT)
#define KERNEL_STREAM_DTYPE "int"
#define BASE_DTYPE int
#define C_STREAM_DTYPE cl_int
#define DCOPIES 1
#else
#error "Need one of FLOAT4,FLOAT,DOUBLE,INT defined to build!"
#endif

char *get_exe()
{
  static const int BUFFSIZE = 2048;
  char buffer[BUFFSIZE];
  memset(buffer, 0, sizeof(char)*BUFFSIZE);
  ssize_t read = readlink("/proc/self/exe", buffer, BUFFSIZE);
  if(read == BUFFSIZE)
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "Executable path is too long (>= %d) for readlink buffer. What is wrong with you?\n", BUFFSIZE);
    }
    exit(EXIT_FAILURE);
  }
  if(read == -1)
  {
    if(verbosity > NO_OUTPUT)
    {
      perror("readlink");
    }
    exit(EXIT_FAILURE);
  }
  return strdup(buffer);
}

// declare the standard OCL objects we need to create and OCL program
//
cl_platform_id *platforms;
cl_platform_id platform;
cl_device_id dev_id;
cl_context context;
cl_command_queue commands;
cl_kernel kernel_cl1;
cl_program program;
cl_int error;

// File where kerenls are found
//
const char *OclFileName = "ocl_kernels.cl";

// --------------------------------------------------------------------------
// Boilerplate function to setup and OCL program for the 1st GPU device
// in the platform, and read and compile the OCL code
// --------------------------------------------------------------------------
void setup_OCL_dev() {

  int i, j;
  char *info;
  size_t infoSize, valueSize;
  char *value;
  cl_uint maxComputeUnits;
  const char* attributeNames[5] = {"Name", "Vendor", "Version", "Profile", 
    "Extensions"};
  const cl_platform_info attributeTypes[5] = {CL_PLATFORM_NAME, CL_PLATFORM_VENDOR,
    CL_PLATFORM_VERSION, CL_PLATFORM_PROFILE, CL_PLATFORM_EXTENSIONS};
  const int attributeCount = sizeof(attributeNames)/sizeof(char*);


  // Get platform ID
  //
  cl_uint num_platforms = 0;

  // Get the number of available OpenCL Platforms.
  error = clGetPlatformIDs(0, NULL, &num_platforms);
  assert(error == CL_SUCCESS);

  // Allocate size for number of available OpenCL Platforms.
  platforms = (cl_platform_id *)malloc(num_platforms * sizeof(cl_platform_id));
  // Query the details of all platforms and save to "platforms[]".
  error = clGetPlatformIDs(num_platforms, platforms, NULL);
  assert(error == CL_SUCCESS);


  for (i = 0; i < num_platforms; i++) {

    if(verbosity > RUN_AND_SYS)
    {
      printf("\nAttempting to find available GPU devices in Platform: %d \n", i+1);
    }

    for (j = 0; j < attributeCount; j++) {

      // Get size of attribute value.
      clGetPlatformInfo(platforms[i], attributeTypes[j], 0, NULL, &infoSize);
      info = (char*)malloc(infoSize);

      // Get the attribute value itself.
      clGetPlatformInfo(platforms[i], attributeTypes[j], infoSize, info, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf("  %d.%d %-11s: %s\n", i+1, j+1, attributeNames[j], info);
      }
      free(info);

    }

    // Get all available GPU devices of current platform.
    cl_uint num_devices = 0;
    clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices);
    if(verbosity > RUN_AND_SYS)
    {
      printf("\nFound %d GPU devices in Platform %d\n", num_devices, i+1);
    }

    if (num_devices > 0) {

      error = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_devices, &dev_id, NULL);
      assert(error == CL_SUCCESS);

      // Print device name.
      clGetDeviceInfo(dev_id, CL_DEVICE_NAME, 0, NULL, &valueSize);
      value = (char*) malloc(valueSize);
      clGetDeviceInfo(dev_id, CL_DEVICE_NAME, valueSize, value, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf("Device: %s\n", value);
      }
      free(value);

      // Print hardware device version.
      clGetDeviceInfo(dev_id, CL_DEVICE_VERSION, 0, NULL, &valueSize);
      value = (char*) malloc(valueSize);
      clGetDeviceInfo(dev_id, CL_DEVICE_VERSION, valueSize, value, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf(" Hardware version: %s\n", value);
      }
      free(value);

      // Print software driver version.
      clGetDeviceInfo(dev_id, CL_DRIVER_VERSION, 0, NULL, &valueSize);
      value = (char*) malloc(valueSize);
      clGetDeviceInfo(dev_id, CL_DRIVER_VERSION, valueSize, value, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf(" Software version: %s\n", value);
      }
      free(value);

      // Print C version supported by compiler for device.
      clGetDeviceInfo(dev_id, CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
      value = (char*) malloc(valueSize);
      clGetDeviceInfo(dev_id, CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf(" OpenCL C version: %s\n", value);
      }
      free(value);

      // Print compute units.
      clGetDeviceInfo(dev_id, CL_DEVICE_MAX_COMPUTE_UNITS,
                      sizeof(maxComputeUnits), &maxComputeUnits, NULL);
      if(verbosity > RUN_AND_SYS)
      {
        printf(" Parallel compute units: %d\n", maxComputeUnits);
        printf("\nUsing this device...\n\n\n");
      }
      platform = platforms[i];

      break;
    }

    if(verbosity > RUN_AND_SYS)
    {
      printf("\n");
    }
  }
  // Create OCL context, which unifies the device and command/data
  // queues/buffers, which will be created next.
  context = clCreateContext(NULL, 1, &dev_id, NULL, NULL, &error);
  assert(error == CL_SUCCESS);

  const cl_queue_properties queue_props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
  commands = clCreateCommandQueueWithProperties(context, dev_id, queue_props, &error);
  assert(error == CL_SUCCESS);

  char *exe_path = get_exe();
  char *exe_dir = dirname(exe_path);
  char temp[2048];
  int num_char = snprintf(temp, 2047, "%s/%s", exe_dir, OclFileName);
  free(exe_path);
  if(num_char > 2047)
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "path to kernel (%s/%s) is too long!\n", exe_dir, OclFileName);
    }
    exit(EXIT_FAILURE);
  }

  // Read the source from OclFileName into a string (kernel_source)
  FILE* kernel_fp = fopen(temp, "r");
  if(!kernel_fp)
  {
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "can't find kernel source %s\n", temp);
    }
    exit(EXIT_FAILURE);
  }
  fseek(kernel_fp, 0, SEEK_END);
  size_t kernel_size = (size_t)ftell(kernel_fp);
  rewind(kernel_fp);
  char* kernel_source = (char *)malloc(sizeof(char)*kernel_size);
  fread((void* )kernel_source, kernel_size, 1, kernel_fp);
  fclose(kernel_fp);

  // Create an OCL program with the source string.
  program = clCreateProgramWithSource(context, 1, 
                                      (const char **)&kernel_source, 
                                      &kernel_size, &error);
  assert(error == CL_SUCCESS);

  // Compile the OCL program (to intermediate representation) and 
  // write errors to a log file.
  snprintf(temp, 2047, "-DSTREAM_TYPE=%s -DSTREAM_BASE_TYPE=%s", KERNEL_STREAM_DTYPE, xstr(BASE_DTYPE));
  error = clBuildProgram(program, 1, &dev_id, temp, NULL, NULL);

  // If there were compilation errors in the .cl file print them here.
  if (error == CL_BUILD_PROGRAM_FAILURE) {
    char *logTxt;
    size_t logSize;
    clGetProgramBuildInfo(program, dev_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    logTxt = (char* )malloc(sizeof(char)*logSize);
    clGetProgramBuildInfo(program, dev_id, CL_PROGRAM_BUILD_LOG, logSize, (void* )logTxt, NULL);
    if(verbosity > NO_OUTPUT)
    {
      fprintf(stderr, "Build Error Log:\n%s", logTxt);
    }
    exit(EXIT_FAILURE);
  }
  assert(error == CL_SUCCESS);

  free(platforms);

}

// --------------------------------------------------------------------------
// Boiler plate code to release OCL resources
// --------------------------------------------------------------------------
void finalize_OCL_dev() {

  clReleaseProgram(program);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);
}
