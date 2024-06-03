#define NAME        triad_avx512
#define BASE_SHIFT  3
#define SIZE        8

#define ALIGN_4 .p2align 4

#define M       %rdi
#define ALPHA   %rsi
#define X       %rdx
#define Y       %rcx
#define Z       %r8
#define I       %r9

#if 0
#define	PREFETCHR	prefetcht0
#define	PREFETCHSIZER	48
#define	PREFETCHW	prefetcht0
#define	PREFETCHSIZEW	PREFETCHSIZER
#endif

	.text
	.align 256
	.globl NAME
	.type NAME, @function
NAME:
	mov	(M),  M
	vbroadcastsd	(ALPHA), %zmm8

	test	M, M
	jle	.L999

	sub	$-64 * SIZE, X
	sub	$-64 * SIZE, Y
	sub	$-64 * SIZE, Z

#ifdef PROLOG 
	test	$(SIZE), Z
	je	.L05

  // xmm0 = xmm7 * Y + xmm0
	vmovsd	     -64 * SIZE(X), %xmm0
	vfmadd213sd  -64 * SIZE(Y), %xmm8, %xmm0
	vmovlpd	     %xmm0, -64 * SIZE(Z)	

	add	SIZE, X
	add	SIZE, Y
	add	SIZE, Z

	dec	M
	jle	.L999
	ALIGN_4

.L03:
  cmp $4, M
  jle .L18

  test $8 * SIZE, Z
  je .L10

  vmovupd

.L05:
	cmp	$2, M
	jle	.L19

// if Z is not aligned
	test	$2 * SIZE, Z
	je	.L10

	vmovupd	    -64 * SIZE(X), %xmm0
	vfmadd213pd	-64 * SIZE(Y), %xmm8, %xmm0
	vmovntpd	  %xmm0, -64 * SIZE(Z)

	add	$2 * SIZE, X
	add	$2 * SIZE, Y
	add	$2 * SIZE, Z

	sub	$2, M
	jle	.L999	
	ALIGN_4

// At least, Z is aligned
#endif

.L10:
	mov	M,  I
	sar	$6, I
	jle	.L15

	vmovupd	-64 * SIZE(X), %zmm0
	vmovupd	-56 * SIZE(X), %zmm1
	vmovupd	-48 * SIZE(X), %zmm2
	vmovupd	-40 * SIZE(X), %zmm3
	vmovupd	-32 * SIZE(X), %zmm4
	vmovupd	-24 * SIZE(X), %zmm5
	vmovupd	-16 * SIZE(X), %zmm6
	vmovupd	 -8 * SIZE(X), %zmm7

	dec	I
	jle	.L12
	ALIGN_4
	
// Triad main loop
.L11:
#ifdef	PREFETCHR
	PREFETCHR (PREFETCHSIZER +   0) * SIZE(X)
#endif

	vfmadd213pd	-64 * SIZE(Y), %zmm8, %zmm0
	vfmadd213pd	-56 * SIZE(Y), %zmm8, %zmm1
	vfmadd213pd	-48 * SIZE(Y), %zmm8, %zmm2
	vfmadd213pd	-40 * SIZE(Y), %zmm8, %zmm3
	vfmadd213pd	-32 * SIZE(Y), %zmm8, %zmm4
	vfmadd213pd	-24 * SIZE(Y), %zmm8, %zmm5
	vfmadd213pd	-16 * SIZE(Y), %zmm8, %zmm6
	vfmadd213pd	-8  * SIZE(Y), %zmm8, %zmm7

#ifdef	PREFETCHW
	PREFETCHW (PREFETCHSIZEW +   0) * SIZE(Y)
#endif

	vmovntpd	%zmm0, -64 * SIZE(Z)
	vmovupd	  0 * SIZE(X), %zmm0

	vmovntpd	%zmm1, -56 * SIZE(Z)
	vmovupd	  8 * SIZE(X), %zmm1

	vmovntpd	%zmm2, -48 * SIZE(Z)
	vmovupd	  16  * SIZE(X), %zmm2

	vmovntpd	%zmm3, -40 * SIZE(Z)
	vmovupd	  24 * SIZE(X), %zmm3

	vmovntpd	%zmm4, -32 * SIZE(Z)
	vmovupd	  32 * SIZE(X), %zmm4

	vmovntpd	%zmm5, -24 * SIZE(Z)
	vmovupd	  40 * SIZE(X), %zmm5

	vmovntpd	%zmm6, -16 * SIZE(Z)
	vmovupd	  48 * SIZE(X), %zmm6

	vmovntpd	%zmm7, -8 * SIZE(Z)
	vmovupd	  56 * SIZE(X), %zmm7

	sub	$-64 * SIZE, Z
	sub	$-64 * SIZE, Y
	sub	$-64 * SIZE, X

	dec	I
	jg	.L11
	ALIGN_4

