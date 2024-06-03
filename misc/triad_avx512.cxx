#include "immintrin.h"

#define UNROLL          (8)
#define NUM_ELES_IN_4K  (512)

#if !defined (RFO_STORES) && !defined (NT_STORES)
#error "store type definition is missing!
#endif

void triad_avx512_pf(ssize_t *p_n, double *p_scalar, double *p_c, double *p_b, double *p_a)
{
  __m512d zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7;

  int p, prolog = 0;
  ssize_t n = *p_n;

  // thread chunking is done to make sure n is CL aligned
#if 0
  if ((unsigned long long)p_a%64) {
    prolog = (64 - (unsigned long long)p_a%64)/sizeof(double);
    p = prolog;
    while (p) {
      *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
      p--;
    }
  }
#endif

#if 0
#ifdef DEBUG
  printf("%dB: fwd_ld1_align_4k = %ld, fwd_ld2_align_4k = %ld, fwd_st_align_4k = %ld\n",
         omp_get_thread_num(), (4096-((unsigned long long)p_b)%4096)/8, (4096-((unsigned long long)p_c)%4096)/8, (4096-((unsigned long long)p_a)%4096)/8);
  fflush(0);
#endif
  prolog = omp_get_thread_num()*32;
  p = prolog;
  while (p) {
    *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
    p--;
  }
#ifdef DEBUG
  printf("%dA: fwd_ld1_align_4k = %ld, fwd_ld2_align_4k = %ld, fwd_st_align_4k = %ld\n",
         omp_get_thread_num(), (4096-((unsigned long long)p_b)%4096)/8, (4096-((unsigned long long)p_c)%4096)/8, (4096-((unsigned long long)p_a)%4096)/8);
  fflush(0);
#endif
#endif


  ssize_t n_blk  = ((n-prolog)/UNROLL)*UNROLL;
  ssize_t n_tail = n - n_blk - prolog;

  zmm0 = _mm512_broadcastsd_pd(_mm_load_sd(p_scalar));

  while (n_blk) {
    zmm1 = _mm512_loadu_pd(p_b);
    zmm1 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c), zmm0, zmm1);
#if defined (NT_STORES)
    _mm512_stream_pd(p_a, zmm1);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_a, zmm1);
#endif

#if UNROLL > 8
    zmm2 = _mm512_loadu_pd(p_b+8);
    zmm2 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+8), zmm0, zmm2);
#if defined (NT_STORES)
    _mm512_stream_pd(p_a+8, zmm2);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_a+8, zmm2);
#endif
#endif

#if UNROLL > 16
    zmm3 = _mm512_loadu_pd(p_b+16);
    zmm3 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+16), zmm0, zmm3);
#if defined (NT_STORES)
    _mm512_stream_pd(p_a+16, zmm3);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_a+16, zmm3);
#endif
#endif

#if UNROLL > 24
    zmm4 = _mm512_loadu_pd(p_b+24);
    zmm4 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+24), zmm0, zmm4);
#if defined (NT_STORES)
    _mm512_stream_pd(p_a+24, zmm4);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_a+24, zmm4);
#endif
#endif

#if 0
    _mm_prefetch((char*)(p_b+NUM_ELES_IN_4K), _MM_HINT_T1);
#if UNROLL > 8
    _mm_prefetch((char*)(p_b+NUM_ELES_IN_4K+8), _MM_HINT_T1);
#endif
#if UNROLL > 16
    _mm_prefetch((char*)(p_b+NUM_ELES_IN_4K+16), _MM_HINT_T1);
#endif
#if UNROLL > 24
    _mm_prefetch((char*)(p_b+NUM_ELES_IN_4K+24), _MM_HINT_T1);
#endif

    _mm_prefetch((char*)(p_c+NUM_ELES_IN_4K), _MM_HINT_T1);
#if UNROLL > 8
    _mm_prefetch((char*)(p_c+NUM_ELES_IN_4K+8), _MM_HINT_T1);
#endif
#if UNROLL > 16
    _mm_prefetch((char*)(p_c+NUM_ELES_IN_4K+16), _MM_HINT_T1);
#endif
#if UNROLL > 24
    _mm_prefetch((char*)(p_c+NUM_ELES_IN_4K+24), _MM_HINT_T1);
#endif

    //disable pf on stores even for RFO since it adversely impacts SpecI2M
#if 1
#if defined (RFO_STORES)
    // _MM_HINT_ET0 is slower than _MM_HINT_T1
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K), _MM_HINT_T1);
#if UNROLL > 8
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+8), _MM_HINT_T1);
#endif
#if UNROLL > 16
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+16), _MM_HINT_T1);
#endif
#if UNROLL > 24
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+24), _MM_HINT_T1);
#endif
#endif
#endif
#endif

#if 0
#if defined (NT_STORES)
    // _MM_HINT_ENTA chokes ICC
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K), _MM_HINT_NTA);
#if UNROLL > 8
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+8), _MM_HINT_NTA);
#endif
#if UNROLL > 16
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+16), _MM_HINT_NTA);
#endif
#if UNROLL > 24
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K+24), _MM_HINT_NTA);
#endif
#endif
#endif

    p_a   += UNROLL;
    p_c   += UNROLL;
    p_b   += UNROLL;
    n_blk -= UNROLL;
  }

  while (n_tail) {
    *(p_a++) = *(p_b++) + *(p_scalar) * *(p_c++);
    n_tail--;
  }
}

