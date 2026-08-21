__kernel void matrixMultiply(
    __global const int *A, __global const int *B, __global int *C,
    const unsigned int numARows, const unsigned int numAColumns,
    const unsigned int numBRows, const unsigned int numBColumns,
    const unsigned int numCRows, const unsigned int numCColumns) {
  //@@ Compute C = AB 
    for(int i = 0; i < input0->shape[0]; i++){
        for(int j = 0; j < input1->shape[1]; j++){
            int index_r = (i * input1->shape[1]) + j;
            result->data[index_r] = 0;
            for(int k = 0; k < input0->shape[1];k++){
                    int index_0 = (i*input0->shape[1]) + k;
                    int index_1 = (k*input1->shape[1]) + j;
                    result->data[index_r] += input0->data[index_0]*input1->data[index_1];
            }
        }
    }
  }
