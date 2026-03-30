#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../include/stb_image_write.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "../include/stb_image_resize2.h"

#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This program resizes images in place within the specified folders.
// It uses stb_image to load and save images, and stb_image_resize to perform the resizing operation.
// The target dimensions for each folder are defined in the FolderTarget struct array.
// The program processes each folder sequentially, resizing all valid image files it finds.
// It also includes error handling for loading, resizing, and writing images, and it skips non-image files and directories.
typedef struct {
    const char *name;
    int width;
    int height;
} FolderTarget;


// Helper function to check if a filename has a .jpg or .jpeg extension (case-insensitive)
static int has_jpg_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot) {
        return 0;
    }

    char ext[8] = {0};
    size_t i;
    for (i = 0; dot[i] != '\0' && i < sizeof(ext) - 1; i++) {
        ext[i] = (char)tolower((unsigned char)dot[i]);
    }
    ext[i] = '\0';

    return strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0;
}

// Function to resize an image at the given path to the specified width and height.
static int resize_image_in_place(const char *path, int out_w, int out_h) {
    int in_w, in_h, in_channels;
    unsigned char *input = stbi_load(path, &in_w, &in_h, &in_channels, 3);
    if (!input) {
        fprintf(stderr, "Failed to load image: %s\n", path);
        return 0;
    }

    size_t out_size = (size_t)out_w * (size_t)out_h * 3u;
    unsigned char *output = (unsigned char *)malloc(out_size);
    if (!output) {
        fprintf(stderr, "Out of memory resizing: %s\n", path);
        stbi_image_free(input);
        return 0;
    }

    unsigned char *result = stbir_resize_uint8_linear(
        input,
        in_w,
        in_h,
        0,
        output,
        out_w,
        out_h,
        0,
        STBIR_RGB
    );

    if (!result) {
        fprintf(stderr, "Resize failed for: %s\n", path);
        free(output);
        stbi_image_free(input);
        return 0;
    }

    if (!stbi_write_jpg(path, out_w, out_h, 3, output, 90)) {
        fprintf(stderr, "Failed to write image: %s\n", path);
        free(output);
        stbi_image_free(input);
        return 0;
    }

    free(output);
    stbi_image_free(input);
    return 1;
}

// Function to process all images in a given folder target, resizing them to the specified dimensions.
static void process_folder(const char *base_dir, const FolderTarget *target) {
    char folder_path[512];
    snprintf(folder_path, sizeof(folder_path), "%s/%s", base_dir, target->name);

    DIR *dir = opendir(folder_path);
    if (!dir) {
        printf("Skipping missing folder: %s\n", folder_path);
        return;
    }

    printf("Processing %s -> %dx%d\n", folder_path, target->width, target->height);

    struct dirent *entry;
    int resized_count = 0;
    int failed_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (!has_jpg_extension(entry->d_name)) {
            continue;
        }

        char image_path[1024];
        snprintf(image_path, sizeof(image_path), "%s/%s", folder_path, entry->d_name);

        if (resize_image_in_place(image_path, target->width, target->height)) {
            resized_count++;
        } else {
            failed_count++;
        }
    }

    closedir(dir);
    printf("Done %s: resized=%d failed=%d\n", target->name, resized_count, failed_count);
}

int main(void) {
    const char *base_dir = "data/input";
    // Define the target dimensions for each folder. The program will look for these folders inside the base directory and resize images accordingly.
    const FolderTarget targets[] = {
        {"small", 854, 480},
        {"medium", 1920, 1080},
        {"large", 3840, 2160}
    };

    size_t i;
    // Process each target folder sequentially, resizing images in place.
    for (i = 0; i < sizeof(targets) / sizeof(targets[0]); i++) {
        process_folder(base_dir, &targets[i]);
    }

    return 0;
}