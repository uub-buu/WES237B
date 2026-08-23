CC       = gcc
CFLAGS   = -g -Wall
INCFLAGS := -I../../helper_lib
LDFLAGS  := ../../helper_lib/helper_lib.a -lm
DATASET_PATH := $(strip $(shell cat Dataset))

ifeq ($(shell uname -o), Darwin)
	LDFLAGS += -framework OpenCL
else ifeq ($(shell uname -o), GNU/Linux) # Assumes NVIDIA GPU
	LDFLAGS  += -L/usr/local/cuda/lib64 -lOpenCL
	INCFLAGS += -I/usr/local/cuda/include
else # Android
	LDFLAGS += -lOpenCL
endif

all: solution

solution: ../../helper_lib/helper_lib.a main.c
	$(CC) $(CFLAGS) -o $@ $^ $(INCFLAGS) $(LDFLAGS)

../../helper_lib/helper_lib.a: 
	cd ../../helper_lib; make

run: solution
	./solution $(DATASET_PATH)0/input0.raw $(DATASET_PATH)0/input1.raw $(DATASET_PATH)0/output.raw output.raw
	./solution $(DATASET_PATH)1/input0.raw $(DATASET_PATH)1/input1.raw $(DATASET_PATH)1/output.raw output.raw
	./solution $(DATASET_PATH)2/input0.raw $(DATASET_PATH)2/input1.raw $(DATASET_PATH)2/output.raw output.raw
	./solution $(DATASET_PATH)3/input0.raw $(DATASET_PATH)3/input1.raw $(DATASET_PATH)3/output.raw output.raw
	./solution $(DATASET_PATH)4/input0.raw $(DATASET_PATH)4/input1.raw $(DATASET_PATH)4/output.raw output.raw
	./solution $(DATASET_PATH)5/input0.raw $(DATASET_PATH)5/input1.raw $(DATASET_PATH)5/output.raw output.raw
	./solution $(DATASET_PATH)6/input0.raw $(DATASET_PATH)6/input1.raw $(DATASET_PATH)6/output.raw output.raw
	./solution $(DATASET_PATH)7/input0.raw $(DATASET_PATH)7/input1.raw $(DATASET_PATH)7/output.raw output.raw
	./solution $(DATASET_PATH)8/input0.raw $(DATASET_PATH)8/input1.raw $(DATASET_PATH)8/output.raw output.raw
	./solution $(DATASET_PATH)9/input0.raw $(DATASET_PATH)9/input1.raw $(DATASET_PATH)9/output.raw output.raw

clean: 
	rm -f solution
	cd ../../helper_lib; make clean