#!/bin/bash

# Directories to process
directories=("2016" "2017" "2018" "2016APV")

# Base directory path
base_dir="/afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY/fileList"

# Loop through each directory
for dir in "${directories[@]}"; do
    # Find all text files in the directory
    find "$base_dir/$dir" -type f -name "*.txt" | while read -r file; do
        # Use sed to replace the path in each file
        sed -i 's|/pnfs/knu.ac.kr/data/cms/store|root://cluster142.knu.ac.kr:1094//store|g' "$file"
        sed -i 's|/u/user/swkim/CMS/chargedDY_NanoAOD|/afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY|g' "$file"
        echo "Updated $file"
    done
done