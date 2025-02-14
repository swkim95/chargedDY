#!/usr/bin/env python3
import os
import argparse

def get_files(base_dir):
    """
    Recursively walks through base_dir and returns a list of full file paths ending with .root.
    """
    file_list = []
    for root, dirs, files in os.walk(base_dir):
        for file in files:
            if file.endswith(".root"):
                file_list.append(os.path.join(root, file))
    return file_list

def main():
    parser = argparse.ArgumentParser(
        description='For each era, read list_<era>.txt containing process names, then create an output <process name>.txt '
                    'under ./<era> listing all .root files (from the base directory) that match the process.'
    )
    parser.add_argument('-b', '--base_dir', default="/pnfs/knu.ac.kr/data/cms/store/user/sungwon/DY_Run2_UL_NanoAOD",
                        help='Base directory path to search for .root files (e.g. /pnfs/knu.ac.kr/data/cms/store/user/sungwon/DY_Run2_UL_NanoAOD)')
    parser.add_argument('-l', '--list_dir', default='.', 
                        help='Directory containing list_<era>.txt files (default: current directory)')
    parser.add_argument('-o', '--output_dir', default='.', 
                        help='Output base directory for writing process files, organized by era (default: current directory)')
    parser.add_argument("-e", "--era", help='Optional: Specify a single era (e.g. 2016post, 2016pre, 2017, 2018). If not provided, iterate over all four eras.')
    args = parser.parse_args()
    
    # If no era provided, iterate over all four eras.
    if args.era:
        eras = [args.era]
    else:
        eras = ["2016", "2016APV", "2017", "2018"]
        
    for era in eras:
        # Construct the list file path: list_<era>.txt in list_dir
        list_file_path = os.path.join(args.list_dir, f"list_{era}.txt")
        if not os.path.exists(list_file_path):
            print(f"List file {list_file_path} does not exist. Skipping era {era}.")
            continue
        with open(list_file_path, 'r') as lf:
            # Each process name per line (stripping whitespace)
            processes = [line.strip() for line in lf if line.strip()]
        
        # Create output directory for this era
        era_output_dir = os.path.join(args.output_dir, era)
        os.makedirs(era_output_dir, exist_ok=True)
        
        for process in processes:
            # Define search directory under <base_dir>/<era>/<process>/
            eraName = era
            if era == "2016" : eraName = "2016_postVFP"
            if era == "2016APV" : eraName = "2016_preVFP"
            process_dir = os.path.join(args.base_dir, eraName, process)
            if not os.path.isdir(process_dir):
                print(f"Directory {process_dir} does not exist. Skipping process '{process}' for era {era}.")
                continue
            matching_files = get_files(process_dir)
            output_file = os.path.join(era_output_dir, f"{process}.txt")
            with open(output_file, 'w') as out_f:
                for mf in matching_files:
                    out_f.write(mf + "\n")
            print(f"For era {era}, process '{process}': found {len(matching_files)} matching .root files. Written to {output_file}")

if __name__ == '__main__':
    main()
