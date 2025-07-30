#pragma once

#include "CL/cl.h"

/**************************
 * Internal only cl types *
 **************************/

// using cl_mem_properties_intel = cl_bitfield;
// using cl_mem_flags_intel = cl_mem_flags;

/******************************
 * Internal only cl_mem_flags *
 ******************************/

#define CL_MEM_TILE_MAX (3)

// clang-format off
#define CL_MEM_TILE_ONLY_MEMORY_VISIBILITY   (1 << 17)
#define CL_MEM_LOCALLY_UNCACHED_RESOURCE     (1 << 18)
#define CL_MEM_COMPRESSED_HINT_INTEL         (1 << 21)
#define CL_MEM_UNCOMPRESSED_HINT_INTEL       (1 << 22)
#define CL_MEM_ALLOW_UNRESTRICTED_SIZE_INTEL (1 << 23)

#define CL_MEM_FLAGS_INTEL                             0x10001
#define CL_MEM_TILE_ID_INTEL                           0x10002
#define CL_MEM_DEVICE_ID_INTEL                         0x10011

// cl_context_properties
#define CL_CONTEXT_FLAGS_INTEL                         0x10003
#define CL_CONTEXT_ALLOW_ONLY_MASKED_QUEUE_INTEL       0x10004
#define CL_CONTEXT_ALLOW_ONLY_SINGLE_TILE_QUEUES_INTEL 0x10005
#define CL_CONTEXT_ALLOW_ALL_KIND_OF_QUEUES_INTEL      0x10006

// cl_queue_properties
#define CL_QUEUE_TILE_ID_INTEL                         0x10000
#define CL_QUEUE_TILE_ID_MASK_INTEL                    0x10007
#define CL_QUEUE_FAMILY_INTEL                          0x10008

// cl_queue_properties values
#define CL_QUEUE_FAMILY_TYPE_RCS_INTEL                 0x0
#define CL_QUEUE_FAMILY_TYPE_CCS0_INTEL                0x1
#define CL_QUEUE_FAMILY_TYPE_CCS1_INTEL                0x2
#define CL_QUEUE_FAMILY_TYPE_CCS2_INTEL                0x3
#define CL_QUEUE_FAMILY_TYPE_CCS3_INTEL                0x4
#define CL_QUEUE_FAMILY_TYPE_BCS_INTEL                 0x5

// Used with clEnqueueVerifyMemory
#define CL_MEM_COMPARE_EQUAL                           0u
#define CL_MEM_COMPARE_NOT_EQUAL                       1u

// cl_kernel_info
#define CL_KERNEL_SUPPORT_COMPRESSION                  0x1000A

/*******************************************
* cl_intel_unified_shared_memory extension *
********************************************/

// These enums are in sync with revision H of the USM spec.

/* cl_device_info */
#define CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL                   0x4190
#define CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL                 0x4191
#define CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL   0x4192
#define CL_DEVICE_CROSS_DEVICE_SHARED_MEM_CAPABILITIES_INTEL    0x4193
#define CL_DEVICE_SHARED_SYSTEM_MEM_CAPABILITIES_INTEL          0x4194

typedef cl_bitfield cl_unified_shared_memory_capabilities_intel;

/* cl_unified_shared_memory_capabilities_intel - bitfield */
#define CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL                   (1 << 0)
#define CL_UNIFIED_SHARED_MEMORY_ATOMIC_ACCESS_INTEL            (1 << 1)
#define CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ACCESS_INTEL        (1 << 2)
#define CL_UNIFIED_SHARED_MEMORY_CONCURRENT_ATOMIC_ACCESS_INTEL (1 << 3)

typedef cl_bitfield cl_mem_properties_intel;

/* cl_mem_properties_intel */
#define CL_MEM_ALLOC_FLAGS_INTEL        0x4195

typedef cl_bitfield cl_mem_alloc_flags_intel;

/* cl_mem_alloc_flags_intel - bitfield */
#define CL_MEM_ALLOC_DEFAULT_INTEL                      0
#define CL_MEM_ALLOC_WRITE_COMBINED_INTEL               (1 << 0)

