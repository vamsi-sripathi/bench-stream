CC  = icc

# STREAM options:
# -DNTIMES control the number of times each stream kernel is executed
# -DOFFSET controls the number of bytes between each of the buffers
# -DSTREAM_TYPE specifies the data-type of elements in the buffers
# -DSTREAM_ARRAY_SIZE specifies the number of elements in each buffer
#  size -A -d *.bin || nm --print-size --size-sort --radix=d *.bin
STREAM_CPP_OPTS   = -DNTIMES=100 -DOFFSET=0 -DSTREAM_TYPE=double
STREAM_ARRAY_SIZE = 134217728 #(=128*1024*1024)

ifdef stream_array_size
STREAM_ARRAY_SIZE = $(stream_array_size)
endif
STREAM_CPP_OPTS += -DSTREAM_ARRAY_SIZE=$(STREAM_ARRAY_SIZE)

ifdef amd
AVX_COPTS         = -axAVX -march=corei7-avx
AVX2_COPTS        = -axCORE-AVX2 -march=core-avx2 
AVX512_COPTS      = 
else
# Intel Compiler options to control the generated ISA
AVX_COPTS         = -xAVX
AVX2_COPTS        = -xCORE-AVX2
AVX512_COPTS      = -xCORE-AVX512 -qopt-zmm-usage=high
endif

# Common Intel Compiler options that are independent of ISA
COMMON_COPTS  = -Wall -O3 -mcmodel=medium -qopenmp -shared-intel

ifdef use_rfo
COMMON_COPTS += -qopt-streaming-stores never -fno-builtin
else
COMMON_COPTS += -qopt-streaming-stores always
endif


AVX_OBJS = stream_avx.o
AVX2_OBJS = stream_avx2.o
AVX512_OBJS = stream_avx512.o

ifdef cpu_target
all: stream_$(cpu_target).bin
else
ifdef amd
all: stream_avx.bin stream_avx2.bin
else
all: stream_avx.bin stream_avx2.bin stream_avx512.bin
endif
endif


ifdef extra_kernels
STREAM_CPP_OPTS += -DSTREAM_EXT_KERNELS
SRC = stream_x.c
AVX_OBJS += stream_avx_ext_kernels.o
AVX2_OBJS += stream_avx2_ext_kernels.o
AVX512_OBJS += stream_avx512_ext_kernels.o
else
SRC = stream.c
endif

stream_avx.o: stream.h $(SRC)
	$(CC) $(COMMON_COPTS) $(AVX_COPTS) $(STREAM_CPP_OPTS) -c $(SRC) -o $@
stream_avx2.o: stream.h $(SRC)
	$(CC) $(COMMON_COPTS) $(AVX2_COPTS) $(STREAM_CPP_OPTS) -c $(SRC) -o $@
stream_avx512.o: stream.h $(SRC)
	$(CC) $(COMMON_COPTS) $(AVX512_COPTS) $(STREAM_CPP_OPTS) -c $(SRC) -o $@

stream_avx_ext_kernels.o: stream.h stream_ext_kernels.c
	$(CC) $(COMMON_COPTS) $(AVX_COPTS) $(STREAM_CPP_OPTS) -c stream_ext_kernels.c -o $@
stream_avx2_ext_kernels.o:
	$(CC) $(COMMON_COPTS) $(AVX2_COPTS) $(STREAM_CPP_OPTS) -c stream_ext_kernels.c -o $@
stream_avx512_ext_kernels.o:
	$(CC) $(COMMON_COPTS) $(AVX512_COPTS) $(STREAM_CPP_OPTS) -c stream_ext_kernels.c -o $@

stream_avx.bin: $(AVX_OBJS)
	$(CC) $(COMMON_COPTS) $(AVX_COPTS) $^ -o $@

stream_avx2.bin: $(AVX2_OBJS)
	$(CC) $(COMMON_COPTS) $(AVX2_COPTS) $^ -o $@

stream_avx512.bin: $(AVX512_OBJS)
	$(CC) $(COMMON_COPTS) $(AVX512_COPTS) $^ -o $@

help:
	@echo -e "Running 'make' with no options would compile the STREAM benchmark with $(STREAM_ARRAY_SIZE) FP64 elements per array for following Intel CPU's:\n"
	@echo -e "\tstream_avx.bin        => Targeted for Intel CPU's that support AVX ISA"
	@echo -e "\tstream_avx2.bin       => Targeted for Intel CPU's that support AVX2 ISA"
	@echo -e "\tstream_avx512.bin     => Targeted for Intel CPU's that support AVX512 ISA"
	@echo -e "\nThe following options are supported:"
	@echo -e "\tstream_array_size=<number_of_elements_per_array>"
	@echo ""
	@echo -e "\tamd=1 generates binary targeted for AMD processors"
	@echo ""
	@echo -e "\tcpu_target=<avx,avx2,avx512>"
	@echo ""
	@echo -e "\tuse_rfo=1 forces to use regular stores instead of Non-temporal stores"
	@echo ""
	@echo -e "\textra_kernels=1 include additonal kernels (reduce, fill) in generated binary"
	@echo ""
	@echo -e "\nFew examples:"
	@echo -e "To compile STREAM benchmark only for Intel AVX512 CPU's, do:"
	@echo -e "\tmake cpu_target=avx512"
	@echo ""
	@echo -e "To compile STREAM benchmark for Intel AVX512 CPU's with each buffer containing 67108864 elements, do:"
	@echo -e "\tmake stream_array_size=67108864 cpu_target=avx512"


clean:
	rm -rf *.o *.bin 

.PHONY: all clean help
