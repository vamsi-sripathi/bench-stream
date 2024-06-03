#include "stream.h"
#include "omp.h"
#include "stdlib.h"

#define NUM_EXTRA_KERNELS (2)
#define ALIGNMENT (64)

extern STREAM_TYPE a[];

void stream_extensions()
{
  double times[NUM_EXTRA_KERNELS][NTIMES];
  double avgtime[NUM_EXTRA_KERNELS] = {0},
         maxtime[NUM_EXTRA_KERNELS] = {0},
         mintime[NUM_EXTRA_KERNELS] = {FLT_MAX,FLT_MAX};

  char	*label[]  = {"Fill:      ", "Reduce:    "};
  double	bytes[] = {
    1 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE,
    1 * sizeof(STREAM_TYPE) * STREAM_ARRAY_SIZE
  };


  STREAM_TYPE total_sum=0., filler_const;
  STREAM_TYPE *partial_sum = NULL;

  int num_counted_thrs=0, k;
  ssize_t	j;

#ifdef _OPENMP
#pragma omp parallel
#pragma omp atomic 
  num_counted_thrs++;
#endif

  /* partial_sum = (STREAM_TYPE *) _mm_malloc(sizeof(STREAM_TYPE)*num_counted_thrs, ALIGNMENT); */
 if (posix_memalign((void **)&partial_sum, ALIGNMENT, sizeof(STREAM_TYPE)*num_counted_thrs)) {
     printf ("posix_memalign failed!\n"); fflush(0);
     return;
   }

  for (j=0; j<num_counted_thrs; j++) {
    partial_sum[j] = 0.;
  }


  for (k=0, a[0]=0.; k<NTIMES; k++) {
#if 1
    filler_const = a[k] + k%5;
    times[0][k] = mysecond();
#pragma omp parallel for
    for (j=0; j<STREAM_ARRAY_SIZE; j++) {
      a[j] = filler_const;
    }
    times[0][k] = mysecond() - times[0][k];
#endif

#if 1
    times[1][k] = mysecond();
#pragma omp parallel
    {
      STREAM_TYPE tmp=0.;
#pragma omp for
      for (j=0; j<STREAM_ARRAY_SIZE; j++) {
        tmp += a[j];
      }
      partial_sum[omp_get_thread_num()] = tmp;
    }
    times[1][k] = mysecond() - times[1][k];

    total_sum = ((k>0) ? (long long)total_sum%k : 0); 
    for (j=0; j<num_counted_thrs; j++) {
      total_sum += partial_sum[j];
    }
#endif
#ifdef VERBOSE
  printf ("Trial-%d, FILL = %.2f, REDUCE = %.2f\n",
          k,
          (bytes[0] * 1.e-9)/times[0][k],
          (bytes[1] * 1.e-9)/times[1][k]);
  fflush(0);
#endif
}

  for (k=1; k<NTIMES; k++)
  {
    for (j=0; j<NUM_EXTRA_KERNELS; j++)
    {
      avgtime[j] = avgtime[j] + times[j][k];
      mintime[j] = MIN(mintime[j], times[j][k]);
      maxtime[j] = MAX(maxtime[j], times[j][k]);
    }
  }

  printf(HLINE);
  printf("STREAM Extra kernels..\n");
  printf(HLINE);

  printf("Function    Best Rate MB/s  Avg time     Min time     Max time\n");
  for (j=0; j<NUM_EXTRA_KERNELS; j++) {
    avgtime[j] = avgtime[j]/(double)(NTIMES-1);

    printf("%s%12.1f  %11.6f  %11.6f  %11.6f\n", label[j],
           1.0E-06 * bytes[j]/mintime[j],
           avgtime[j],
           mintime[j],
           maxtime[j]);
  }
  printf(HLINE);

  for (j=0; j<STREAM_ARRAY_SIZE; j++) {
    if (a[j] != filler_const) {
      printf ("validation failed for Fill kernel at index-%ld: Expected = %f, Observed = %f\n",
              j, filler_const, a[j]); 
    }
  }
  printf ("validation passed for Fill kernel\n");

  STREAM_TYPE tmp, exp_sum;
  for (k=0, tmp=0.; k<NTIMES; k++) {
    tmp += k%5;
    exp_sum = ((k>0) ? (long long)exp_sum%k : 0); 
    exp_sum += tmp * STREAM_ARRAY_SIZE;
  }
  
  double abs_diff = fabs(exp_sum - total_sum);
  // entries in the reduction kernel are all intergers, so the following should be fine..
  if (abs_diff != 0.) {
    printf ("validation failed for Reduce kernel: Expected = %f, Observed = %f, fabs = %e\n", exp_sum, total_sum, fabs(exp_sum - total_sum));
  } else {
    printf ("validation passed for Reduce kernel\n");
  }
  fflush(0);

  free(partial_sum);
}
