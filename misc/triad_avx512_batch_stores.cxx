#include "immintrin.h"

#if defined (RFO_STORES)
#define STORE_PD  _mm512_storeu_pd
#elif defined (NT_STORES)
#define STORE_PD  _mm512_stream_pd
#else
#error "store type definition is missing!
#endif

void triad_avx512_batch_stores(ssize_t *p_n, double *p_scalar, double *p_c, double *p_b, double *p_a)
{
  ssize_t j, i;
  __m512d zmm0, zmm1, zmm2, zmm3, zmm4, zmm5, zmm6, zmm7,
          zmm8, zmm9, zmm10, zmm11, zmm12, zmm13, zmm14, zmm15,
          zmm16, zmm17, zmm18, zmm19, zmm20, zmm21, zmm22, zmm23,
          zmm24, zmm25, zmm26, zmm27, zmm28, zmm29, zmm30, zmm31;

  int p, prolog = 0;
  ssize_t n = *p_n;

  if ((unsigned long long)p_a%64) {
    prolog = (64 - (unsigned long long)p_a%64)/sizeof(double);
    p = prolog;
    while (p) {
      *(p_a++) = *(p_b++) + *p_scalar * *(p_c++);
      p--;
    } 
  }

  ssize_t n_blk  = ((n-prolog)/248)*248;
  ssize_t n_tail = n - n_blk - prolog;

  zmm0 = _mm512_broadcastsd_pd(_mm_load_sd(p_scalar));

mainloop:
  while (n_blk) {
    zmm1 = _mm512_loadu_pd(p_b);
    zmm1 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c), zmm0, zmm1);

    zmm2 = _mm512_loadu_pd(p_b+8);
    zmm2 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+8), zmm0, zmm2);

    zmm3 = _mm512_loadu_pd(p_b+16);
    zmm3 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+16), zmm0, zmm3);

    zmm4 = _mm512_loadu_pd(p_b+24);
    zmm4 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+24), zmm0, zmm4);

    zmm5  = _mm512_loadu_pd(p_b+32);
    zmm5 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+32), zmm0, zmm5);

    zmm6  = _mm512_loadu_pd(p_b+40);
    zmm6 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+40), zmm0, zmm6);

    zmm7  = _mm512_loadu_pd(p_b+48);
    zmm7 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+48), zmm0, zmm7);

    zmm8  = _mm512_loadu_pd(p_b+56);
    zmm8 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+56), zmm0, zmm8);

    zmm9  = _mm512_loadu_pd(p_b+64);
    zmm9 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+64), zmm0, zmm9);

    zmm10  = _mm512_loadu_pd(p_b+72);
    zmm10 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+72), zmm0, zmm10);

    zmm11  = _mm512_loadu_pd(p_b+80);
    zmm11 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+80), zmm0, zmm11);

    zmm12  = _mm512_loadu_pd(p_b+88);
    zmm12 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+88), zmm0, zmm12);

    zmm13  = _mm512_loadu_pd(p_b+96);
    zmm13 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+96), zmm0, zmm13);

    zmm14  = _mm512_loadu_pd(p_b+104);
    zmm14 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+104), zmm0, zmm14);

    zmm15  = _mm512_loadu_pd(p_b+112);
    zmm15 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+112), zmm0, zmm15);

    zmm16  = _mm512_loadu_pd(p_b+120);
    zmm16 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+120), zmm0, zmm16);

    zmm17  = _mm512_loadu_pd(p_b+128);
    zmm17 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+128), zmm0, zmm17);

    zmm18  = _mm512_loadu_pd(p_b+136);
    zmm18 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+136), zmm0, zmm18);

    zmm19  = _mm512_loadu_pd(p_b+144);
    zmm19 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+144), zmm0, zmm19);

    zmm20  = _mm512_loadu_pd(p_b+152);
    zmm20 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+152), zmm0, zmm20);

    zmm21  = _mm512_loadu_pd(p_b+160);
    zmm21 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+160), zmm0, zmm21);

    zmm22  = _mm512_loadu_pd(p_b+168);
    zmm22 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+168), zmm0, zmm22);

    zmm23  = _mm512_loadu_pd(p_b+176);
    zmm23 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+176), zmm0, zmm23);

    zmm24  = _mm512_loadu_pd(p_b+184);
    zmm24 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+184), zmm0, zmm24);

    zmm25  = _mm512_loadu_pd(p_b+192);
    zmm25 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+192), zmm0, zmm25);

    zmm26  = _mm512_loadu_pd(p_b+200);
    zmm26 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+200), zmm0, zmm26);

    zmm27  = _mm512_loadu_pd(p_b+208);
    zmm27 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+208), zmm0, zmm27);

    zmm28  = _mm512_loadu_pd(p_b+216);
    zmm28 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+216), zmm0, zmm28);

    zmm29  = _mm512_loadu_pd(p_b+224);
    zmm29 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+224), zmm0, zmm29);

    zmm30  = _mm512_loadu_pd(p_b+232);
    zmm30 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+232), zmm0, zmm30);

    zmm31  = _mm512_loadu_pd(p_b+240);
    zmm31 = _mm512_fmadd_pd(_mm512_loadu_pd(p_c+240), zmm0, zmm31);

    goto store;
  }

store:
  if (n_blk) {
    STORE_PD(p_a, zmm1);
    STORE_PD(p_a+8, zmm2);
    STORE_PD(p_a+16, zmm3);
    STORE_PD(p_a+24, zmm4);
    STORE_PD(p_a+32, zmm5);
    STORE_PD(p_a+40, zmm6);
    STORE_PD(p_a+48, zmm7);
    STORE_PD(p_a+56, zmm8);
    STORE_PD(p_a+64, zmm9);
    STORE_PD(p_a+72, zmm10);
    STORE_PD(p_a+80, zmm11);
    STORE_PD(p_a+88, zmm12);
    STORE_PD(p_a+96, zmm13);
    STORE_PD(p_a+104, zmm14);
    STORE_PD(p_a+112, zmm15);
    STORE_PD(p_a+120, zmm16);

    STORE_PD(p_a+128, zmm17);
    STORE_PD(p_a+136, zmm18);
    STORE_PD(p_a+144, zmm19);
    STORE_PD(p_a+152, zmm20);
    STORE_PD(p_a+160, zmm21);
    STORE_PD(p_a+168, zmm22);
    STORE_PD(p_a+176, zmm23);
    STORE_PD(p_a+184, zmm24);
    STORE_PD(p_a+192, zmm25);
    STORE_PD(p_a+200, zmm26);
    STORE_PD(p_a+208, zmm27);
    STORE_PD(p_a+216, zmm28);
    STORE_PD(p_a+224, zmm29);
    STORE_PD(p_a+232, zmm30);
    STORE_PD(p_a+240, zmm31);

    p_a   += 248;
    p_c   += 248;
    p_b   += 248;
    n_blk -= 248;

    goto mainloop;
  }


  while (n_tail) {
    *(p_a++) = *(p_b++) + *(p_scalar) * *(p_c++);
    n_tail--;
  }
}
