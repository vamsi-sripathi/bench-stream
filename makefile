CC  = icc

# STREAM options:
# -DNTIMES control the number of times each stream kernel is executed
# -DOFFSET controls the number of bytes between each of the buffers
# -DSTREAM_TYPE specifies the data-type of elements in the buffers
# -DSTREAM_ARRAY_SIZE specifies the number of elements in each buffer
STREAM_CPP_OPTS   = -DNTIMES=100 -DOFFSET=0 -DSTREAM_TYPE=double
STREAM_ARRAY_SIZE = 134217728 #(128*1024*1024)

ifdef stream_array_size
STREAM_ARRAY_SIZE = $(stream_array_size)
endif
STREAM_CPP_OPTS += -DSTREAM_ARRAY_SIZE=$(STREAM_ARRAY_SIZE)

# Intel Compiler options to control the generated ISA
AVX_COPTS         = -xAVX
AVX2_COPTS        = -xCORE-AVX2
# MIC_AVX512_COPTS  = -xMIC-AVX512
AVX512_COPTS      = -xCORE-AVX512 -qopt-zmm-usage=high

# Common Intel Compiler options that are independent of ISA
COMMON_COPTS  = -Wall -O3 -mcmodel=medium -qopenmp -shared-intel -qopt-streaming-stores always

SRC = stream.c

ifdef cpu_target
all: stream_$(cpu_target).bin
else
all: stream_avx.bin stream_avx2.bin stream_avx512.bin
endif

stream_avx.bin: $(SRC)
	@echo -e "\n======= Generating STREAM benchmark with AVX ISA ======="
	$(CC) $(COMMON_COPTS) $(AVX_COPTS) $(STREAM_CPP_OPTS) -c $< -o stream_avx.o
	$(CC) $(COMMON_COPTS) $(AVX_COPTS) stream_avx.o -o $@

stream_avx2.bin: $(SRC)
	@echo -e "\n======= Generating STREAM benchmark with AVX2 ISA ======="
	$(CC) $(COMMON_COPTS) $(AVX2_COPTS) $(STREAM_CPP_OPTS) -c $< -o stream_avx2.o
	$(CC) $(COMMON_COPTS) $(AVX2_COPTS) stream_avx2.o -o $@

stream_avx512.bin: $(SRC)
	@echo -e "\n======= Generating STREAM benchmark with AVX512 ISA ======="
	$(CC) $(COMMON_COPTS) $(AVX512_COPTS) $(STREAM_CPP_OPTS) -c $< -o stream_avx512.o
	$(CC) $(COMMON_COPTS) $(AVX512_COPTS) stream_avx512.o -o $@

help:
	@echo -e "Running 'make' with no options would compile the STREAM benchmark with $(STREAM_ARRAY_SIZE) FP64 elements per array for following Intel CPU's:\n"
	@echo -e "\tstream_avx.bin        => Targeted for Intel CPU's that support AVX ISA"
	@echo -e "\tstream_avx2.bin       => Targeted for Intel CPU's that support AVX2 ISA"
	@echo -e "\tstream_avx512.bin     => Targeted for Intel CPU's that support AVX512 ISA"
	@echo -e "\nThe following options are supported:"
	@echo -e "\tstream_array_size=<number_of_elements_per_array>"
	@echo ""
	@echo -e "\tcpu_target=<avx,avx2,avx512>"
	@echo -e "\nFew examples:"
	@echo -e "To compile STREAM benchmark only for Intel AVX512 CPU's, do:"
	@echo -e "\tmake cpu_target=avx512"
	@echo ""
	@echo -e "To compile STREAM benchmark for Intel AVX512 CPU's with each buffer containing 67108864 elements, do:"
	@echo -e "\tmake stream_array_size=67108864 cpu_target=avx512"


clean:
	rm -rf *.o *.bin 

.PHONY: all clean help
