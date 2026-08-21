#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"
#include <arm_neon.h>

#define CHECK_ERR(err, msg)                           \
    if (err != CL_SUCCESS)                            \
    {                                                 \
        fprintf(stderr, "%s failed: %d\n", msg, err); \
        exit(EXIT_FAILURE);                           \
    }

void NeonBlockMatrixMultiply(Matrix *input0, Matrix *input1, Matrix *result)
{
    //@@ Insert code to implement block matrix multiply with ARM Neon intrinsics here
    int block_size = 4; // calculated using the following: <totalMatrices> x (block_size x block_size) x <sizeof(int)> = RB3 L1 cache size
    int input0_height = input0->shape[0];
    int input0_width = input0->shape[1];
    //int input1_height = input1->shape[0];
    int input1_width = input1->shape[1];

    for (int b_i = 0; b_i < input0_height; b_i += block_size)
    {
        for (int b_j = 0; b_j < input1_width; b_j += block_size)
        {
            for (int b_k = 0; b_k < input0_width; b_k += block_size)
            {
                // Load block elements into 4x4 vectors
                int32x4_t b_col0 = vld1q_s32(&input1->data[b_j]);
                int32x4_t b_col1 = vld1q_s32(&input1->data[b_j+4]);
                int32x4_t b_col2 = vld1q_s32(&input1->data[b_j+8]);
                int32x4_t b_col3 = vld1q_s32(&input1->data[b_j+12]);

                /* THIS IS THE NAIVE MATRIX MULTIPLY APPROACH*/
                for (int i = b_i; i < ((b_i + block_size < input0_height) ? (b_i + block_size) : input0_height); i+=4)
                {
                    // for (int j = b_j; j < ((b_j + block_size < input1_width) ? (b_j + block_size) : input1_width); j++)
                    // {
                        /* load the first 4 32-bit integers*/
                        int32x4_t a_row = vld1q_s32(&input0->data[i]);

                        for (int k = b_k; k < ((b_k + block_size < input0_width) ? (b_k + block_size) : input0_width); k++)
                        {
                            
                        }
                    // }
                }
            }
        }
    }
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
    //initialize ouput data to zeroes
    memset(host_c.data , 0, sizeof(int) * host_c.shape[0] * host_c.shape[1]); 

    // Call your matrix multiply.
    NeonBlockMatrixMultiply(&host_a, &host_b, &host_c);

    // // Call to print the matrix
    // PrintMatrix(&host_c);

    // Save the matrix
    SaveMatrix(input_file_d, &host_c);

    // Check the result of the matrix multiply
    err = CheckMatrix(&answer, &host_c);
    CHECK_ERR(err, "CheckMatrix");

    // Release host memory
    free(host_a.data);
    free(host_b.data);
    free(host_c.data);
    free(answer.data);

    return 0;
}