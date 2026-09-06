#define TILE_WIDTH 16
#define KERNEL_SZ 7

// default implementation
__kernel void im2col(__global float *unrolled, __global float *x, const int B,
                     const int C_in, const int H, const int W, const int K) {

int col_id = get_global_id(0);
int row_id = get_global_id(1); 
int batch_id = get_global_id(2);

int H_out = H - K + 1;
int W_out = W - K + 1;

if (col_id >= H_out * W_out || row_id >= C_in * K * K || batch_id >= B) return;
//output spatial dimension. This will be the size of our output image
// since our unroll dimensions include C*K*K we can find the channel with the following:
int channel_id = row_id / (K * K);
// kernel slot is where the input will be relative to its sliding window
int kernel_slot = row_id % (K* K);
// offsets of the input with respect to its slot position
int k_row_offset = kernel_slot / K;
int k_col_offset = kernel_slot % K;
// we can find the starting coordinates  of our kernel window on x with the following
int row_start_x = col_id / W_out;
int col_start_x = col_id % W_out;

int x_in_row = row_start_x + k_row_offset;
int x_in_col = col_start_x + k_col_offset;
// calculate the batch and channel offset of the input
int batch_offset_x = batch_id * (C_in * H * W);
int channel_offset_x = (channel_id * (H * W));

int index_i =  batch_offset_x + channel_offset_x + (x_in_row * W) + x_in_col;

int batch_offset = batch_id * (C_in * K * K * H_out * W_out);
int index_o = batch_offset + (row_id * H_out * W_out) + col_id;

unrolled[index_o] = x[index_i];
}
