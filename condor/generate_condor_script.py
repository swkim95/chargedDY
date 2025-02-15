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

def generate_python_scripts(process_name, base_output_directory, condor_sub_directory, config_name, isMC, era, doPU, doL1, doRocco, doIDSF, doIso, doTrig):

        # Determine the full output directory path for root files
        full_output_directory = os.path.join(base_output_directory, config_name, era, process_name)

        # Determine the full condor submission directory path
        full_condor_directory = os.path.join(condor_sub_directory, config_name, era, process_name)

        # Create the output directory for root files if it doesn't exist
        os.makedirs(full_output_directory, exist_ok=True)

        # Create the condor submission directory if it doesn't exist
        os.makedirs(full_condor_directory, exist_ok=True)

        # Create log directory structure under condor submission directory
        log_condor_path = os.path.join(full_condor_directory, "log_condor")
        os.makedirs(os.path.join(log_condor_path, "log"), exist_ok=True)
        os.makedirs(os.path.join(log_condor_path, "err"), exist_ok=True)
        os.makedirs(os.path.join(log_condor_path, "out"), exist_ok=True)

        # Define the shell script content
        exe_script_content = f'''#! /bin/bash

export X509_USER_PROXY=${3}
voms-proxy-info -all
voms-proxy-info -all -file ${3}

source /cvmfs/sft.cern.ch/lcg/views/LCG_107/x86_64-el9-gcc14-opt/setup.sh

cd /afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY/install

export INSTALL_DIR_PATH=$PWD

export PATH=$PATH:$INSTALL_DIR_PATH/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_DIR_PATH/lib

./DYanalysis ${1} {era} {process_name} {isMC} {doPU} {doL1} {doRocco} {doIDSF} {doIso} {doTrig} {full_output_directory}/{process_name}_${{2}}.root
'''

        sub_script_content = f'''universe = vanilla
executable = exe_{process_name}.sh

Proxy_path = /afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY/x509up

arguments = $(InputFileList) $(Process) $(Proxy_path)
request_memory = 1024 MB

should_transfer_files = YES
when_to_transfer_output = ON_EXIT

output = ./log_condor/out/o_$(Process).out
error  = ./log_condor/err/e_$(Process).err
log    = ./log_condor/log/l_$(Process).log

+JobFlavour = "tomorrow"

JobBatchName = {process_name}_{era}_{config_name}
queue InputFileList from /afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY/fileList/{era}/{process_name}.txt

'''

        # Create the full path for the new Python file
        exe_script_full_path = os.path.join(full_condor_directory, f"exe_{process_name}.sh")
        sub_script_full_path = os.path.join(full_condor_directory, f"condor_{process_name}.sub")

        # Write the content to the new Python file
        with open(exe_script_full_path, 'w') as exe_script_file:
            exe_script_file.write(exe_script_content)
        print(f"Generated script: {exe_script_full_path}")

        with open(sub_script_full_path, 'w') as sub_script_file:
            sub_script_file.write(sub_script_content)
        print(f"Generated script: {sub_script_full_path}")


# How to run:
# python3 generate_condor_script.py -o /path/to/output/dir -s /path/to/submit/dir
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate condor scripts for DYanalysis jobs")
    parser.add_argument("-o", "--base_output_directory", help="Base output directory for root files")
    parser.add_argument("-s", "--condor_sub_directory", help="Condor submission directory")
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
            list_file = os.path.join("/afs/cern.ch/user/s/sungwon/private/ChargedDY/chargedDY/fileList", f"list_{era}.txt")
            if not os.path.exists(list_file):
                print(f"List file {list_file} does not exist. Skipping era {era}.")
                continue
            with open(list_file, "r") as f:
                processes = [line.strip() for line in f if line.strip()]

            for process in processes:
                # Auto-detect isMC flag if desired.
                isMC = 0 if "SingleMuon" in process else 1
                generate_python_scripts(process, args.base_output_directory, args.condor_sub_directory, config_name, isMC, era, doPU, doL1, doRocco, doIDSF, doIso, doTrig)