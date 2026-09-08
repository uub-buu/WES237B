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

size_t buffer_size_dev_x;
size_t buffer_size_dev_y;
size_t buffer_size_dev_k;
size_t buffer_size_dev_unroll;

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
    buffer_size_dev_x = W * H * C * B * sizeof(float);
    buffer_size_dev_y = output_col * output_row * M * B * sizeof(float);
    buffer_size_dev_k = M * C * K * K * sizeof(float);
    buffer_size_dev_unroll = B * C * K * K * output_col * output_row * sizeof(float);

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
    //size_t global_item_size[3] = {(H - K + 1) * (W - K + 1), C * K * K, 1};
    size_t local_item_size[3] = {TILE_SIZE, TILE_SIZE, 1};

    auto round_up = [](size_t val, size_t tile) {
        return ((val + tile - 1) / tile) * tile;
    };

    size_t global_item_size[3] = {
        round_up((H - K + 1) * (W - K + 1), local_item_size[0]), // Dim 0: col_id
        round_up(C * K * K,     local_item_size[1]), // Dim 1: row_id
        round_up(B,             local_item_size[2])  // Dim 2: batch_id
    };
    //@@ Launch the im2col kernel here
    err = clSetKernelArg(opencl->im2col_kernel, 0, sizeof(cl_mem), &device_x_unroll);
    CHECK_ERR(err, "clSetKernelArg 0");
    err |= clSetKernelArg(opencl->im2col_kernel, 1, sizeof(cl_mem), &device_x);
    CHECK_ERR(err, "clSetKernelArg 1");
    err |= clSetKernelArg(opencl->im2col_kernel, 2, sizeof(int), &B);
    CHECK_ERR(err, "clSetKernelArg 2");
    err |= clSetKernelArg(opencl->im2col_kernel, 3, sizeof(int), &C);
    CHECK_ERR(err, "clSetKernelArg 3");
    err |= clSetKernelArg(opencl->im2col_kernel, 4, sizeof(int), &H);
    CHECK_ERR(err, "clSetKernelArg 4");
    err |= clSetKernelArg(opencl->im2col_kernel, 5, sizeof(int), &W);
    CHECK_ERR(err, "clSetKernelArg 5");
    err |= clSetKernelArg(opencl->im2col_kernel, 6, sizeof(int), &K);
    CHECK_ERR(err, "clSetKernelArg 6");

    err = clEnqueueNDRangeKernel(
        opencl->queue,
        opencl->im2col_kernel,
        3,
        NULL,
        global_item_size,
        local_item_size,
        0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueNDRangeKernel");
    //@@ ====== End im2col =====

    //@@ ====== Start gemm =====
    // C (m x n) = A (m x k) * B (k x n)
    // size of unrolled is (B, C×K×K, (H − K + 1)×(W − K + 1))
    // from conv_cust.h -> size=channel_in*h_kernel*w_kernel*channel_out
    const size_t m = M;
    const size_t n = (H - K + 1) * (W - K + 1);
    const size_t k = C * K * K;

    std::vector<size_t> k_offsets = std::vector<size_t>(B, 0);
    std::vector<size_t> x_offsets = std::vector<size_t>(B);
    std::vector<size_t> y_offsets = std::vector<size_t>(B);
    // okay so if B is the batches and our x and y offsets are our results for the batches then we need
    // to adjust the elements which represents the starting point of each batch. If B is 1 then this isnt needed
    for (int i = 0; i < B; ++i) {
        x_offsets[i] = i * (k * n);
        y_offsets[i] = i * (m * n);
    }
    std::vector<float> alphas = std::vector<float>(B, 1);
    std::vector<float> betas = std::vector<float>(B, 0);

    // @@ Call clblast::GemmBatched here
    clblast::StatusCode clblast_err = clblast::GemmBatched(clblast::Layout::kRowMajor,
                                                           clblast::Transpose::kNo, clblast::Transpose::kNo,
                                                           m, n, k,
                                                           alphas.data(),
                                                           device_k, k_offsets.data(), k,
                                                           device_x_unroll, x_offsets.data(), n,
                                                           betas.data(),
                                                           device_y, y_offsets.data(), n,
                                                           B,
                                                           &opencl->queue, nullptr);
    CHECK_ERR((cl_int)clblast_err, "clblast::GemmBatched");

    //@@ ====== End gemm =====
    clblast::ClearCache();

}

void OpenCLInterface::conv_forward_gemm_opencl_epilog(float *host_y, cl_mem device_y, cl_mem device_x, cl_mem device_k, cl_mem device_x_unroll, const int B, const int M, const int C, const int H, const int W, const int K)
{
    //@@ Copy the output back to host
    cl_int err;
    err = clEnqueueReadBuffer(
        opencl->queue,
        device_y,
        CL_TRUE,
        0,
        buffer_size_dev_y,
        host_y,
        0,
        NULL,
        NULL);
    CHECK_ERR(err, "clEnqueueReadBuffer");
    //@@ Free the GPU memory here
    clReleaseMemObject(device_x);
    clReleaseMemObject(device_y);
    clReleaseMemObject(device_k);
    clReleaseMemObject(device_x_unroll);
}
