#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"

#include "../include/filters.h"
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) printf("Usage: mpirun -np <n> %s <input_image>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int w, h, channels;
    unsigned char* full_input = NULL;
    unsigned char* full_output = NULL;

    // 1. Rank 0 loads the image
    if (rank == 0) {
        full_input = stbi_load(argv[1], &w, &h, &channels, 0);
        if (!full_input) {
            fprintf(stderr, "Error: Could not load image %s\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("Image Loaded: %dx%d, %d channels\n", w, h, channels);
        full_output = malloc(w * h);
    }

    // 2. BROADCAST: Everyone needs to know these values before allocating memory
    MPI_Bcast(&w, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&h, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // 3. Handle non-divisible heights (Workload distribution)
    int* sendcounts = malloc(size * sizeof(int));
    int* displs = malloc(size * sizeof(int));
    int* sendcounts_rgb = malloc(size * sizeof(int));
    int* displs_rgb = malloc(size * sizeof(int));

    int offset = 0;
    for (int i = 0; i < size; i++) {
        int rows = (h / size) + (i < (h % size) ? 1 : 0);
        sendcounts[i] = rows * w;
        displs[i] = offset * w;
        sendcounts_rgb[i] = rows * w * channels;
        displs_rgb[i] = offset * w * channels;
        offset += rows;
    }

    int local_rows = sendcounts[rank] / w;
    int local_pixels = sendcounts[rank];

    // 4. Local Buffers
    unsigned char* local_rgb = malloc(local_pixels * channels);
    unsigned char* local_gray = malloc(local_pixels);
    unsigned char* local_gray_halos = calloc(w * (local_rows + 2), 1);
    unsigned char* local_blur_halos = calloc(w * (local_rows + 2), 1);
    unsigned char* local_final = malloc(local_pixels);

    // 5. Distribute data
    MPI_Scatterv(full_input, sendcounts_rgb, displs_rgb, MPI_UNSIGNED_CHAR,
                 local_rgb, local_pixels * channels, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // 6. Step A: Grayscale
    grayscale_serial(local_rgb, local_gray, w, local_rows, channels);

    // 7. Step B: Halo Exchange
    // Copy local gray into the "middle" of the halo buffer
    memcpy(local_gray_halos + w, local_gray, local_pixels);

    int up = (rank == 0) ? MPI_PROC_NULL : rank - 1;
    int down = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

    // Send my top row to 'up', receive into my bottom halo from 'down'
    MPI_Sendrecv(local_gray, w, MPI_UNSIGNED_CHAR, up, 0,
                 local_gray_halos + (local_rows + 1) * w, w, MPI_UNSIGNED_CHAR, down, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Send my bottom row to 'down', receive into my top halo from 'up'
    MPI_Sendrecv(local_gray + (local_rows - 1) * w, w, MPI_UNSIGNED_CHAR, down, 1,
                 local_gray_halos, w, MPI_UNSIGNED_CHAR, up, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    

    // 8. Step C: Filtering (Passing the chunk with halos)
    blur_serial(local_gray_halos, local_blur_halos, w, local_rows + 2);
    sobel_serial(local_blur_halos, local_final, w, local_rows + 2);

    // 9. Gather everything back
    MPI_Gatherv(local_final, local_pixels, MPI_UNSIGNED_CHAR,
                full_output, sendcounts, displs, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    // 10. Save and Cleanup
    if (rank == 0) {
        stbi_write_jpg("data/output_mpi/test_output_mpi.jpg", w, h, 1, full_output, 90);
        printf("MPI Success: Saved to data/output_mpi/test_output_mpi.jpg\n");
        stbi_image_free(full_input);
        free(full_output);
    }

    free(sendcounts); free(displs); free(sendcounts_rgb); free(displs_rgb);
    free(local_rgb); free(local_gray); free(local_gray_halos);
    free(local_blur_halos); free(local_final);

    MPI_Finalize();
    return 0;
}