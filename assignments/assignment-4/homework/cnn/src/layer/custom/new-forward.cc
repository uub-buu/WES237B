#include <cmath>
#include <iostream>
#include <vector>

#include <clblast.h>

#include "kernel.h"
#include "device.h"

#include "opencl-new-forward.h"

#define CHECK_ERR(err, msg)                            \
    if (err != CL_SUCCESS)                             \
    {                                                  \
        fprintf(stderr, "%s failed: %d.\n", msg, err); \
        exit(EXIT_FAILURE);                            \
    }

void OpenCLInterface::conv_forward_gemm_opencl_prolog(
    const float *host_y, const float *host_x, const float *host_k,
    cl_mem *device_y, cl_mem *device_x, cl_mem *device_k, cl_mem *device_x_unroll,
    const int B, const int M, const int C,
    const int H, const int W, const int K)
{
    cl_int err;

    //@@ Allocate GPU memory here (don't forget batch sizes!)
    int output_row = H - K + 1;
    int output_col = W - K + 1;
    // define buffer sizes for all our memory devicesz
    size_t buffer_size_dev_x = W * H * C * B * sizeof(float);
    size_t buffer_size_dev_y = output_col * output_row * M * B * sizeof(float);
    size_t buffer_size_dev_k = K * K * sizeof(float);
    size_t buffer_size_dev_unroll = B * C * K * K * output_col * output_row * sizeof(float);

    *device_y = clCreateBuffer(opencl->context,
                               CL_MEM_READ_WRITE,
                               buffer_size_dev_y,
                               NULL,
                               &err);
    CHECK_ERR(err, "clCreateBuffer for device_y");
    *device_x = clCreateBuffer(opencl->context,
                               CL_MEM_READ_ONLY,
                               buffer_size_dev_x,
                               NULL,
                               &err);
    CHECK_ERR(err, "clCreateBuffer for device_x");
    *device_x_unroll = clCreateBuffer(opencl->context,
                                      CL_MEM_READ_WRITE,
                                      buffer_size_dev_unroll,
                                      NULL,
                                      &err);
    CHECK_ERR(err, "clCreateBuffer for device_x_unroll");

    *device_k = clCreateBuffer(opencl->context,
                               CL_MEM_READ_ONLY,
                               buffer_size_dev_k,
                               NULL,
                               &err);
    CHECK_ERR(err, "clCreateBuffer for device_k");
    //@@ Copy memory to the GPU here
    err = clEnqueueWriteBuffer(opencl->queue, *device_y, CL_TRUE, 0, buffer_size_dev_y, host_y, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_y");

    err = clEnqueueWriteBuffer(opencl->queue, *device_x, CL_TRUE, 0, buffer_size_dev_x, host_x, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_x");

    err = clEnqueueWriteBuffer(opencl->queue, *device_k, CL_TRUE, 0, buffer_size_dev_k, host_k, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_k");

    err = clEnqueueWriteBuffer(opencl->queue, *device_x_unroll, CL_TRUE, 0, buffer_size_dev_unroll, NULL, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_y");
}

void OpenCLInterface::conv_forward_gemm_opencl(cl_mem device_y, const cl_mem device_x, const cl_mem device_k, const cl_mem device_x_unroll, const int B, const int M, const int C, const int H, const int W, const int K)
{
    //@@ ====== Start im2col =====

    // @@ define local and global work sizes

    //@@ Launch the im2col kernel here

    //@@ ====== End im2col =====

    //@@ ====== Start gemm =====

    // @@ Call clblast::GemmBatched here

    //@@ ====== End gemm =====
}

void OpenCLInterface::conv_forward_gemm_opencl_epilog(float *host_y, cl_mem device_y, cl_mem device_x, cl_mem device_k, cl_mem device_x_unroll, const int B, const int M, const int C, const int H, const int W, const int K)
{
    //@@ Copy the output back to host

    //@@ Free the GPU memory here
}
