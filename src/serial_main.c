#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

int main(int argc, char** argv) {
  // Check for input image argument, if not provided, print usage and exit
  if (argc < 2) {
    printf("Usage: %s <input_image>\n", argv[0]);
    return 1;
  }

  int w, h, channels;

  // Loading the input image using stb_image
  unsigned char* input_image = stbi_load(argv[1], &w, &h, &channels, 0);

  // Buffers for pipeline stages
  unsigned char *gray_image = malloc(w * h);
  unsigned char *blurred_image = malloc(w * h);
  unsigned char *final = malloc(w * h);

  // Timing the serial execution of the pipeline
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // Running pipeline stages sequentially
  grayscale_filter(input_image, gray_image, w, h, channels, false);
  blur_filter(gray_image, blurred_image, w, h, false);
  sobel_filter(blurred_image, final, w, h, false);

  clock_gettime(CLOCK_MONOTONIC, &end);

  // Calculating and printing the execution time for the serial pipeline
  // The execution time is calculated by taking the difference between the start and end times, converting it to seconds.
  double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Serial execution time: %f seconds\n", time);

  // Saving the final output image using stb_image_write
  stbi_write_jpg("data/output_serial/test_output_serial.jpg", w, h, 1, final, 90);

  // Cleanup
  free(gray_image);
  free(blurred_image);
  free(final);
  stbi_image_free(input_image);

  return 0;
}