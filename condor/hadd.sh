#!/bin/bash

# Check if a directory is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: ./hadd.sh <directory_path>"
    echo "Example: ./hadd.sh ../samples/"
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

# Process individual directories first
for process_dir in "$INPUT_PATH"/*; do
    if [ -d "$process_dir" ]; then
        process_name=$(basename "$process_dir")
        echo "Merging files for: $process_name"
        hadd "$INPUT_PATH/hist_$process_name.root" "$process_dir"/*.root
    fi
done

# Special merging for specific samples
echo "Performing special merging for specific samples..."

# Function to safely merge files
merge_files() {
    local pattern="$1"
    local output="$2"
    if ls "$INPUT_PATH"/hist_"$pattern"_*.root 1> /dev/null 2>&1; then
        echo "Merging $pattern samples..."
        hadd "$INPUT_PATH/$output" "$INPUT_PATH"/hist_"$pattern"_*.root
    else
        echo "Warning: No files found matching pattern $pattern"
    fi
}

# Merge specific sample groups
merge_files "DYJetsToMuMu_M-10to50" "hist_DYJetsToMuMu_M-10to50.root"
merge_files "WJetsToLNu_HT-100To200" "hist_WJetsToLNu_HT-100To200.root"
merge_files "WJetsToLNu_HT-200To400" "hist_WJetsToLNu_HT-200To400.root"
merge_files "WJetsToLNu_HT-400To600" "hist_WJetsToLNu_HT-400To600.root"
merge_files "WJetsToLNu_HT600To800" "hist_WJetsToLNu_HT600To800.root"
merge_files "WJetsToLNu_HT800To1200" "hist_WJetsToLNu_HT800To1200.root"
merge_files "WJetsToLNu_HT1200To2500" "hist_WJetsToLNu_HT1200To2500.root"
merge_files "WJetsToLNu_HT2500ToInf" "hist_WJetsToLNu_HT2500ToInf.root"
merge_files "WToMuNu_M-200" "hist_WToMuNu_M-200.root"

echo "All merging operations completed in $INPUT_PATH"

