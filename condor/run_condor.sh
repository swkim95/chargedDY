#!/bin/bash

# Check if a directory is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: ./run_condor.sh <directory_path>"
    echo "Example: ./run_condor.sh ../samples/"
    exit 1
fi

# Get the absolute path of the input directory
INPUT_PATH="$1"
if [[ ! "$INPUT_PATH" = /* ]]; then
    # If relative path, convert to absolute
    INPUT_PATH="$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
fi

# Check if the directory exists
if [ ! -d "$INPUT_PATH" ]; then
    echo "Error: Directory '$INPUT_PATH' does not exist"
    exit 1
fi

echo "Processing directory: $INPUT_PATH"

# Loop over each directory in the input path
for dir in "$INPUT_PATH"/*; do
    if [ -d "$dir" ]; then
        echo "Processing subdirectory: $dir"
        
        # Create log folders
        mkdir -p "$dir/log_condor/err"
        mkdir -p "$dir/log_condor/log"
        mkdir -p "$dir/log_condor/out"

        # Check for the Condor submission script
        sample_name=$(basename "$dir")
        condor_script="condor_${sample_name}.sub"
        
        if [ -f "$dir/$condor_script" ]; then
            # Change to the directory and submit the job
            echo "Found submission file: $condor_script"
            cd "$dir" || { echo "Error: Failed to change directory to $dir"; exit 1; }
            condor_submit "$condor_script"
            echo "Successfully submitted: $condor_script from $dir"
            cd - > /dev/null  # Change back to the previous directory
        else
            echo "Warning: No submission file ($condor_script) found in $dir"
        fi
    fi
done

echo "Completed processing all subdirectories in $INPUT_PATH"