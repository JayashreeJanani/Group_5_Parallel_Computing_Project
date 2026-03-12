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
