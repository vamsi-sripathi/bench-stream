#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void stream_triad(const __global STREAM_TYPE *restrict x, const __global STREAM_TYPE *restrict y, __global STREAM_TYPE *restrict z, const STREAM_TYPE scalar)
{
	const int i = get_global_id(0);
	z[i] = x[i] + y[i]*scalar;
}

__kernel void stream_ro(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict sum)
{
	const int i = get_global_id(0);
  STREAM_BASE_TYPE s = 0.;

	s +=  x[i];

  if (s == 0.0) {
     //printf ("WARNING: Writing to buffer in RO kernel, this should not be happening!\n");
     *sum = s;
  }

#if 0
  const int lid = get_local_id(0);
  const int grp_id = get_group_id(0);
  const int grp_size = get_local_size(0);

  if (lid==0) {
    sum[grp_id] = s;
  }
#endif
}

__kernel void stream_wo(__global STREAM_TYPE *restrict x, const STREAM_BASE_TYPE val)
{
	const int i = get_global_id(0);

  x[i] = val;
}

__kernel void stream_scale(const __global STREAM_TYPE *restrict x, __global STREAM_TYPE *restrict y, const STREAM_TYPE scalar)
{
	const int i = get_global_id(0);

	y[i] = x[i]*scalar;
}

#define NUM_OFFSET_BYTES     (64)
#define NUM_BYTES_PER_THREAD (16)
#define NUM_BYTES_PER_ELE    (8)

__kernel void stream_wo_offset(__global STREAM_TYPE *restrict x, const STREAM_BASE_TYPE val)
{
  const int offset = get_global_id(0) * (NUM_OFFSET_BYTES/NUM_BYTES_PER_ELE);
  const int step   = ((NUM_BYTES_PER_THREAD > NUM_OFFSET_BYTES) ? NUM_OFFSET_BYTES/NUM_BYTES_PER_ELE : NUM_BYTES_PER_THREAD/NUM_BYTES_PER_ELE);

  for (unsigned int j=0; j<step; j++) {
    x[offset+j] = val;
  }
}

__kernel void stream_ro_offset(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict sum)
{
  STREAM_BASE_TYPE s = 0.;
  const int offset = get_global_id(0) * (NUM_OFFSET_BYTES/NUM_BYTES_PER_ELE);
  const int step   = ((NUM_BYTES_PER_THREAD > NUM_OFFSET_BYTES) ? NUM_OFFSET_BYTES/NUM_BYTES_PER_ELE : NUM_BYTES_PER_THREAD/NUM_BYTES_PER_ELE);

  for (unsigned int j=0; j<step; j++) {
    s += x[offset+j];
  }

  if (s == 0.0) {
     *sum = s;
  }
}

#define UNROLL 4
#define SUBGROUP_SIZE 16
#pragma OPENCL EXTENSION cl_intel_subgroups: enable

__attribute__((intel_reqd_sub_group_size(SUBGROUP_SIZE)))
__kernel void stream_ro_unroll2(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict sum)
{
    int i = ((get_global_id(0) / SUBGROUP_SIZE) * (SUBGROUP_SIZE * UNROLL)) + ((get_global_id(0) % SUBGROUP_SIZE) * UNROLL);
    STREAM_TYPE s = 0;
    /* printf ("\n tid = %d, i = %d", get_global_id(0), i); */

    for (unsigned int u = 0; u < UNROLL; u++)
    {
        s +=  x[i];
        i++;
    }

    if (s == 0.0) {
        *sum = s;
    }
}

__kernel void stream_ro_unroll(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict sum)
{
	  const int i = get_global_id(0)*16;
    STREAM_BASE_TYPE s = 0.;

    s +=  x[i+0];
    s +=  x[i+1];
    s +=  x[i+2];
    s +=  x[i+3];
    s +=  x[i+4];
    s +=  x[i+5];
    s +=  x[i+6];
    s +=  x[i+7];
    s +=  x[i+8];
    s +=  x[i+9];
    s +=  x[i+10];
    s +=  x[i+11];
    s +=  x[i+12];
    s +=  x[i+13];
    s +=  x[i+14];
    s +=  x[i+15];

    if (s == 0.0) {
       *sum = s;
    }
}

__kernel void stream_triad_unroll(const __global STREAM_TYPE *restrict x, const __global STREAM_TYPE *restrict y, __global STREAM_TYPE *restrict z, const STREAM_TYPE scalar)
{
	const int i = get_global_id(0)*16;

	z[i] = x[i] + y[i]*scalar;
	z[i+1] = x[i+1] + y[i+1]*scalar;
	z[i+2] = x[i+2] + y[i+2]*scalar;
	z[i+3] = x[i+3] + y[i+3]*scalar;
	z[i+4] = x[i+4] + y[i+4]*scalar;
	z[i+5] = x[i+5] + y[i+5]*scalar;
	z[i+6] = x[i+6] + y[i+6]*scalar;
	z[i+7] = x[i+7] + y[i+7]*scalar;
	z[i+8] = x[i+8] + y[i+8]*scalar;
	z[i+9] = x[i+9] + y[i+9]*scalar;
	z[i+10] = x[i+10] + y[i+10]*scalar;
	z[i+11] = x[i+11] + y[i+11]*scalar;
	z[i+12] = x[i+12] + y[i+12]*scalar;
	z[i+13] = x[i+13] + y[i+13]*scalar;
	z[i+14] = x[i+14] + y[i+14]*scalar;
	z[i+15] = x[i+15] + y[i+15]*scalar;
}

__kernel void stream_wo_unroll(__global STREAM_TYPE *restrict x, const STREAM_BASE_TYPE val)
{
	const int i = get_global_id(0)*16;

  x[i] = val;
  x[i+1] = val;
  x[i+2] = val;
  x[i+3] = val;
  x[i+4] = val;
  x[i+5] = val;
  x[i+6] = val;
  x[i+7] = val;
  x[i+8] = val;
  x[i+9] = val;
  x[i+10] = val;
  x[i+11] = val;
  x[i+12] = val;
  x[i+13] = val;
  x[i+14] = val;
  x[i+15] = val;
}

__kernel void stream_scale_unroll(const __global STREAM_TYPE *restrict x, __global STREAM_TYPE *restrict y, const STREAM_TYPE scalar)
{
	const int i = get_global_id(0)*16;

	y[i] = x[i]*scalar;
	y[i+1] = x[i+1]*scalar;
	y[i+2] = x[i+2]*scalar;
	y[i+3] = x[i+3]*scalar;
	y[i+4] = x[i+4]*scalar;
	y[i+5] = x[i+5]*scalar;
	y[i+6] = x[i+6]*scalar;
	y[i+7] = x[i+7]*scalar;
	y[i+8] = x[i+8]*scalar;
	y[i+9] = x[i+9]*scalar;
	y[i+10] = x[i+10]*scalar;
	y[i+11] = x[i+11]*scalar;
	y[i+12] = x[i+12]*scalar;
	y[i+13] = x[i+13]*scalar;
	y[i+14] = x[i+14]*scalar;
	y[i+15] = x[i+15]*scalar;
}
