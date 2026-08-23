  __kernel void matrixMultiply(
    __global const int *A, __global const int *B, __global int *C,
    const unsigned int numARows, const unsigned int numAColumns,
    const unsigned int numBRows, const unsigned int numBColumns,
    const unsigned int numCRows, const unsigned int numCColumns,
    __local int *local_tile_mem_a, __local int *local_tile_mem_b,
    const int tile_dim) {
    //@@ Compute C = AB 

    int row = get_global_id(0);
    int col = get_global_id(1); 

    int local_row = get_local_id(0);
    int local_col = get_local_id(1);

    int local_tile_index = (local_row * tile_dim) + local_col;
    
    int index_c = row * numCColumns + col;
    C[index_c] = 0;
    int sum = 0;

    for(int t = 0; t < ((numAColumns + tile_dim - 1) / tile_dim); t++){

      int i_a = (row * numAColumns) + (t * tile_dim + local_col);
      int i_b = (t * tile_dim + local_row) * numBColumns + col;
      

      if (row < numARows && (t * tile_dim + local_col) < numAColumns) {
        local_tile_mem_a[local_tile_index] = A[i_a];
      } else {
        local_tile_mem_a[local_tile_index] = 0;
      }

      if ((t * tile_dim + local_row) < numBRows && col < numBColumns) {
        local_tile_mem_b[local_tile_index] = B[i_b];
      } else {
          local_tile_mem_b[local_tile_index] = 0;
      }

      barrier(CLK_LOCAL_MEM_FENCE);
      
      for(int k = 0; k < tile_dim; k++){
        sum+=local_tile_mem_a[local_row * tile_dim + k] * local_tile_mem_b[k * tile_dim + local_col];
      }

      barrier(CLK_LOCAL_MEM_FENCE);
    }

    if (row < numCRows && col < numCColumns) {
        C[index_c] = sum;
    }
  }
