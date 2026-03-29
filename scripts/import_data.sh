#!/bin/bash

# Configuration
DATA_DIR="../data/input"
ZIP_FILE="$DATA_DIR/100k-vehicle-dashcam-image-dataset.zip"
URL="https://www.kaggle.com/api/v1/datasets/download/mdfahimbinamin/100k-vehicle-dashcam-image-dataset"
NUM_FILES=3000
PARTITIONS=$((NUM_FILES / 3))
FOLDER_IN_ZIP="test"

# Ensure the target directory exists
mkdir -p "$DATA_DIR"

# Check if directory already has images (count ignores subdirectories)
FILE_COUNT=$(find "$DATA_DIR" -maxdepth 1 -type f | wc -l)

if [ "$FILE_COUNT" -gt 2 ]; then
    echo "Found $FILE_COUNT files in $DATA_DIR."
    echo "Assuming dataset is already downloaded. Skipping download step."
else
    echo "Dataset not found. Downloading using curl..."
    curl -L -o "$ZIP_FILE" "$URL"

    # Check if unzip is installed
    if command -v unzip &> /dev/null; then
        echo "Fetching list of files from the zip..."
        
        # 1. 'unzip -qql' lists files.
        # 2. 'awk' gets just the file path
        # 3. 'grep -v "/$"' ignores directory entries
        # 4. 'grep -Ei "\.jpg$"' keeps only jpg images
        # 5. 'head' grabs exactly NUM_FILES
        unzip -qql "$ZIP_FILE" "$FOLDER_IN_ZIP/*" | awk '{print $4}' | grep -v "/$" | grep -Ei '\.jpg$' | head -n $NUM_FILES > "$DATA_DIR/file_list.txt"
        
        ACTUAL_COUNT=$(wc -l < "$DATA_DIR/file_list.txt")
        echo "Directly extracting $ACTUAL_COUNT images to $DATA_DIR..."

        # Use xargs to pass the file list to unzip in batches (to avoid "Argument list too long" errors)
        # The -j flag extracts files directly into the target folder without creating 'test/'
        cat "$DATA_DIR/file_list.txt" | xargs -n 500 unzip -j -q "$ZIP_FILE" -d "$DATA_DIR"
        
        echo "Cleaning up..."
        rm "$DATA_DIR/file_list.txt"
        rm "$ZIP_FILE"
    else
        echo "Warning: 'unzip' command not found. Please extract $ZIP_FILE manually."
        exit 1
    fi
fi

if [ -d "$DATA_DIR/small" ] && [ -d "$DATA_DIR/medium" ] && [ -d "$DATA_DIR/large" ]; then
    echo "Partition folders already exist. Skipping partition step."
else
    echo "Creating small, medium, and large partitions..."
    mkdir -p "$DATA_DIR/small" "$DATA_DIR/medium" "$DATA_DIR/large"

    # Collect extracted jpg files from the root of DATA_DIR in stable order.
    mapfile -d '' IMAGE_FILES < <(find "$DATA_DIR" -maxdepth 1 -type f -name '*.jpg' -print0 | sort -z)
    TOTAL_IMAGES=${#IMAGE_FILES[@]}

    for i in 0 1 2; do
        case $i in
            0) TARGET_DIR="$DATA_DIR/small" ;;
            1) TARGET_DIR="$DATA_DIR/medium" ;;
            2) TARGET_DIR="$DATA_DIR/large" ;;
        esac

        START=$((i * PARTITIONS))
        END=$((START + PARTITIONS))

        for ((j=START; j<END && j<TOTAL_IMAGES; j++)); do
            mv "${IMAGE_FILES[$j]}" "$TARGET_DIR/"
        done
    done

    echo "Dataset ready! Up to $PARTITIONS images moved into each of: small, medium, large."
fi