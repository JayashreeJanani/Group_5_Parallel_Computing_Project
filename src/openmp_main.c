#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <stdbool.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <input_image>\n", argv[0]);
        return 1;
    }

    int w, h, channels;
    unsigned char* input_image = stbi_load(argv[1], &w, &h, &channels, 0);

    unsigned char *gray_image = malloc(w * h);
    unsigned char *blurred_image = malloc(w * h);
    unsigned char *final = malloc(w * h);

    // High-resolution timing
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Parallel Pipeline Execution
    grayscale_filter(input_image, gray_image, w, h, channels, true);
    blur_filter(gray_image, blurred_image, w, h, true);
    sobel_filter(blurred_image, final, w, h, true);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("OpenMP execution time (%d threads): %f seconds\n", omp_get_max_threads(), time);

    stbi_write_jpg("data/output_openmp/test_output_openmp.jpg", w, h, 1, final, 90);

    // Cleanup
    free(gray_image);
    free(blurred_image);
    free(final);
    stbi_image_free(input_image);

    return 0;
}