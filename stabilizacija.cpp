#include <iostream>
#include <cmath>
#include <vector>

struct KeyPoint {
    int x;
    int y;
    float angle;
};

extern "C" {
    void stabilizuj_frejm(unsigned char *data, int width, int height, int channels, int stride) {
        std::vector<unsigned char> gray(width*height);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int index=(y*stride)+(x*channels);

                unsigned char r=data[index]; // red
                unsigned char g=data[index + 1]; // Green
                unsigned char b=data[index + 2]; // Blue
                //Using int instead of float for faster calculations
                unsigned char y_val = (unsigned char)((114 * b + 587 * g + 299 * r) / 1000);

                int gray_idx = y * width + x;
                gray[gray_idx] = y_val;

                data[index]=y_val;
                data[index+1]=y_val;
                data[index+2]=y_val;
                /*if (x < width && y < height) {
                    int index = (y * stride) + (x * channels);
                    
                    data[index]     = 0; // Blue
                    data[index + 1] = 0; // Green
                    data[index + 2] = 0; // Red
                }*/

            }
        }
    }
}