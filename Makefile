EXECUTABLES = DeviceInfo vec-add vec-add-pseudo-opencl vec-add-opencl
CC = gcc

all: $(EXECUTABLES)

LDFLAGS += $(foreach librarydir,$(subst :, ,$(LD_LIBRARY_PATH)),-L$(librarydir))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
  LDFLAGS += -lrt -lOpenCL
  CFLAGS += -std=gnu99 -g -O2
endif
ifeq ($(UNAME_S),Darwin)
  LDFLAGS +=  -framework OpenCL
  CFLAGS += -std=c99 -g -O2
endif

ifdef OPENCL_INC
  CL_CFLAGS = -I$(OPENCL_INC)
endif

ifdef OPENCL_LIB
  CL_LDFLAGS = -L$(OPENCL_LIB)
endif


DeviceInfo: DeviceInfo.c cl-helper.c
	$(CC) $(CFLAGS) $(CL_CFLAGS) $(CL_LDFLAGS)  -o $@ $^ $(LDFLAGS)

vec-add: vec-add.c cl-helper.c
	$(CC) $(CFLAGS) $(CL_CFLAGS) $(CL_LDFLAGS)  -o $@ $^ $(LDFLAGS)

vec-add-pseudo-opencl: vec-add-pseudo-opencl.c cl-helper.c
	$(CC) $(CFLAGS) $(CL_CFLAGS) $(CL_LDFLAGS)  -o $@ $^ $(LDFLAGS)

vec-add-opencl: vec-add-opencl.c cl-helper.c
	$(CC) $(CFLAGS) $(CL_CFLAGS) $(CL_LDFLAGS)  -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) *.o
