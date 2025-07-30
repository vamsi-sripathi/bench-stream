#pragma once

#include "CL/cl.h"

#define CL_QUEUE_TILE_ID_INTEL (0x10000)

/**************************
 * Internal only cl types *
 **************************/

typedef cl_bitfield  cl_mem_properties_intel;
typedef cl_mem_flags cl_mem_flags_intel;

/******************************
 * Internal only cl_mem_flags *
 ******************************/

#define CL_MEM_TILE_MAX (3)

// clang-format off
#define CL_MEM_TILE_ONLY_MEMORY_VISIBILITY (1 << 17)
#define CL_MEM_LOCALLY_UNCACHED_RESOURCE   (1 << 18)
#define CL_MEM_COMPRESSED_RESOURCE         (1 << 21)
#define CL_MEM_UNCOMPRESSED_RESOURCE       (1 << 22)

#define CL_MEM_FLAGS_INTEL                             0x10001
#define CL_MEM_TILE_ID_INTEL                           0x10002

#define CL_CONTEXT_FLAGS_INTEL                         0x10003
#define CL_CONTEXT_ALLOW_ONLY_MASKED_QUEUE_INTEL       0x10004
#define CL_CONTEXT_ALLOW_ONLY_SINGLE_TILE_QUEUES_INTEL 0x10005
#define CL_CONTEXT_ALLOW_ALL_KIND_OF_QUEUES_INTEL      0x10006
// clang-format on

/*********************
 * embargo API calls *
 *********************/

#define clCreateBufferWithPropertiesINTELFunctionName "clCreateBufferWithPropertiesINTEL"

typedef CL_API_ENTRY cl_mem(CL_API_CALL *INTELpfn_clCreateBufferWithPropertiesINTEL)(
    cl_context context,
    const cl_mem_properties_intel *properties,
    cl_mem_flags flags,
    size_t size,
    void *hostPtr,
    cl_int *errcodeRet) CL_API_SUFFIX__VERSION_2_1;
