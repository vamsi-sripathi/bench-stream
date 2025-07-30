#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void stream_ro(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict y)
{
	const int i = get_global_id(0);
  STREAM_BASE_TYPE s = 0.;

	s +=  x[i];

  if (s == 0.0) {
     *y = s;
  }

#if 0
  const int lid = get_local_id(0);
  const int grp_id = get_group_id(0);
  const int grp_size = get_local_size(0);

  if (lid==0) {
    y[grp_id] = s;
  }
#endif
}

__kernel void stream_ro_unroll(const __global STREAM_TYPE *restrict x,  __global STREAM_TYPE *restrict y)
{
	const int i = get_global_id(0)*64;
  STREAM_BASE_TYPE s = 0.;
  unsigned int j;

  for (j=0; j<64; j++) {
	  s +=  x[i+j];
  }

  if (s == 0.0) {
     *y = s;
  }
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

__kernel void stream_triad(const __global STREAM_TYPE *restrict x, const __global STREAM_TYPE *restrict y, __global STREAM_TYPE *restrict z, const STREAM_TYPE scalar)
{
	const int i = get_global_id(0);
	z[i] = x[i] + y[i]*scalar;
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
