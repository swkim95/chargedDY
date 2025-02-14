#!/bin/bash

# Check if a directory is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: ./check_err.sh <directory_path>"
    echo "Example: ./check_err.sh ../samples/"
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

# Count total log files
total_logs=0
for process_dir in "$INPUT_PATH"/*; do
    if [ -d "$process_dir" ]; then
        total_logs=$(($total_logs + $(find "$process_dir/log_condor/log" -name "l_*.log" 2>/dev/null | wc -l)))
    fi
done

# Initialize progress counter
processed_logs=0
progress_width=50 # Fixed width for the progress bar

# Loop through all Process name directories under the specified base directory
for process_dir in "$INPUT_PATH"/*; do
    if [ -d "$process_dir" ]; then
        # Loop through all log files in the current Process name directory
        for log_file in "$process_dir"/log_condor/log/l_*.log; do
            # Check if the log file exists
            if [ ! -f "$log_file" ]; then
                echo "No log files found in $process_dir/log_condor/log/"
                continue
            fi

            # Extract the process number from the log file name
            process_number=$(basename "$log_file" | cut -d'_' -f2 | cut -d'.' -f1)
            process_name=$(basename "$process_dir")

            # Display progress message
            processed_logs=$((processed_logs + 1))
            percent=$((processed_logs * 100 / total_logs))
            num_hashes=$((processed_logs * progress_width / total_logs))

            # Create the progress bar using simple string manipulation
            progress_bar=$(printf "%-${num_hashes}s" "#" | tr ' ' '#')
            progress_bar+=$(printf "%-$((progress_width - num_hashes))s" "" | tr ' ' '-')
            progress_bar="[$progress_bar]"

            printf "%s %d/%d complete ( %d%% )\r" "$progress_bar" "$processed_logs" "$total_logs" "$percent"

            # Check if the log file contains an exit-code
            if grep -q "Job terminated of its own accord" "$log_file"; then
                exit_code_line=$(grep "Job terminated of its own accord" "$log_file")
                exit_code=$(echo "$exit_code_line" | grep -oP "exit-code \K-?\d+")

                # Validate that the exit code is an integer
                if [[ "$exit_code" =~ ^-?[0-9]+$ ]]; then
                    # Check if the exit code is not zero
                    if [ "$exit_code" -ne 0 ]; then
                        # Notify the user about the error
                        echo -e "\nFound error with HTCondor, Process name: $process_name Process number: $process_number"

                        # Display content from the corresponding output file
                        output_file="$process_dir/log_condor/out/o_$process_number.out"
                        if [ -f "$output_file" ]; then
                            echo -e "--- Output file content (o_$process_number.out) ---"
                            cat "$output_file"
                        else
                            echo "Output file $output_file not found."
                        fi

                        # Display content from the corresponding error file
                        error_file="$process_dir/log_condor/err/e_$process_number.err"
                        if [ -f "$error_file" ]; then
                            echo -e "--- Error file content (e_$process_number.err) ---"
                            cat "$error_file"
                        else
                            echo "Error file $error_file not found."
                        fi
                    fi
                else
                    echo -e "\nWarning: Invalid exit code in $log_file: '$exit_code'"
                fi
            else
                echo -e "\nWarning: No job termination logs found in $log_file"
            fi
        done
    fi
done

echo -e "\nAll files checked. Total logs checked: $processed_logs/$total_logs"