typedef cl_uint cl_mem_info_intel;

/* cl_mem_alloc_info_intel */
#define CL_MEM_ALLOC_TYPE_INTEL         0x419A
#define CL_MEM_ALLOC_BASE_PTR_INTEL     0x419B
#define CL_MEM_ALLOC_SIZE_INTEL         0x419C
#define CL_MEM_ALLOC_DEVICE_INTEL       0x419D
#define CL_MEM_ALLOC_INFO_TBD0_INTEL    0x419E
#define CL_MEM_ALLOC_INFO_TBD1_INTEL    0x419F

typedef cl_uint cl_unified_shared_memory_type_intel;

/* cl_unified_shared_memory_type_intel */
#define CL_MEM_TYPE_UNKNOWN_INTEL       0x4196
#define CL_MEM_TYPE_HOST_INTEL          0x4197
#define CL_MEM_TYPE_DEVICE_INTEL        0x4198
#define CL_MEM_TYPE_SHARED_INTEL        0x4199

typedef cl_uint cl_mem_advice_intel;

/* cl_mem_advice_intel */
#define CL_MEM_ADVICE_TBD0_INTEL        0x4208
#define CL_MEM_ADVICE_TBD1_INTEL        0x4209
#define CL_MEM_ADVICE_TBD2_INTEL        0x420A
#define CL_MEM_ADVICE_TBD3_INTEL        0x420B
#define CL_MEM_ADVICE_TBD4_INTEL        0x420C
#define CL_MEM_ADVICE_TBD5_INTEL        0x420D
#define CL_MEM_ADVICE_TBD6_INTEL        0x420E
#define CL_MEM_ADVICE_TBD7_INTEL        0x420F

/* cl_kernel_exec_info */
#define CL_KERNEL_EXEC_INFO_INDIRECT_HOST_ACCESS_INTEL      0x4200
#define CL_KERNEL_EXEC_INFO_INDIRECT_DEVICE_ACCESS_INTEL    0x4201
#define CL_KERNEL_EXEC_INFO_INDIRECT_SHARED_ACCESS_INTEL    0x4202
#define CL_KERNEL_EXEC_INFO_USM_PTRS_INTEL                  0x4203

/* cl_command_type */
#define CL_COMMAND_MEMFILL_INTEL        0x4204
#define CL_COMMAND_MEMCPY_INTEL         0x4205
#define CL_COMMAND_MIGRATEMEM_INTEL     0x4206
#define CL_COMMAND_MEMADVISE_INTEL      0x4207

/*********************************************
 * Internal only kernel exec info properties *
 *********************************************/

// using cl_execution_info_kernel_type_intel = cl_uint;
#define CL_KERNEL_EXEC_INFO_KERNEL_TYPE_INTEL     0x1000C
#define CL_KERNEL_EXEC_INFO_DEFAULT_TYPE_INTEL    0x1000D
#define CL_KERNEL_EXEC_INFO_CONCURRENT_TYPE_INTEL 0x1000E

/************************************************
 * cl_enqueue_resources_barrier_intel extension *
 ************************************************/

#define CL_COMMAND_RESOURCE_BARRIER                    0x10010

typedef cl_uint cl_resource_barrier_type;
#define CL_RESOURCE_BARRIER_TYPE_ACQUIRE               0x1 // FLUSH+EVICT
#define CL_RESOURCE_BARRIER_TYPE_RELEASE               0x2 // FLUSH
#define CL_RESOURCE_BARRIER_TYPE_DISCARD               0x3 // DISCARD

typedef cl_uint cl_resource_memory_scope;
#define CL_MEMORY_SCOPE_DEVICE                         0x0 // INCLUDES CROSS-TILE
#define CL_MEMORY_SCOPE_ALL_SVM_DEVICES                0x1 // CL_MEMORY_SCOPE_DEVICE + CROSS-DEVICE
// clang-format on

typedef struct _cl_resource_barrier_descriptor_intel {
    void* svm_allocation_pointer;
    cl_mem mem_object;
    cl_resource_barrier_type type;
    cl_resource_memory_scope scope;
} cl_resource_barrier_descriptor_intel;

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

