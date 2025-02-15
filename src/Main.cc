#include "DYanalyzer.h"

#include <iostream>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    // Arguments
    // 1. Input file list
    // 2. Era
    // 3. Process name
    // 4. IsMC
    // 5. DoPUCorrection
    // 6. DoL1PreFiringCorrection
    // 7. DoIDSF
    // 8. DoIsoSF
    // 9. DoTrigSF
    // 10. DoRocco
    // 11. Output file name

    // Check if the number of arguments is correct
    if (argc != 12) {
        std::cerr << "---------------------------------------------------------" << std::endl;
        std::cerr << "[Error] Main.cc - The number of arguments is incorrect" << std::endl;
        std::cerr << "[Error] Main.cc - Usage: ./DYanalysis <Input file list> <Era> <Process name> <IsMC> <DoPUCorrection> <DoL1PreFiringCorrection> <DoIDSF> <DoIsoSF> <DoTrigSF> <DoRocco> <Output file name>" << std::endl;
        std::cerr << "---------------------------------------------------------" << std::endl;
        return 1;
    }

    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "[Info] Main.cc - Start DY analysis" << std::endl;
    std::cout << "[Info] Main.cc - Input file list: " << argv[1] << std::endl;
    std::cout << "[Info] Main.cc - Era: " << argv[2] << std::endl;
    std::cout << "[Info] Main.cc - Process name: " << argv[3] << std::endl;
    std::cout << "[Info] Main.cc - IsMC: " << argv[4] << std::endl;
    std::cout << "[Info] Main.cc - DoPUCorrection: " << argv[5] << std::endl;
    std::cout << "[Info] Main.cc - DoL1PreFiringCorrection: " << argv[6] << std::endl;
    std::cout << "[Info] Main.cc - DoRocco: " << argv[7] << std::endl;
    std::cout << "[Info] Main.cc - DoIDSF: " << argv[8] << std::endl;
    std::cout << "[Info] Main.cc - DoIsoSF: " << argv[9] << std::endl;
    std::cout << "[Info] Main.cc - DoTrigSF: " << argv[10] << std::endl;
    std::cout << "[Info] Main.cc - Output file name: " << argv[11] << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;

    // Get arguments
    std::string sInputFileList = argv[1];
    std::string sEra = argv[2];
    std::string sProcessName = argv[3];
    bool bIsMC = std::stoi(argv[4]);
    bool bDoPUCorrection = std::stoi(argv[5]);
    bool bDoL1PreFiringCorrection = std::stoi(argv[6]);
    bool bDoRocco = std::stoi(argv[7]);
    bool bDoIDSF = std::stoi(argv[8]);
    bool bDoIsoSF = std::stoi(argv[9]);
    bool bDoTrigSF = std::stoi(argv[10]);
    std::string sOutputFileName = argv[11];
    // Set Muon Eff SF histogram names
    std::string sHistName_ID = "NUM_TightID_DEN_TrackerMuons_abseta_pt";
    std::string sHistName_Iso = "NUM_TightRelIso_DEN_TightIDandIPCut_abseta_pt";
    std::string sHistName_Trig = "NUM_IsoMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";

    if (sEra.find("2016") != std::string::npos) {
        sHistName_Trig = "NUM_IsoMu24_or_IsoTkMu24_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";
    }
    else if (sEra == "2017") {
        sHistName_Trig = "NUM_IsoMu27_DEN_CutBasedIdTight_and_PFIsoTight_abseta_pt";
    }
    
    // Set Rocco file name
    std::string sRoccoFileName = "";
    if (sEra == "2016APV") {
        sRoccoFileName = "../RoccoR/RoccoR2016aUL.txt";
    } else if (sEra == "2016") {
        sRoccoFileName = "../RoccoR/RoccoR2016bUL.txt";
    } else if (sEra == "2017") {
        sRoccoFileName = "../RoccoR/RoccoR2017UL.txt";
    } else if (sEra == "2018") {
        sRoccoFileName = "../RoccoR/RoccoR2018UL.txt";
    }

    DYanalyzer analyzer(sInputFileList, sProcessName, sEra, sHistName_ID, sHistName_Iso, sHistName_Trig, sRoccoFileName, bIsMC, bDoPUCorrection, bDoL1PreFiringCorrection, bDoRocco, bDoIDSF, bDoIsoSF, bDoTrigSF);
    analyzer.Init();
    analyzer.Analyze_Z();

    TFile* f_output = TFile::Open(sOutputFileName.c_str(), "RECREATE");
    f_output->cd();
    analyzer.WriteHistograms(f_output);
    f_output->Close();
    
    std::cout << "---------------------------------------------------------" << std::endl;
    std::cout << "[Info] Main.cc - DY analysis is finished" << std::endl;
    std::cout << "[Info] Main.cc - Output file is saved as " << sOutputFileName << std::endl;
    std::cout << "---------------------------------------------------------" << std::endl;
    
    return 0;
}