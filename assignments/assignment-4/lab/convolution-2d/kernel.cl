
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
            outputData[out_index] = accum; // Apply clamp(accum, 0, 255) if needed
        }
    }
    /**
    maskWidth := 5
    maskRadius := maskWidth/2 # this is integer division, so the result is 2
    for i from 0 to height do
        for j from 0 to width do
            for k from 0 to channels
                accum := 0
                for y from -maskRadius to maskRadius do
                    for x from -maskRadius to maskRadius do
                        xOffset := j + x
                        yOffset := i + y
                        if xOffset >= 0 && xOffset < width &&
                            yOffset >= 0 && yOffset < height then
                            imagePixel := I[(yOffset * width + xOffset) * channels + k]
                            maskValue := K[(y+maskRadius)*maskWidth+x+maskRadius]
                            accum += imagePixel * maskValue
                        end
                    end
                end
                # pixels are in the range of 0 to 1
                P[(i * width + j)*channels + k] = clamp(accum, 0, 1)
            end
        end
    end**/
}
