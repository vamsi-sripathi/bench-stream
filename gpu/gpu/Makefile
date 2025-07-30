ifdef N
STREAM_ARRAY_SIZE=$(N)
else
STREAM_ARRAY_SIZE=134217728
endif

CXX      = dpcpp
CXXFLAGS = -O3 -DSTREAM_ARRAY_SIZE=$(STREAM_ARRAY_SIZE) -DOFFSET=0 -DNTIMES=100 -DTUNED

all: stream_sycl.bin stream_sycl_extra.bin

stream_sycl.bin: stream.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

stream_sycl_extra.bin: stream_extra.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.bin *.o
