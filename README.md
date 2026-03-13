# CS6025 Intro To Parallel Process

## Group 5

### Group Members: Ruturaj Sushil Ghodke (3775789), Thirunavukarasu MuthuValliappan(3785346), Jayashree Janani Purushothaman(3772545)

## Project Title: Autonomous Vehicle Vision System



**Github Repo**: https://github.com/JayashreeJanani/Group_5_Parallel_Computing_Project
---

### Compilation and Execution Steps

1. From the project root directory, run:
    ```bash
        make
    ```
2. Execution - Run the below command. It will take input image as second argument:
    ```bash
        ./serial_app data/input/test_input.jpeg
    ```

### Outputs and Timings
After executing the program, the output image will be stored in the **data/output_serial** folder.

The program computes the time taken by the entire operation in seconds, and prints it to the terminal.

---
### Note:
This project utilizes the stb libraries by Sean Barrett:


stb_image.h: Used for loading various image formats into raw pixel arrays.
stb_image_write.h: Used for saving the processed arrays back to image files.

These are single-file, public-domain header libraries. We have chosen to use them to focus our development on the parallelization of the image filters rather than the complexities of the JPEG/PNG file formats.

Source: https://github.com/nothings/stb



