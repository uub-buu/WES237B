#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <clblast.h>

#include "device.h"
#include "kernel.h"
#include "matrix.h"

#define CHECK_ERR(err, msg)                           \
    if (err != CL_SUCCESS)                            \
    {                                                 \
        fprintf(stderr, "%s failed: %d\n", msg, err); \
        exit(EXIT_FAILURE);                           \
    }

void CopyMatrix(const Matrix *input, float *output)
{
    for (unsigned int i = 0; i < input->shape[0] * input->shape[1]; ++i)
    {
        output[i] = (float)input->data[i];
    }
}

void OpenCLMatrixMultiply(Matrix *input0, Matrix *input1, Matrix *result)
{
    // We need to convert the int data to float data for CLBlast
    float *h_A = (float *)malloc(input0->shape[0] * input0->shape[1] * sizeof(float));
    float *h_B = (float *)malloc(input1->shape[0] * input1->shape[1] * sizeof(float));
    float *h_C = (float *)malloc(result->shape[0] * result->shape[1] * sizeof(float));

    CopyMatrix(input0, h_A);
    CopyMatrix(input1, h_B);

    // Device input and output buffers
    cl_mem device_a, device_b, device_c;

    cl_int err;

    cl_device_id device_id; // device ID
    cl_context context;     // context
    cl_command_queue queue; // command queue

    // Find platforms and devices
    OclPlatformProp *platforms = NULL;
    cl_uint num_platforms;

    err = OclFindPlatforms((const OclPlatformProp **)&platforms, &num_platforms);
    CHECK_ERR(err, "OclFindPlatforms");

    err = OclGetDeviceWithFallback(&device_id, OCL_DEVICE_TYPE);
    CHECK_ERR(err, "OclGetDeviceWithFallback");

    // Create a context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    CHECK_ERR(err, "clCreateContext");

    // Create a command queue
#if __APPLE__
    queue = clCreateCommandQueue(context, device_id, 0, &err);
#else
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
#endif
    CHECK_ERR(err, "clCreateCommandQueueWithProperties");

    // Define the batch size
    const int B = 1;

    //@@ Allocate GPU memory here
    size_t buffer_size_dev_a = input0->shape[0] * input0->shape[1] * sizeof(float);
    size_t buffer_size_dev_b = input1->shape[0] * input1->shape[1] * sizeof(float);
    size_t buffer_size_dev_c = result->shape[0] * result->shape[1] * sizeof(float);
    device_a = clCreateBuffer(context,
                              CL_MEM_READ_ONLY,
                              buffer_size_dev_a,
                              NULL,
                              &err);
    CHECK_ERR(err, "clCreateBuffer input0");
    device_b = clCreateBuffer(context,
                              CL_MEM_READ_ONLY,
                              buffer_size_dev_b,
                              NULL,
                              &err);
    CHECK_ERR(err, "clCreateBuffer input1");
    device_c = clCreateBuffer(context,
                              CL_MEM_READ_WRITE,
                              buffer_size_dev_c,
                              NULL,
                              &err);
    CHECK_ERR(err, "clCreateBuffer result");

    //@@ Copy memory to the GPU here
    err = clEnqueueWriteBuffer(queue, device_a, CL_TRUE, 0, buffer_size_dev_a, input0->data, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer input0");
    err |= clEnqueueWriteBuffer(queue, device_b, CL_TRUE, 0, buffer_size_dev_b, input1->data, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer input1");
    err |= clEnqueueWriteBuffer(queue, device_c, CL_TRUE, 0, buffer_size_dev_c, result->data, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueWriteBuffer result");
    //@@ Call GEMM here
    // C (m x n) = A (m x k) * B (k x n)
    const size_t m = input0->shape[0];
    const size_t n = input1->shape[1];
    const size_t k = input0->shape[1];

    std::vector<size_t> a_offsets = std::vector<size_t>(B, 0);
    std::vector<size_t> b_offsets = std::vector<size_t>(B, 0);
    std::vector<size_t> c_offsets = std::vector<size_t>(B, 0);

    std::vector<float> alphas = std::vector<float>(B, 1);
    std::vector<float> betas = std::vector<float>(B, 0);

    clblast::StatusCode clblast_err = clblast::GemmBatched(clblast::Layout::kRowMajor, clblast::Transpose::kNo, clblast::Transpose::kNo,
                                                           m, n, k,
                                                           alphas.data(),
                                                           device_a, a_offsets.data(), k,
                                                           device_b, b_offsets.data(), n,
                                                           betas.data(),
                                                           device_c, c_offsets.data(), n,
                                                           B,
                                                           &queue, nullptr);
    CHECK_ERR((cl_int)clblast_err, "clblast::GemmBatched");

    //@@ Copy the GPU memory back to the CPU here
    err = clEnqueueReadBuffer(queue, device_c, CL_TRUE, 0, result->shape[0] * result->shape[1] * sizeof(float), h_C, 0, NULL, NULL);
    CHECK_ERR(err, "clEnqueueReadBuffer device_c");

    //@@ Free the GPU memory here
    clReleaseMemObject(device_a);
    clReleaseMemObject(device_b);
    clReleaseMemObject(device_c);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    // Copy back from h_C to result
    for (unsigned int i = 0; i < result->shape[0] * result->shape[1]; ++i)
    {
        result->data[i] = (int)h_C[i];
    }

    // Release the malloc'd memory
    free(h_A);
    free(h_B);
    free(h_C);
}

int main(int argc, char *argv[])
{
    if (argc != 5)
    {
        fprintf(stderr, "Usage: %s <input_file_0> <input_file_1> <answer_file> <output_file>\n", argv[0]);
        return -1;
    }

    const char *input_file_a = argv[1];
    const char *input_file_b = argv[2];
    const char *input_file_c = argv[3];
    const char *input_file_d = argv[4];

    // Host input and output vectors and sizes
    Matrix host_a, host_b, host_c, answer;

    cl_int err;

    err = LoadMatrix(input_file_a, &host_a);
    CHECK_ERR(err, "LoadMatrix");

    err = LoadMatrix(input_file_b, &host_b);
    CHECK_ERR(err, "LoadMatrix");

    err = LoadMatrix(input_file_c, &answer);
    CHECK_ERR(err, "LoadMatrix");

    int rows, cols;
    //@@ Update these values for the output rows and cols of the output
    //@@ Do not use the results from the answer matrix
    rows = host_a.shape[0];
    cols = host_b.shape[1];
    // Allocate the memory for the target.
    host_c.shape[0] = rows;
    host_c.shape[1] = cols;
    host_c.data = (int *)malloc(sizeof(int) * host_c.shape[0] * host_c.shape[1]);
    memset(host_c.data, 0, sizeof(int) * host_c.shape[0] * host_c.shape[1]);
    // Call your matrix multiply.
    OpenCLMatrixMultiply(&host_a, &host_b, &host_c);

    // // Call to print the matrix
    // PrintMatrix(&host_c);

    // Save the matrix
    SaveMatrix(input_file_d, &host_c);

    // Check the result of the matrix multiply
    CheckMatrix(&answer, &host_c);

    // Release host memory
    free(host_a.data);
    free(host_b.data);
    free(host_c.data);
    free(answer.data);

    return 0;
}
