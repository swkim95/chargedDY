#!/usr/bin/env python3
import os
import sys
import argparse

def split_file(era, process_name, split_n, base_dir):
    # Construct input file path: base_dir/era/process_name.txt
    input_file = os.path.join(base_dir, era, f"{process_name}.txt")
    if not os.path.exists(input_file):
        print(f"Input file {input_file} does not exist.")
        sys.exit(1)
    
    # Create output directory: base_dir/era/process_name/
    output_dir = os.path.join(base_dir, era, process_name)
    os.makedirs(output_dir, exist_ok=True)

    # Read all lines from the input file
    with open(input_file, "r") as f:
        lines = f.readlines()

    total_lines = len(lines)
    if total_lines == 0:
        print("Input file is empty. Nothing to split.")
        return

    # Calculate the number of split files needed
    num_files = (total_lines + split_n - 1) // split_n

    for i in range(num_files):
        part_lines = lines[i * split_n : (i + 1) * split_n]
        output_file = os.path.join(output_dir, f"{process_name}_{i}.txt")
        with open(output_file, "w") as out_f:
            out_f.writelines(part_lines)
        print(f"Written {len(part_lines)} lines to {output_file}")

def main():
    parser = argparse.ArgumentParser(
        description="For each era, read list_<era>.txt to get process names, then split the corresponding process file into smaller chunks."
    )
    parser.add_argument("-e", "--era", help="Optional: specify a single era (e.g. 2016APV, 2016, 2017, 2018). If not provided, iterate over all eras.", required=False)
    parser.add_argument("-n", "--split_n", type=int, default = 20, help="Number of lines per split file")
    parser.add_argument("-b", "--base_dir", default="./", help="Base directory for file lists (default: ./)")
    parser.add_argument("-l", "--list_dir", default=".", help="Directory containing list_<era>.txt files (default: current directory)")
    args = parser.parse_args()
    
    if args.era:
        eras = [args.era]
    else:
        eras = ["2016APV", "2016", "2017", "2018"]
    
    for era in eras:
        # Read the process names from list_<era>.txt located in the list_dir.
        list_file = os.path.join(args.list_dir, f"list_{era}.txt")
        if not os.path.exists(list_file):
            print(f"List file {list_file} does not exist. Skipping era {era}.")
            continue
        with open(list_file, "r") as f:
            processes = [line.strip() for line in f if line.strip()]
        
        for process in processes:
            split_file(era, process, args.split_n, args.base_dir)

if __name__ == "__main__":
    main()
