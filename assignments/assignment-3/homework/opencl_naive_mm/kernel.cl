__kernel void matrixMultiply(
    __global const int *A, __global const int *B, __global int *C,
    const unsigned int numARows, const unsigned int numAColumns,
    const unsigned int numBRows, const unsigned int numBColumns,
    const unsigned int numCRows, const unsigned int numCColumns) {
  //@@ Compute C = AB 
    int row = get_global_id(0);
    int col = get_global_id(1);
    if(row<numCRows && col < numCColumns){
      int index_c = row * numCColumns + col;
      C[index_c] = 0;
      for(int k = 0; k < numAColumns; k++){
        int i_a = (row * numAColumns) + k;
        int i_b = (k * numBColumns) + col;
        C[index_c]+=A[i_a]*B[i_b];
      }
    }

  }
