#include <cmath>
#include <iostream>
#include <vector>

#include <clblast.h>

#include "kernel.h"
#include "device.h"

#include "opencl-new-forward.h"

#define TILE_SIZE 16
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

    err |= clEnqueueWriteBuffer(opencl->queue, *device_x, CL_TRUE, 0, buffer_size_dev_x, host_x, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_x");

    err |= clEnqueueWriteBuffer(opencl->queue, *device_k, CL_TRUE, 0, buffer_size_dev_k, host_k, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer for device_k");
}

void OpenCLInterface::conv_forward_gemm_opencl(cl_mem device_y, const cl_mem device_x, const cl_mem device_k, const cl_mem device_x_unroll, const int B, const int M, const int C, const int H, const int W, const int K)
{
    //@@ ====== Start im2col =====
    // need to track errors
    cl_int err;
    // @@ define local and global work sizes
    size_t global_item_size[3] = {C * K * K, (H - K + 1) * (W - K + 1), 1};
    size_t local_item_size[3] = {TILE_SIZE, TILE_SIZE, 1};
    //@@ Launch the im2col kernel here
    err = clSetKernelArg(opencl->im2col_kernel, 0, sizeof(cl_mem), &device_x_unroll);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 1, sizeof(cl_mem), &device_x);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 2, sizeof(int), &B);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 3, sizeof(int), &C);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 4, sizeof(int), &H);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 5, sizeof(int), &W);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 6, sizeof(int), &K);
    CHECK_ERR(err, "clSetKernelArg 0");

    err = clEnqueueNDRangeKernel(
        opencl->queue,
        opencl->im2col_kernel,
        1,
        NULL,
        global_item_size,
        local_item_size,
        0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueNDRangeKernel");
    //@@ ====== End im2col =====

    //@@ ====== Start gemm =====
    // C (m x n) = A (m x k) * B (k x n)
    // size of unrolled is (B, C×K×K, (H − K + 1)×(W − K + 1))
    const size_t m = C*K*K;
    const size_t n = input1->shape[1];
    //from conv_cust.h -> size=channel_in*h_kernel*w_kernel*channel_out
    const size_t k = C * K * K * M;

    std::vector<size_t> k_offsets = std::vector<size_t>(B, 0);
    std::vector<size_t> x_offsets = std::vector<size_t>(B, 0);
    std::vector<size_t> y_offsets = std::vector<size_t>(B, 0);

    std::vector<float> alphas = std::vector<float>(B, 1);
    std::vector<float> betas = std::vector<float>(B, 0);

    // @@ Call clblast::GemmBatched here
    clblast::StatusCode clblast_err = clblast::GemmBatched(clblast::Layout::kRowMajor, clblast::Transpose::kNo, clblast::Transpose::kNo,
                                                           m, n, k,
                                                           alphas.data(),
                                                           device_x, x_offsets.data(), k,
                                                           device_k, k_offsets.data(), n,
                                                           betas.data(),
                                                           device_y, y_offsets.data(), n,
                                                           B,
                                                           opencl->queue, nullptr);
    CHECK_ERR((cl_int)clblast_err, "clblast::GemmBatched");

    //@@ ====== End gemm =====
}

void OpenCLInterface::conv_forward_gemm_opencl_epilog(float *host_y, cl_mem device_y, cl_mem device_x, cl_mem device_k, cl_mem device_x_unroll, const int B, const int M, const int C, const int H, const int W, const int K)
{
    //@@ Copy the output back to host

    //@@ Free the GPU memory here
}