.L12:
	vfmadd213pd	-64 * SIZE(Y), %zmm8, %zmm0
	vfmadd213pd	-56 * SIZE(Y), %zmm8, %zmm1
	vfmadd213pd	-48 * SIZE(Y), %zmm8, %zmm2
	vfmadd213pd	-40 * SIZE(Y), %zmm8, %zmm3
	vfmadd213pd	-32 * SIZE(Y), %zmm8, %zmm4
	vfmadd213pd	-24 * SIZE(Y), %zmm8, %zmm5
	vfmadd213pd	-16 * SIZE(Y), %zmm8, %zmm6
	vfmadd213pd	 -8 * SIZE(Y), %zmm8, %zmm7

	vmovntpd	%zmm0, -64 * SIZE(Z)
	vmovntpd	%zmm1, -56 * SIZE(Z)
	vmovntpd	%zmm1, -48 * SIZE(Z)
	vmovntpd	%zmm1, -40 * SIZE(Z)
	vmovntpd	%zmm1, -32 * SIZE(Z)
	vmovntpd	%zmm1, -24 * SIZE(Z)
	vmovntpd	%zmm1, -16 * SIZE(Z)
	vmovntpd	%zmm1, -8  * SIZE(Z)

	sub	$-64 * SIZE, Z
	sub	$-64 * SIZE, Y
	sub	$-64 * SIZE, X
	ALIGN_4

.L15:
// if (M & 32)
	test	$32, M
	jle	.L16

	vmovupd	     -64 * SIZE(X), %zmm0
	vfmadd213pd	 -64 * SIZE(Y), %zmm8, %zmm0
	vmovntpd	   %zmm0, -64 * SIZE(Z)

	vmovupd	     -56 * SIZE(X), %zmm1
	vfmadd213pd	 -56 * SIZE(Y), %zmm8, %zmm1
	vmovntpd	   %zmm1, -56 * SIZE(Z)

	vmovupd	     -48 * SIZE(X), %zmm2
	vfmadd213pd	 -48 * SIZE(Y), %zmm8, %zmm2
	vmovntpd	   %zmm2, -48 * SIZE(Z)
  
	vmovupd	     -40 * SIZE(X), %zmm3
	vfmadd213pd	 -40 * SIZE(Y), %zmm8, %zmm3
	vmovntpd	   %zmm3, -40 * SIZE(Z)

	add	$32 * SIZE, Z
	add	$32 * SIZE, Y
	add	$32 * SIZE, X
	ALIGN_4

.L16:
// if (M & 16)
	test	$16, M
	jle	.L17

	vmovupd	     -64 * SIZE(X), %zmm0
	vfmadd213pd	 -64 * SIZE(Y), %zmm8, %zmm0
	vmovntpd	   %zmm0, -64 * SIZE(Z)

	vmovupd	     -56 * SIZE(X), %zmm1
	vfmadd213pd	 -56 * SIZE(Y), %zmm8, %zmm1
	vmovntpd	   %zmm1, -56 * SIZE(Z)

	add	$16 * SIZE, Z
	add	$16 * SIZE, Y
	add	$16 * SIZE, X
	ALIGN_4	


.L17:
// if (M & 8)
	test	$8, M
	jle	.L18

	vmovupd	     -64 * SIZE(X), %zmm0
	vfmadd213pd	 -64 * SIZE(Y), %zmm8, %zmm0
	vmovntpd	   %zmm0, -64 * SIZE(Z)

	add	$8 * SIZE, Z
	add	$8 * SIZE, Y
	add	$8 * SIZE, X
	ALIGN_4	


.L18:
// if (M & 4)
	test	$4, M
	jle	.L19

	vmovupd	    -64 * SIZE(X), %ymm0
	vfmadd213pd	-64 * SIZE(Y), %ymm8, %ymm0
	vmovntpd    %ymm0, -64 * SIZE(Z)

	add	$4 * SIZE, Z
	add	$4 * SIZE, Y
	add	$4 * SIZE, X
	ALIGN_4	

.L19:
// if (M & 2)
	test	$2, M
	jle	.L20

	vmovupd	    -64 * SIZE(X), %xmm0
	vfmadd213pd	-64 * SIZE(Y), %xmm7, %xmm0
	vmovntpd	  %xmm0, -64 * SIZE(Z)

	add	$2 * SIZE, Z
	add	$2 * SIZE, Y
	add	$2 * SIZE, X
	ALIGN_4	

.L20:
// if (M & 1)
	test	$1, M
	jle	.L999

	vmovsd	    -64 * SIZE(X), %xmm0
	vfmadd213sd -64 * SIZE(Y), %xmm7, %xmm0
	vmovlpd	    %xmm0, -64 * SIZE(Z)	

	jmp	.L999
	ALIGN_4

.L999:
// Recovering all stack/registers

	vzeroupper
	
	ret
	.size	 NAME, .-NAME
