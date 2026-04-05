#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <dirent.h>

int main(int argc, char** argv) {
  // Check for input directory argument, if not provided, print usage and exit
  if (argc < 2) {
    printf("Usage: %s <input_directory>\n", argv[0]);
    return 1;
  }

  // Open the input directory to read image files
  DIR *dir;
  struct dirent *ent; // Structure to hold directory entries

  if ((dir = opendir(argv[1])) != NULL) {
    // Start total timing
    struct timespec total_start, total_end;
    clock_gettime(CLOCK_MONOTONIC, &total_start);

    int iteration = 0; // Variable to track the iteration number
    while ((ent = readdir(dir)) != NULL) {
      // Skip the current and parent directory entries
      if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

      // Construct the full path to the input image
      char input_path[512];
      snprintf(input_path, sizeof(input_path), "%s/%s", argv[1], ent->d_name);

      // Load the input image using stb_image
      int w, h, channels;
      unsigned char* input_image = stbi_load(input_path, &w, &h, &channels, 0);
      if (!input_image) {
        fprintf(stderr, "Error: Could not load image %s\n", input_path);
        continue; // Skip to the next file if loading fails
      }

      // Buffers for pipeline stages
      unsigned char *gray_image = malloc(w * h);
      unsigned char *blurred_image = malloc(w * h);
      unsigned char *final = malloc(w * h);

      // Timing the serial execution of the pipeline for the current image
      struct timespec start, end;
      clock_gettime(CLOCK_MONOTONIC, &start);

      // Running pipeline stages sequentially
      grayscale_filter(input_image, gray_image, w, h, channels, false);
      blur_filter(gray_image, blurred_image, w, h, false);
      sobel_filter(blurred_image, final, w, h, false);

      clock_gettime(CLOCK_MONOTONIC, &end);

      // Calculating and printing the execution time for the current image
      // double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

      // Print the iteration number
      // printf("%d: Processed %s in %f seconds\n", iteration, ent->d_name, time);
      iteration++;

      // Construct the output path by replacing "input" with "output" in the filename
      char output_path[512];
      snprintf(output_path, sizeof(output_path), "data/output_serial/%s", ent->d_name);

      // Save the final output image using stb_image_write
      stbi_write_jpg(output_path, w, h, 1, final, 90);

      // Cleanup for the current image
      free(gray_image);
      free(blurred_image);
      free(final);
      stbi_image_free(input_image);
    }
    closedir(dir);

    // End total timing and print the total execution time for processing all images
    clock_gettime(CLOCK_MONOTONIC, &total_end);
    double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
    printf("SERIAL BASELINE: Total execution time for all images: %f seconds\n", total_time);
  } else {
    // If the directory cannot be opened, print an error message and exit
    fprintf(stderr, "Error: Could not open directory %s\n", argv[1]);
    return 1;
  }

    return 0;
}