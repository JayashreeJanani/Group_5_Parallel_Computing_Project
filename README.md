# CS6025 Intro To Parallel Process

## Group 5

### Group Members: Ruturaj Sushil Ghodke (3775789), Thirunavukarasu MuthuValliappan(3785346), Jayashree Janani Purushothaman(3772545)

## Project Title: Autonomous Vehicle Vision System



**Github Repo**: https://github.com/JayashreeJanani/Group_5_Parallel_Computing_Project
---

### Compilation and Execution Steps

1. First download the dataset; execute this command from the root folder:
    ```bash
        cd scripts && ./import_data.sh
    ```
2. Execute this command from the root folder to resize and classify the input data into small medium and large:
    ```bash
        cd scripts && ./resize_data.sh
    ```
3. To compile the programs, run from the project root directory:
    ```bash
        make
    ```
4. Execution - Run the below commands for serial execution. It will take the given paths below as second argument:
    ```bash
        ./serial_app data/input/small   #Small resolution dataset
        ./serial_app data/input/medium  #Medium resolution dataset
        ./serial_app data/input/large   #Large resolution dataset
    ```
5. Execution - Run the below commands for openmp execution. It will take the given paths below as second argument:
    ```bash
        export OMP_NUM_THREADS=4 #mention number of threads here
        ./openmp_app data/input/small   #Small resolution dataset
        ./openmp_app data/input/medium  #Medium resolution dataset
        ./openmp_app data/input/large   #Large resolution dataset
    ```
6. Execution - Run the below commands for MPI execution. It will take the given paths below as second argument:
    ```bash
        mpirun -np 4 ./mpi_app data/input/small   #Small resolution dataset #mention number of processor after -np
        mpirun -np 4 ./mpi_app data/input/medium  #Medium resolution dataset
        mpirun -np 4 ./mpi_app data/input/large   #Large resolution dataset
    ```
### NOTE
Clear the outputs by running this command in the root folder
```bash
    cd scripts && ./reset_outputs.sh
```

### Outputs and Timings
After executing the program, the output image will be stored in the **data/output_serial**, **data/output_openmp** and **data/output_mpi** folder.

The program computes the time taken by the entire operation in seconds, and prints it to the terminal.

---
### Note:
This project utilizes the stb libraries by Sean Barrett:


stb_image.h: Used for loading various image formats into raw pixel arrays.
stb_image_write.h: Used for saving the processed arrays back to image files.

These are single-file, public-domain header libraries. We have chosen to use them to focus our development on the parallelization of the image filters rather than the complexities of the JPEG/PNG file formats.

Source: https://github.com/nothings/stb



