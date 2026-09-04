#define TILE_WIDTH 16
#define KERNEL_SZ 7

// default implementation
__kernel void im2col(__global float *unrolled, __global float *x, const int B,
                     const int C_in, const int H, const int W, const int K) {

#define x4d(i3, i2, i1, i0)                                                    \
  x[(i3) * (C_in * H * W) + (i2) * (H * W) + (i1) * (W) + i0]
  // `unrolled` is a (B, H_unroll, W_unroll) tensor
#define x_unroll_3d(i2, i1, i0)                                                \
  unrolled[((i2) * H_unroll + (i1)) * W_unroll + (i0)]
int row = get_global_id(1);
int col = get_global_id(0);
// spacial output dimensions for our output image.
int outH = H - K + 1;
int outW = W - K + 1;
// since padding is zero we are going to slide everything 
//@@ Define your im2col operations here.

/* Pseudo code*/
/*
//we need our input data in a flatten array
// we will assume 1 channel 1 batch for now
  [x(0) -> x(K-1), x(K) -> 2K - 1, x(2K) -> x(3K-1), .... ]
  

*/
//main loop for receptive field
for(int xrow = 0; xrow < H; xrow++){
  for(int xcol = 0; xcol < W; xcol++){
    for(int k = 0; k < K; k++){
      int offset_x = xcol + k;
    }
  }
}



for b in 0..<B:
    for c_in in 0..<C:
        for row_i in 0..<H:
            for col_i in 0..<W:
                for mask_offset_row in 0..<K:
                    for mask_offset_col in 0..<K:
                        # indices in the output of the convolution whose receptive fields include (row_i, col_i)
                        row_o = ??
                        col_o = ??
                        row_o_in_bounds = 0 <= row_o and row_o < H - K + 1
                        col_o_in_bounds = 0 <= col_o and col_o < W - K + 1
                        if row_o_in_bounds and col_o_in_bounds:
                            # indices in x_unroll to write to
                            col_u = ??
                            row_u = ??

                            x_unroll[b, row_u, col_u] = x[b, c_in, row_i, col_i]
#undef x4d
#undef x_unroll_3d
}
