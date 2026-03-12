#include "../include/filters.h"
#include <math.h>
#include <stdlib.h>

// Grayscale filter implementation (RGB to Grayscale)
void grayscale_serial(unsigned char* input, unsigned char* output, int width, int height, int channels) {
  for (int i = 0; i < width * height; i++) {
    int r = input[i * channels];
    int g = input[i * channels + 1];
    int b = input[i * channels + 2];
    output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
  }
}
// Gaussian blur filter implementation (3x3 kernel)
void blur_serial(unsigned char* input, unsigned char* output, int width, int height) {
  float kernel[3][3] = {
        {1/16.0, 2/16.0, 1/16.0},
        {2/16.0, 4/16.0, 2/16.0},
        {1/16.0, 2/16.0, 1/16.0}
    };
 
  for (int y = 1; y < height - 1; y++) {
      for (int x = 1; x < width - 1; x++) {
          float sum = 0.0;
          for (int ky = -1; ky <= 1; ky++) {
              for (int kx = -1; kx <= 1; kx++) {
                  sum += input[IX(x + kx, y + ky, width)] * kernel[ky + 1][kx + 1];
              }
          }
          output[IX(x, y, width)] = (unsigned char)sum;
      }
  }
}