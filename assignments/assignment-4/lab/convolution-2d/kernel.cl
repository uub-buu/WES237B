
__kernel void convolution2D(
    __global int * inputData, __global int * outputData, __constant int * maskData,
    int width, int height, int maskWidth,  int imageChannels, int stride){
    //@@ Insert code to implement matrix multiplication here
    int j = get_global_id(0); // x
    int i = get_global_id(1); // y

    int out_width = width - (maskWidth - 1);
    int out_height = height - (maskWidth - 1);

    // VALID padding bounds check
    if (j < out_width && i < out_height) {
        for (int k = 0; k < imageChannels; k++) {
            int accum = 0;
            
            for (int y = 0; y < maskWidth; y++) {
                for (int x = 0; x < maskWidth; x++) {
                    int in_index = ((i + y) * width + (j + x)) * imageChannels + k;
                    int mask_index = y * maskWidth + x;
                    
                    accum += inputData[in_index] * maskData[mask_index];
                }
            }

            int out_index = (i * out_width + j) * imageChannels + k;
            outputData[out_index] = accum;
        }
    }

}