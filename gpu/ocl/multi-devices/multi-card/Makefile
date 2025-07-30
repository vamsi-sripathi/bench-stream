CC=gcc
OCL_INCLUDE=/opt/intel/opencl/include
CCOPTS=--std=gnu99 -I$(OCL_INCLUDE)
LIBS=-lOpenCL -lm

all: stream-ro stream-wo stream-scale stream-triad

stream-ro: stream_x.c ocl_kernels.cl ocl_util.h
	$(CC)  -DRO_KERNEL -DTYPE_DOUBLE $(CCOPTS) $< -o $@ $(LIBS)
stream-wo: stream_x.c ocl_kernels.cl ocl_util.h
	$(CC)  -DWO_KERNEL -DTYPE_DOUBLE $(CCOPTS) $< -o $@ $(LIBS)
stream-scale: stream_x.c ocl_kernels.cl ocl_util.h
	$(CC)  -DSCALE_KERNEL -DTYPE_DOUBLE $(CCOPTS) $< -o $@ $(LIBS)
stream-triad: stream_x.c ocl_kernels.cl ocl_util.h
	$(CC)  -DTRIAD_KERNEL -DTYPE_DOUBLE $(CCOPTS) $< -o $@ $(LIBS)

clean:
	rm -rf *.o stream-ro stream-wo stream-scale stream-triad 

.PHONY: all clean
