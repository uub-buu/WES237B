
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "matrix.h"

#define CHECK_ERR(err, msg)                           \
    if (err != CL_SUCCESS)                            \
    {                                                 \
        fprintf(stderr, "%s failed: %d\n", msg, err); \
        exit(EXIT_FAILURE);                           \
    }

void blockMatrixMultiply(Matrix *input0, Matrix *input1, Matrix *result)
{
    // answer for matrix result is stored in data array.
    // starting from first row first column -> first row, last column -> second row, first column. until we get to last row last column
    //@@ Insert code to implement naive matrix multiply here
    int block_size = 64; // calculated using the following: <totalMatrices> x (block_size x block_size) x <sizeof(int)> = RB3 L1 cache size
    int input0_height = input0->shape[0];
    int input0_width = input0->shape[1];
    int input1_height = input1->shape[0];
    int input1_width = input1->shape[1];
    for (int b_i = 0; b_i < input0_height; b_i += block_size)
    {
        for (int b_j = 0; b_j < input1_width; b_j += block_size)
        {
            for (int b_k = 0; b_k < input0_width; b_k += block_size)
            {
                /* THIS IS THE NAIVE MATRIX MULTIPLY APPROACH*/
                for (int ii = b_i; ii < ((b_i + block_size < input0_height) ? (b_i + block_size) : input0_height); ii++)
                {
                    for (int jj = b_j; jj < ((b_j + block_size < input1_width) ? (b_j + block_size) : input1_width); jj++)
                    {
                        int temp = 0;
                        for (int kk = b_k; kk < ((b_k + block_size < input0_width) ? (b_k + block_size) : input0_width); kk++)
                        {

                            int index_0 = (ii * input0->shape[1]) + kk;
                            int index_1 = (kk * input1->shape[1]) + jj;
                            temp += input0->data[index_0] * input1->data[index_1];
                        }
                        int index_r = (ii * input1->shape[1]) + jj;
                        result->data[index_r] += temp;
                    }
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

    // Call your matrix multiply.
    blockMatrixMultiply(&host_a, &host_b, &host_c);

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
