#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <math.h>

// Parallel Grayscale: Independent pixels allow for high efficiency
void grayscale_openmp(unsigned char* input, unsigned char* output, int width, int height, int channels) {
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < width * height; i++) {
        int r = input[i * channels];
        int g = input[i * channels + 1];
        int b = input[i * channels + 2];
        output[i] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
    }
}

// Parallel Gaussian Blur: Uses row-based decomposition
void blur_openmp(unsigned char* input, unsigned char* output, int width, int height) {
    float kernel[3][3] = {
        {1/16.0, 2/16.0, 1/16.0},
        {2/16.0, 4/16.0, 2/16.0},
        {1/16.0, 2/16.0, 1/16.0}
    };

    #pragma omp parallel for schedule(static)
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

// Parallel Sobel: Optimization using static scheduling for balanced workloads
void sobel_openmp(unsigned char* input, unsigned char* output, int width, int height) {
    int Gx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int Gy[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};

    #pragma omp parallel for schedule(static)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            float sumX = 0, sumY = 0;
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int val = input[IX(x + kx, y + ky, width)];
                    sumX += val * Gx[ky + 1][kx + 1];
                    sumY += val * Gy[ky + 1][kx + 1];
                }
            }
            int mag = (int)sqrt(sumX * sumX + sumY * sumY);
            output[IX(x, y, width)] = (mag > 255) ? 255 : mag;
        }
    }
}

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
    grayscale_openmp(input_image, gray_image, w, h, channels);
    blur_openmp(gray_image, blurred_image, w, h);
    sobel_openmp(blurred_image, final, w, h);

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