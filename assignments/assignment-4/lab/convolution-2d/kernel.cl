
__kernel void convolution2D(
    __global int * inputData, __global int * outputData, __constant int * maskData,
    int width, int height, int maskWidth,  int imageChannels, int stride){
    //@@ Insert code to implement matrix multiplication here
    int col = get_global_id(0); //col
    int row = get_global_id(1); //row

    int out_width = (width - maskWidth)/stride + 1;
    int out_height = (height - maskWidth)/stride + 1;

    if (col < out_width && row < out_height) {
        for (int k = 0; k < imageChannels; k++) {
            int accum = 0;
            
            for (int y = 0; y < maskWidth; y++) {
                for (int x = 0; x < maskWidth; x++) {
                    int in_y = row * stride + y;
                    int in_x = col * stride + x;
                    int in_index = (in_y * width + in_x) * imageChannels + k;
                    int mask_index = y * maskWidth + x;
                    
                    accum += inputData[in_index] * maskData[mask_index];
                }
            }

            int out_index = (row * out_width + col) * imageChannels + k;
            outputData[out_index] = accum;
        }
    }

}