void triad_avx512_fwdbwd(ssize_t *p_n, double *p_scalar, double *p_c, double *p_b, double *p_a)
{
  __m512d zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7;

  int p, count = 0, fwd_prolog = 0, bwd_prolog = 0;
  ssize_t n = *p_n;

  double *p_aa = p_a + n;
  double *p_bb = p_b + n;
  double *p_cc = p_c + n;

  // Align on stores for both fwd and bwd access
#if 0
  if ((unsigned long long)p_a%64) {
    fwd_prolog = (64 - (unsigned long long)p_a%64)/sizeof(double);
    p = fwd_prolog;
    while (p) {
      *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
      p--;
    }
  }

  if ((unsigned long long)p_aa%64) {
    bwd_prolog = ((unsigned long long)p_aa%64)/sizeof(double);
    p = bwd_prolog;
    while (p) {
      *(--p_aa) = *(--p_bb) + *p_scalar * *(--p_cc);
      p--;
    }
  }
#endif

#if 1
  fwd_prolog = omp_get_thread_num()*32;
  p = fwd_prolog;
  while (p) {
    *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
    p--;
  }

  bwd_prolog = fwd_prolog;
  p = bwd_prolog;
  while (p) {
    *(--p_aa) = *(--p_bb) + *p_scalar * *(--p_cc);
    p--;
  }
#endif

  p_aa -= 8;
  p_bb -= 8;
  p_cc -= 8;

  // Make sure that store buffers are not crossing the 4K boundary
  // at the same time
  int fwd_align_4k = ((unsigned long long)p_a)%4096;
  int bwd_align_4k = ((unsigned long long)p_aa)%4096;

  if ((fwd_align_4k == 0) && (bwd_align_4k == 0)) {
    count = 64;
  } else if (fwd_align_4k == bwd_align_4k) {
    count = (4096 - fwd_align_4k)/sizeof(double);
  }

  for (int i=0; i<count; i++) {
    *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
  }
  fwd_prolog += count;

  ssize_t n_blk  = ((n-(fwd_prolog + bwd_prolog))/16)*16;
  ssize_t n_tail = n - n_blk - (fwd_prolog + bwd_prolog);

#ifdef DEBUG
  printf("%d: fwd_st_align_4k = %d, bwd_st_align_4k = %d\n", omp_get_thread_num(), (4096-fwd_align_4k)/8, (4096-bwd_align_4k)/8); fflush(0);
  printf("%d: fwd_ld1_align_4k = %ld, bwd_ld1_align_4k = %ld\n", omp_get_thread_num(), (4096-((unsigned long long)p_b)%4096)/8, (4096-((unsigned long long)p_bb)%4096)/8); fflush(0);
  printf("%d: fwd_ld2_align_4k = %ld, bwd_ld2_align_4k = %ld\n", omp_get_thread_num(), (4096-((unsigned long long)p_c)%4096)/8, (4096-((unsigned long long)p_cc)%4096)/8); fflush(0);
#endif

  zmm0 = _mm512_broadcastsd_pd(_mm_load_sd(p_scalar));

  while (n_blk) {
    zmm1 = _mm512_loadu_pd(p_b);
    zmm1 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c), zmm0, zmm1);
#if defined (NT_STORES)
    _mm512_stream_pd(p_a, zmm1);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_a, zmm1);
#endif

    zmm2 = _mm512_loadu_pd(p_bb);
    zmm2 = _mm512_fmadd_pd(_mm512_loadu_pd(p_cc), zmm0, zmm2);
#if defined (NT_STORES)
    _mm512_stream_pd(p_aa, zmm2);
#elif defined (RFO_STORES)
    _mm512_storeu_pd(p_aa, zmm2);
#endif

// #define ENABLE_PF
#if defined (ENABLE_PF)
    _mm_prefetch((char*)(p_b+NUM_ELES_IN_4K), _MM_HINT_T1);
    _mm_prefetch((char*)(p_bb-NUM_ELES_IN_4K), _MM_HINT_T1);

    _mm_prefetch((char*)(p_c+NUM_ELES_IN_4K), _MM_HINT_T1);
    _mm_prefetch((char*)(p_cc-NUM_ELES_IN_4K), _MM_HINT_T1);

    //disable pf on stores even for RFO since it adversely impacts SpecI2M
#if 0
#if defined (RFO_STORES)
    // _MM_HINT_ET0 is slower than _MM_HINT_T1
     _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K), _MM_HINT_T1);
     _mm_prefetch((char*)(p_aa-NUM_ELES_IN_4K), _MM_HINT_T1);
#endif

#if defined (NT_STORES)
    // _MM_HINT_ENTA chokes ICC
    _mm_prefetch((char*)(p_a+NUM_ELES_IN_4K), _MM_HINT_NTA);
    _mm_prefetch((char*)(p_aa-NUM_ELES_IN_4K), _MM_HINT_NTA);
#endif
#endif
#endif


    p_a += 8;
    p_c += 8;
    p_b += 8;

    p_aa -= 8;
    p_cc -= 8;
    p_bb -= 8;
    n_blk -= 16;
  }


  while (n_tail) {
    *(p_a++) = *(p_b++) + *(p_scalar) * *(p_c++);
    n_tail--;
  }
}
