#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) printf("Usage: mpirun -np <n> %s <input_directory>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    DIR *dir = NULL;
    struct  dirent *ent = NULL;
    struct timespec total_start, total_end;

    // Only the root process will read the directory and load the image, then broadcast the dimensions to all processes
    if (rank == 0) {
      dir = opendir(argv[1]);
      if (!dir) {
          fprintf(stderr, "Error: Could not open directory %s\n", argv[1]);
          MPI_Abort(MPI_COMM_WORLD, 1);
      }
      clock_gettime(CLOCK_MONOTONIC, &total_start);
    }

    int iteration = 0; 

    while (1) {
      int continue_processing = 0; // Flag to indicate if there are more images to process
      char filename[512] = {0}; // Buffer to hold the filename of the current image
      char input_path[512] = {0}; // Buffer to hold the full path to the current image

      if (rank == 0) {
        while ((ent = readdir(dir)) != NULL) {
          if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;

          // Skip .gitkeep and other non-images
          if (!strstr(ent->d_name, ".jpg") && !strstr(ent->d_name, ".jpeg") && !strstr(ent->d_name, ".png")) continue; 
          
          strncpy(filename, ent->d_name, sizeof(filename) - 1);
          snprintf(input_path, sizeof(input_path), "%s/%s", argv[1], filename);
          continue_processing = 1; // Set flag to indicate that we have an image to process
          break; // Break after finding the first valid image
        }
      }

      // Broadcast the continue_processing flag to all processes
      MPI_Bcast(&continue_processing, 1, MPI_INT, 0, MPI_COMM_WORLD);
      if (!continue_processing) break; // If no more images to process, exit the loop

      // Broadcast the filename so Rank 0 can use it for saving later
      MPI_Bcast(filename, 512, MPI_CHAR, 0, MPI_COMM_WORLD);

      int w, h, channels;
      unsigned char* full_input = NULL;
      unsigned char* full_output = NULL;

      struct timespec start, end; // Variables for timing the processing of each image

      if (rank == 0) {
        full_input = stbi_load(input_path, &w, &h, &channels, 0);
        if (!full_input) {
            fprintf(stderr, "Error: Could not load image %s\n", input_path);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        full_output = malloc(w * h);
        clock_gettime(CLOCK_MONOTONIC, &start); // Start timing for the current image
      }
        MPI_Bcast(&w, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&h, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int* sendcounts = malloc(size * sizeof(int));
        int* displs = malloc(size * sizeof(int));
        int* sc_rgb = malloc(size * sizeof(int));
        int* dp_rgb = malloc(size * sizeof(int));

        int offset = 0;
        for (int i = 0; i < size; i++) {
            int rows = (h / size) + (i < (h % size) ? 1 : 0);
            sendcounts[i] = rows * w;
            displs[i] = offset * w;
            sc_rgb[i] = rows * w * channels;
            dp_rgb[i] = offset * w * channels;
            offset += rows;
        }

        int local_rows = sendcounts[rank] / w;
        int local_pixels = sendcounts[rank];

        unsigned char* local_rgb = malloc(local_pixels * channels);
        unsigned char* local_gray = malloc(local_pixels);
        
        // CRITICAL: Use calloc for ALL halo buffers to prevent "garbage" lines
        unsigned char* gray_halos = calloc(w * (local_rows + 2), 1);
        unsigned char* blur_halos = calloc(w * (local_rows + 2), 1);
        unsigned char* sobel_halos = calloc(w * (local_rows + 2), 1);
        unsigned char* local_final = malloc(local_pixels);

        MPI_Scatterv(full_input, sc_rgb, dp_rgb, MPI_UNSIGNED_CHAR,
                    local_rgb, local_pixels * channels, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        
        // 1. Grayscale
        grayscale_filter(local_rgb, local_gray, w, local_rows, channels, false);

        // 2. Initial Halo Exchange
        memcpy(gray_halos + w, local_gray, local_pixels);
        int up = (rank == 0) ? MPI_PROC_NULL : rank - 1;
        int down = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

        MPI_Sendrecv(local_gray, w, MPI_UNSIGNED_CHAR, up, 0,
                    gray_halos + (local_rows + 1) * w, w, MPI_UNSIGNED_CHAR, down, 0,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(local_gray + (local_rows - 1) * w, w, MPI_UNSIGNED_CHAR, down, 1,
                    gray_halos, w, MPI_UNSIGNED_CHAR, up, 1,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 3. Blur
        blur_filter(gray_halos, blur_halos, w, local_rows + 2, false);

        // 4. SECOND HALO EXCHANGE (This makes the blurred edges valid for Sobel)
        // We create a temp buffer of the blurred pixels to swap them
        unsigned char* temp_blur = malloc(local_pixels);
        for(int i = 0; i < local_rows; i++) {
            memcpy(temp_blur + (i * w), blur_halos + ((i + 1) * w), w);
        }

        MPI_Sendrecv(temp_blur, w, MPI_UNSIGNED_CHAR, up, 2,
                    blur_halos + (local_rows + 1) * w, w, MPI_UNSIGNED_CHAR, down, 2,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(temp_blur + (local_rows - 1) * w, w, MPI_UNSIGNED_CHAR, down, 3,
                    blur_halos, w, MPI_UNSIGNED_CHAR, up, 3,
                    MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 5. Sobel
        sobel_filter(blur_halos, sobel_halos, w, local_rows + 2, false);

        // 6. Extraction (The +1 skips the top halo row)
        for (int i = 0; i < local_rows; i++) {
            memcpy(local_final + (i * w), sobel_halos + ((i + 1) * w), w);
        }

        // 7. Gather result
        MPI_Gatherv(local_final, local_pixels, MPI_UNSIGNED_CHAR,
                    full_output, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

        
        if (rank == 0) {
            clock_gettime(CLOCK_MONOTONIC, &end);
            //double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            //printf("%d: MPI execution time: %f seconds for %s\n", iteration, time, filename);
            
            char output_path[512];
            snprintf(output_path, sizeof(output_path), "data/output_mpi/%s", filename);
            stbi_write_jpg(output_path, w, h, 1, full_output, 90);
            
            stbi_image_free(full_input);
            free(full_output);
        }

      iteration++;
      free(sendcounts); free(displs); free(sc_rgb); free(dp_rgb);
      free(local_rgb); free(local_gray); free(gray_halos);
      free(blur_halos); free(sobel_halos); free(local_final); free(temp_blur);

    }

    if (rank == 0) {
        closedir(dir);
        clock_gettime(CLOCK_MONOTONIC, &total_end);
        double total_time = (total_end.tv_sec - total_start.tv_sec) + (total_end.tv_nsec - total_start.tv_nsec) / 1e9;
        printf("Total MPI execution time for all images: %f seconds\n", total_time);
    }

    MPI_Finalize();
    return 0;
}