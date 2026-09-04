
__kernel void convolution2D(
    __global int * inputData, __global int * outputData, __constant int * maskData,
    int width, int height, int maskWidth,  int imageChannels, int stride){
    //@@ Insert code to implement matrix multiplication here
    int j = get_global_id(0);
    int i = get_global_id(1);

    int maskRadius = maskWidth / 2;

    if (j < height && i < width) {
        for (int k = 0; k < imageChannels; k++)
        {
            int accum = 0;
            for (int y = -maskRadius; y <= maskRadius; y++) {
                for (int x = -maskRadius; x <= maskRadius; x++) {
                    int xOffset = i + x;
                    int yOffset = j + y;
                    if (xOffset > -1 && xOffset < width && yOffset > -1 && yOffset < height) {
                        int imagePixel = inputData[(yOffset * width + xOffset) * imageChannels + k];
                        int maskValue = maskData[(y+maskRadius) * maskWidth+x+maskRadius];
                        accum += imagePixel * maskValue;
                    }
                }
            }
            outputData[(j*width + i) * imageChannels + k] = clamp(accum, 0, 1);
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
