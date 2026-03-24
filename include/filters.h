// filters.h
// This header file declares the functions for the image processing filters used in the project.
#include <stdbool.h>
#ifndef FILTERS_H
#define FILTERS_H
 
// Macro to calculate the index in a 1D array for a 2D image
// The image is stored in row-major order, so the index for pixel (x, y) in an image of width w is calculated as:
#define IX(x, y, w) ((y) * (w) + (x))
 
// Function prototypes for the filters, implemented in filters.c
void grayscale_filter(unsigned char* input, unsigned char* output, int width, int height, int channels, bool is_openmp);
void blur_filter(unsigned char* input, unsigned char* output, int width, int height, bool is_openmp);
void sobel_filter(unsigned char* input, unsigned char* output, int width, int height, bool is_openmp);
 
#endif
