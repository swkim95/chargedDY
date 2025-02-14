import os
import sys
import argparse

# Function to generate condor scripts
# It takes base output directory and created sub-directories like this:
# base_output_directory/
# -- config_name/
# ---- era/
# ------ process_name/
# -------- exe_process_name.sh
# -------- condor_process_name.sub
# -------- log_condor/
# ---------- log/
# ---------- err/
# ---------- out/
# List of config_name: Org, PU, L1, Rocco, ID, Iso, All
# List of era: 2016APV, 2016, 2017, 2018

def generate_python_scripts(process_name, base_output_directory, config_name, isMC, era, doPU, doL1, doRocco, doIDSF, doIso, doTrig):

        # Determine the full output directory path
        full_output_directory = os.path.join(base_output_directory, process_name)

        # Create the output directory for condor scripts if it doesn't exist
        os.makedirs(full_output_directory, exist_ok=True)

        # Create log directory structure
        log_condor_path = os.path.join(full_output_directory, "log_condor")
        os.makedirs(os.path.join(log_condor_path, "log"), exist_ok=True)
        os.makedirs(os.path.join(log_condor_path, "err"), exist_ok=True)
        os.makedirs(os.path.join(log_condor_path, "out"), exist_ok=True)

        # Define the shell script content without directory creation (pre-created at the Python level)
        exe_script_content = f'''#! /bin/bash

source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh

cd /u/user/swkim/CMS/chargedDY_NanoAOD/chargedDY/install

export INSTALL_DIR_PATH=$PWD

export PATH=$PATH:$INSTALL_DIR_PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

./DYanalysis ${1} {era} {process_name} 0 {doPU} {doL1} {doRocco} {doIDSF} {doIso} {doTrig} {full_output_directory}/{process_name}_${{2}}.root
'''

        if isMC:
            exe_script_content = f'''#!/bin/bash

source /cvmfs/sft.cern.ch/lcg/views/LCG_105/x86_64-centos7-gcc12-opt/setup.sh

cd /u/user/swkim/CMS/chargedDY_NanoAOD/chargedDY/install

export INSTALL_DIR_PATH=$PWD

export PATH=$PATH:$INSTALL_DIR_PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

./DYanalysis ${1} {era} {process_name} 1 {doPU} {doL1} {doRocco} {doIDSF} {doIso} {doTrig} {full_output_directory}/{process_name}_${{2}}.root
'''

        sub_script_content = f'''universe = vanilla
executable = exe_{process_name}.sh

arguments = $(InputFileList) $(Process)
request_memory = 1024 MB

should_transfer_files = YES
when_to_transfer_output = ON_EXIT

output = ./log_condor/out/o_$(Process).out
error  = ./log_condor/err/e_$(Process).err
log    = ./log_condor/log/l_$(Process).log

JobBatchName = {process_name}_{era}_{config_name}
queue InputFileList from /u/user/swkim/CMS/chargedDY_NanoAOD/chargedDY/fileList/{era}/{process_name}.txt

'''

## Use this when some cluster is not available
## Blacklist cluster by using Requirements
#         sub_script_content = f'''universe = vanilla
# executable = exe_{process_name}.sh

# Requirements = (Machine =!= "cluster291.knu.ac.kr") && (TARGET.Arch == "X86_64") && (TARGET.OpSys == "LINUX") && (TARGET.HasFileTransfer)

# arguments = $(InputFileList) $(Process)
# request_memory = 1024 MB

# should_transfer_files = YES
# when_to_transfer_output = ON_EXIT

# output = ./log_condor/out/o_$(Process).out
# error  = ./log_condor/err/e_$(Process).err
# log    = ./log_condor/log/l_$(Process).log

# JobBatchName = {process_name}_{era}_{config_name}
# queue InputFileList from /u/user/swkim/CMS/chargedDY_NanoAOD/chargedDY/fileList/{era}/{process_name}.txt

# '''
        # Create the full path for the new Python file
        script_full_path   = os.path.join(full_output_directory, f"exe_{process_name}.sh")
        script_full_path_2 = os.path.join(full_output_directory, f"condor_{process_name}.sub")

        # Write the content to the new Python file
        with open(script_full_path, 'w') as script_file:
            script_file.write(exe_script_content)
        print(f"Generated script: {script_full_path}")

        with open(script_full_path_2, 'w') as script_file:
            script_file.write(sub_script_content)
        print(f"Generated script: {script_full_path_2}")


# How to run:
# python3 generate_condor_script.py -o /path/to/output/dir
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate condor scripts for DYanalysis jobs")
    parser.add_argument("-o", "--base_output_directory", help="Base output directory")
    args = parser.parse_args()
    
    # Define the eras (we always iterate over these four)
    eras = ["2016APV", "2016", "2017", "2018"]
    
    # Define the directory configurations
    configurations = {
        "Org": (0, 0, 0, 0, 0, 0),
        "PU": (1, 0, 0, 0, 0, 0),
        "L1": (1, 1, 0, 0, 0, 0),
        "Rocco": (1, 1, 1, 0, 0, 0),
        "ID": (1, 1, 1, 1, 0, 0),
        "Iso": (1, 1, 1, 1, 1, 0),
        "All": (1, 1, 1, 1, 1, 1)
    }
    
    for config_name, (doPU, doL1, doRocco, doIDSF, doIso, doTrig) in configurations.items():
        config_output_directory = os.path.join(args.base_output_directory, config_name)
        os.makedirs(config_output_directory, exist_ok=True)
        
        for era in eras:
            era_output_directory = os.path.join(config_output_directory, era)
            os.makedirs(era_output_directory, exist_ok=True)
            
            # Construct list file path for this era.
            list_file = os.path.join("/u/user/swkim/CMS/chargedDY_NanoAOD/chargedDY/fileList", f"list_{era}.txt")
            if not os.path.exists(list_file):
                print(f"List file {list_file} does not exist. Skipping era {era}.")
                continue
            with open(list_file, "r") as f:
                processes = [line.strip() for line in f if line.strip()]

            for process in processes:
                # Auto-detect isMC flag if desired.
                isMC = 0 if "SingleMuon" in process else 1
                generate_python_scripts(process, era_output_directory, config_name, isMC, era, doPU, doL1, doRocco, doIDSF, doIso, doTrig)