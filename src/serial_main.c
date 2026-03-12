#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv) {
  // Check for input image argument, if not provided, print usage and exit
  if (argc < 2) {
    printf("Usage: %s <input_image>\n", argv[0]);
    return 1;
  }

  int w, h, channels;

  // Load the input image using stb_image
  unsigned char* input_image = stbi_load(argv[1], &w, &h, &channels, 0);

  // Buffers for pipeline stages
  unsigned char *gray_image = malloc(w * h);
  unsigned char *blurred_image = malloc(w * h);
  unsigned char *final = malloc(w * h);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  // Run pipeline stages sequentially
  grayscale_serial(input_image, gray_image, w, h, channels);
  blur_serial(gray_image, blurred_image, w, h);
  sobel_serial(blurred_image, final, w, h);

  clock_gettime(CLOCK_MONOTONIC, &end);
  double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Serial execution time: %f seconds\n", time);

  stbi_write_jpg("data/output_serial/test_output_serial.jpg", w, h, 1, final, 90);

  // Cleanup
  free(gray_image);
  free(blurred_image);
  free(final);
  stbi_image_free(input_image);

  return 0;
}