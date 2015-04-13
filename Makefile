EXECUTABLES = DeviceInfo
CC = gcc49

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


DeviceInfo: DeviceInfo.c
	$(CC) $(CFLAGS) $(CL_CFLAGS) $(CL_LDFLAGS)  -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) *.o
