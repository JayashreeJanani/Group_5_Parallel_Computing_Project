#include "../include/filters.h"
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <omp.h>

/*
  Grayscale filter implementation (RGB to Grayscale)

  The grayscale filter converts a color image to grayscale by applying a weighted average of the red, green, and blue channels. 
  The weights are based on the perceived brightness of each color: 
  - Red contributes 29.9% (0.299) to the final intensity
  - Green contributes 58.7% (0.587) to the final intensity
  - Blue contributes 11.4% (0.114) to the final intensity

  
*/
void grayscale_filter(unsigned char* input, unsigned char* output, int width, int height, int channels, bool is_openmp) {
  // Parallelizing the loop using OpenMP if is_openmp is true, otherwise it will run sequentially
  #pragma omp parallel for schedule(static) if(is_openmp)
  // Iterating over each pixel in the input image
  for (int i = 0; i < width * height; i++) {
    // Extracting the RGB values from the input image using the channels information
    int r = input[i * channels];
    int g = input[i * channels + 1];
    int b = input[i * channels + 2];

    // Applying the grayscale formula and storing the result in the output buffer
    output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
  }
}


/*
  Gaussian blur filter implementation

  The Gaussian blur filter applies a convolution operation to the input image using a 3x3 kernel that approximates a Gaussian distribution. 
  The kernel is defined as follows:

    [1/16, 2/16, 1/16]
    [2/16, 4/16, 2/16]
    [1/16, 2/16, 1/16]

  Each pixel in the output image is computed by taking the weighted sum of the neighboring pixels in the input image, where the weights are determined by the kernel. 
  This results in a blurred version of the original image, which helps to reduce noise and detail.
*/
void blur_filter(unsigned char* input, unsigned char* output, int width, int height, bool is_openmp) {
  float kernel[3][3] = {
        {1/16.0, 2/16.0, 1/16.0},
        {2/16.0, 4/16.0, 2/16.0},
        {1/16.0, 2/16.0, 1/16.0}
    };
 
  // Iterating over each pixel in the input image, skipping the borders to avoid out-of-bounds access
  #pragma omp parallel for schedule(static) if(is_openmp)
  for (int y = 1; y < height - 1; y++) {
      for (int x = 1; x < width - 1; x++) {
          float sum = 0.0;
          // Applying the convolution operation using the kernel
          for (int ky = -1; ky <= 1; ky++) {
              for (int kx = -1; kx <= 1; kx++) {
                  sum += input[IX(x + kx, y + ky, width)] * kernel[ky + 1][kx + 1];
              }
          }
          // Storing the computed value in the output buffer
          output[IX(x, y, width)] = (unsigned char)sum;
      }
  }
}


/*  
  Sobel edge detection filter implementation

  The Sobel edge detection filter applies two separate convolution operations to the input image using two 3x3 kernels, Gx and Gy, which are designed to detect horizontal and vertical edges, respectively. 
  The kernels are defined as follows:

    Gx:
    [-1, 0, 1]
    [-2, 0, 2]
    [-1, 0, 1]

    Gy:
    [1, 2, 1]
    [0, 0, 0]
    [-1, -2, -1]

  Each pixel in the output image is computed by taking the magnitude of the gradient vector formed by the results of the two convolutions (Gx and Gy). 
  This results in an image that highlights the edges present in the original image.
*/
void sobel_filter(unsigned char* input, unsigned char* output, int width, int height, bool is_openmp) {
  // Sobel kernels for horizontal (Gx) and vertical (Gy) edge detection
    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
 
    // Iterating over each pixel in the input image, skipping the borders to avoid out-of-bounds access
    #pragma omp parallel for schedule(static) if(is_openmp)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float sumX = 0, sumY = 0;
            // Applying the convolution operation using the Gx and Gy kernels
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int val = input[IX(x + kx, y + ky, width)];
                    sumX += val * Gx[ky + 1][kx + 1];
                    sumY += val * Gy[ky + 1][kx + 1];
                }
            }
            // Calculating the magnitude of the gradient vector and storing the result in the output buffer
            int mag = (int)sqrt(sumX * sumX + sumY * sumY);
            output[IX(x, y, width)] = (mag > 255) ? 255 : mag;
        }
    }
  }