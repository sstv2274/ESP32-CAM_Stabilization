#include <iostream>
#include <cmath>
#include <vector>

//16 piksela u krugu
const int krug_x[16] = { 0,  1,  2,  3, 3, 3, 2, 1, 0, -1, -2, -3, -3, -3, -2, -1 };
const int krug_y[16] = {-3, -3, -2, -1, 0, 1, 2, 3, 3,  3,  2,  1,  0, -1, -2, -3 };


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
                //hardver nam daje sliku u ovom formatu pa ce tako i ostati
                unsigned char b=data[index]; // Blue
                unsigned char g=data[index + 1]; // Green
                unsigned char r=data[index + 2]; // Red
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