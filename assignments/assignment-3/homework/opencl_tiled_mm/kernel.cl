__kernel void matrixMultiply(
    __global const int *A, __global const int *B, __global int *C,
    const unsigned int numARows, const unsigned int numAColumns,
    const unsigned int numBRows, const unsigned int numBColumns,
    const unsigned int numCRows, const unsigned int numCColumns,
    __local int *local_tile_mem_a,
    __local int *local_tile_mem_b,
    const size_t *tile_size) {
  //@@ Compute C = AB 

    int row = get_global_id(0);
    int col = get_global_id(1); 

    int local_row = get_local_id(0);
    int local_col = get_local_id(1);

    int index_c = row * numCColumns + col;
    C[index_c] = 0;
    // need to loop over the tiles first
    
    if(row<numCRows && col < numCColumns){
      for(int k = 0; k < numAColumns; k++){
        int i_a = (row * numAColumns) + k;
        int i_b = (k * numBColumns) + col;
        C[index_c]+=A[i_a]*B[i_b];
      }
    }
}
