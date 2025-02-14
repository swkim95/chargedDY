#!/bin/bash

# Check if a directory is provided as an argument
if [ "$#" -ne 1 ]; then
    echo "Usage: ./memory_report.sh <directory_path>"
    echo "Example: ./memory_report.sh ../samples/"
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

# Variables to hold overall memory usage statistics
total_memory=0
total_count=0
declare -A process_memory

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

            # Update processed logs count
            processed_logs=$((processed_logs + 1))
            percent=$((processed_logs * 100 / total_logs))
            num_hashes=$((processed_logs * progress_width / total_logs))

            # Create the progress bar
            progress_bar=$(printf "%-${num_hashes}s" "#" | tr ' ' '#')
            progress_bar+=$(printf "%-$((progress_width - num_hashes))s" "" | tr ' ' '-')
            progress_bar="[$progress_bar]"

            # Display the progress
            printf "Processing %s ... %s %d/%d complete (%d%%)               \r" "$(basename "$process_dir")" "$progress_bar" "$processed_logs" "$total_logs" "$percent"

            # Check if the log file contains an exit code of 0
            if grep -q "Job terminated of its own accord" "$log_file"; then
                exit_code=$(grep -A1 "Job terminated of its own accord" "$log_file" | tail -n1 | grep -oP "exit-code \K-?\d+")
                
                if [[ "$exit_code" -eq 0 ]]; then
                    # Extract Memory usage from the line starting with "Memory (MB)"
                    memory_usage=$(grep "Memory (MB)" "$log_file" | awk '{print $4}')

                    if [[ -n "$memory_usage" ]]; then
                        # Aggregate total memory usage
                        total_memory=$((total_memory + memory_usage))
                        total_count=$((total_count + 1))

                        # Record memory usage by process
                        process_name=$(basename "$process_dir")
                        process_memory[$process_name]+="$memory_usage "
                    fi
                fi
            fi
        done
    fi
done

echo -e "\n\nMemory Usage Report for $INPUT_PATH"
echo "=================================="

# Calculating overall statistics
if [ $total_count -gt 0 ]; then
    # Total summary
    echo -e "\nOverall Memory Statistics:"
    echo "Total successful jobs analyzed: $total_count"
    
    min_memory=9999999
    max_memory=0
    sum_memory=0

    # Iterate through recorded memory usage values
    for mem in ${process_memory[@]}; do
        for m in $mem; do
            if [[ "$m" -lt "$min_memory" ]]; then
                min_memory="$m"
            fi
            if [[ "$m" -gt "$max_memory" ]]; then
                max_memory="$m"
            fi
            sum_memory=$((sum_memory + m))
        done
    done

    avg_memory=$((sum_memory / total_count))

    printf "%-10s %-10s %-10s\n" "Min (MB)" "Max (MB)" "Avg (MB)"
    printf "%-10d %-10d %-10d\n" "$min_memory" "$max_memory" "$avg_memory"

    # Process-specific statistics
    echo -e "\nPer-Process Memory Statistics:"
    echo "--------------------------------"
    for process_name in "${!process_memory[@]}"; do
        memory_array=(${process_memory[$process_name]})
        process_count="${#memory_array[@]}"

        if [ "$process_count" -gt 0 ]; then
            min_memory=9999999
            max_memory=0
            sum_memory=0

            for mem in "${memory_array[@]}"; do
                if [[ "$mem" -lt "$min_memory" ]]; then
                    min_memory="$mem"
                fi
                if [[ "$mem" -gt "$max_memory" ]]; then
                    max_memory="$mem"
                fi
                sum_memory=$((sum_memory + mem))
            done

            avg_memory=$((sum_memory / process_count))

            echo -e "\n$process_name (Jobs: $process_count)"
            printf "%-10s %-10s %-10s\n" "Min (MB)" "Max (MB)" "Avg (MB)"
            printf "%-10d %-10d %-10d\n" "$min_memory" "$max_memory" "$avg_memory"
        fi
    done
else
    echo "No successful jobs found with exit code 0."
fi