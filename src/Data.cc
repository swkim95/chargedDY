#include "Data.h"

void Data::Init() {
    // Check if Data is already initialized
    if (bIsInit) {
        std::cerr << "[Warning] Data::Init() - Data is already initialized" << std::endl;
        return;
    }

    // Initialize TChain
    fChain = new TChain("Events");

    // Read input file list
    std::ifstream infile(sInputFileList);
    if (!infile) {
        throw std::runtime_error("[Runtime Error] Data::Init() - Cannot open input file list: " + sInputFileList);
    }

    // For 2016B_APV_ver2, 3 files missing HLT_TkMu50, use Mu50 instead
    if (sInputFileList.find("SingleMuon_Run2016B_APV_ver2_7.txt") != std::string::npos) {
        bNoTkMu50 = true;
    }

    // Add input files to TChain
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "[Info] Data::Init() - Adding files to TChain" << std::endl;
    std::string line;
    int fileCount = 0;
    while (std::getline(infile, line)) {
        if (line.empty()) continue; // Skip empty lines
        fChain->Add(line.c_str());
        std::cout << "[Info] Data::Init() - Adding file: " << line << std::endl;
        fileCount++;
    }
    infile.close();
    std::cout << "[Info] Data::Init() - Added " << fileCount << " files to TChain" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    // Initialize TTreeReader
    fReader = new TTreeReader(fChain);
    // Load branches
    this->LoadBranches();
    // Set total events
    nTotalEvents = fChain->GetEntries();
    // Print initialization information
    this->PrintInitInfo();
    // Set bIsInit to true
    bIsInit = true;
}

void Data::PrintInitInfo() {
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << "[Info] Data::PrintInitInfo() - Data is initialized" << std::endl;
    std::cout << "[Info] Data::PrintInitInfo() - Input File List: " << sInputFileList << std::endl;
    std::cout << "[Info] Data::PrintInitInfo() - Total Events: " << nTotalEvents << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;    
}

void Data::LoadBranches() {
    // Print loading branches information   
    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "[Info] Data::LoadBranches() - Loading branches" << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;

    // Common branches for Data and MC
    // L1 pre-firing weight
    L1PreFiringWeight_Nom = new TTreeReaderValue<Float_t>(*fReader, "L1PreFiringWeight_Nom");
    // HLT
    // HighPt HLTs
    if (sEra == "2016APV" && !bNoTkMu50) {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
        HLT_TkMu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_TkMu50");
    }
    if (sEra == "2016APV" && bNoTkMu50) {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
    }
    if (sEra == "2016") {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
        HLT_TkMu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_TkMu50");
    }
    if (sEra == "2017" && !(sProcessName == "SingleMuon_Run2017B")) {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
        HLT_TkMu100 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_TkMu100");
        HLT_OldMu100 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_OldMu100");
    }
    if (sEra == "2017" && sProcessName == "SingleMuon_Run2017B") {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
    }
    if (sEra == "2018") {
        HLT_Mu50 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_Mu50");
        HLT_TkMu100 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_TkMu100");
        HLT_OldMu100 = new TTreeReaderValue<Bool_t>(*fReader, "HLT_OldMu100");
    }

    // Noise filter
    // Ref: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#UL_data
    // For 2016: Do not use Flag_ecalBadCalibFilter, Flag_BadChargedCandidateFilter
    // For 2017, 2018: Do not use Flag_BadChargedCandidateFilter
    Flag_goodVertices = new TTreeReaderValue<Bool_t>(*fReader, "Flag_goodVertices");
    Flag_globalSuperTightHalo2016Filter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_globalSuperTightHalo2016Filter");
    Flag_HBHENoiseFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseFilter");
    Flag_HBHENoiseIsoFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_HBHENoiseIsoFilter");
    Flag_EcalDeadCellTriggerPrimitiveFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_EcalDeadCellTriggerPrimitiveFilter");
    Flag_BadPFMuonFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonFilter");
    Flag_BadPFMuonDzFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadPFMuonDzFilter");
    Flag_hfNoisyHitsFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_hfNoisyHitsFilter");
    // Flag_BadChargedCandidateFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_BadChargedCandidateFilter");
    Flag_eeBadScFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_eeBadScFilter");
    if (sEra.find("2016") != std::string::npos) {
        Flag_ecalBadCalibFilter = new TTreeReaderValue<Bool_t>(*fReader, "Flag_ecalBadCalibFilter");
    }

    // Muons
    nMuon = new TTreeReaderValue<UInt_t>(*fReader, "nMuon"); 
    Muon_pt = new TTreeReaderArray<Float_t>(*fReader, "Muon_pt"); 
    Muon_eta = new TTreeReaderArray<Float_t>(*fReader, "Muon_eta");
    Muon_phi = new TTreeReaderArray<Float_t>(*fReader, "Muon_phi");
    Muon_mass = new TTreeReaderArray<Float_t>(*fReader, "Muon_mass");
    Muon_nTrackerLayers = new TTreeReaderArray<Int_t>(*fReader, "Muon_nTrackerLayers");
    Muon_nStations = new TTreeReaderArray<Int_t>(*fReader, "Muon_nStations");
    Muon_charge = new TTreeReaderArray<Int_t>(*fReader, "Muon_charge");
    Muon_tightId = new TTreeReaderArray<Bool_t>(*fReader, "Muon_tightId");
    Muon_mediumId = new TTreeReaderArray<Bool_t>(*fReader, "Muon_mediumId");
    Muon_looseId = new TTreeReaderArray<Bool_t>(*fReader, "Muon_looseId");
    Muon_highPtId = new TTreeReaderArray<UChar_t>(*fReader, "Muon_highPtId");
    Muon_isGlobal = new TTreeReaderArray<Bool_t>(*fReader, "Muon_isGlobal");
    Muon_isStandalone = new TTreeReaderArray<Bool_t>(*fReader, "Muon_isStandalone");
    Muon_isTracker = new TTreeReaderArray<Bool_t>(*fReader, "Muon_isTracker");
    Muon_isPFcand = new TTreeReaderArray<Bool_t>(*fReader, "Muon_isPFcand");
    Muon_tkRelIso = new TTreeReaderArray<Float_t>(*fReader, "Muon_tkRelIso");
    Muon_pfRelIso03_all = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_all");
    Muon_pfRelIso03_chg = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso03_chg");
    Muon_pfRelIso04_all = new TTreeReaderArray<Float_t>(*fReader, "Muon_pfRelIso04_all");
    Muon_tunepRelPt = new TTreeReaderArray<Float_t>(*fReader, "Muon_tunepRelPt");
    
    // Electron
    nElectron = new TTreeReaderValue<UInt_t>(*fReader, "nElectron");
    Electron_pt = new TTreeReaderArray<Float_t>(*fReader, "Electron_pt");
    Electron_eta = new TTreeReaderArray<Float_t>(*fReader, "Electron_eta");
    Electron_phi = new TTreeReaderArray<Float_t>(*fReader, "Electron_phi");
    Electron_mass = new TTreeReaderArray<Float_t>(*fReader, "Electron_mass");
    Electron_charge = new TTreeReaderArray<Int_t>(*fReader, "Electron_charge");
    Electron_cutBased = new TTreeReaderArray<Int_t>(*fReader, "Electron_cutBased");
    Electron_deltaEtaSC = new TTreeReaderArray<Float_t>(*fReader, "Electron_deltaEtaSC");
    // PF MET
    MET_phi = new TTreeReaderValue<Float_t>(*fReader, "MET_phi");
    MET_pt = new TTreeReaderValue<Float_t>(*fReader, "MET_pt");
    MET_sumEt = new TTreeReaderValue<Float_t>(*fReader, "MET_sumEt");

    // PUPPI MET
    PuppiMET_phi = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_phi");
    PuppiMET_pt = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_pt");
    PuppiMET_sumEt = new TTreeReaderValue<Float_t>(*fReader, "PuppiMET_sumEt");

    // NPV
    NPV = new TTreeReaderValue<Int_t>(*fReader, "PV_npvs");

    // Gen level branches and pileup branches will be loaded only for MCs
    if (bIsMC) {
        GenWeight                = new TTreeReaderValue<Float_t>(*fReader, "genWeight");
        Pileup_nPU               = new TTreeReaderValue<Int_t>(*fReader, "Pileup_nPU");
        Pileup_nTrueInt          = new TTreeReaderValue<Float_t>(*fReader, "Pileup_nTrueInt");
        nGenPart                 = new TTreeReaderValue<UInt_t>(*fReader, "nGenPart");;
        GenPart_eta              = new TTreeReaderArray<Float_t>(*fReader, "GenPart_eta");
        GenPart_phi              = new TTreeReaderArray<Float_t>(*fReader, "GenPart_phi");
        GenPart_pt               = new TTreeReaderArray<Float_t>(*fReader, "GenPart_pt");
        GenPart_mass             = new TTreeReaderArray<Float_t>(*fReader, "GenPart_mass");
        GenPart_pdgId            = new TTreeReaderArray<Int_t>(*fReader, "GenPart_pdgId");
        GenPart_status           = new TTreeReaderArray<Int_t>(*fReader, "GenPart_status");
        GenPart_statusFlags      = new TTreeReaderArray<Int_t>(*fReader, "GenPart_statusFlags");
        GenPart_genPartIdxMother = new TTreeReaderArray<Int_t>(*fReader, "GenPart_genPartIdxMother");
        GenMET_phi               = new TTreeReaderArray<Float_t>(*fReader, "GenMET_phi");
        GenMET_pt                = new TTreeReaderArray<Float_t>(*fReader, "GenMET_pt");
    }
    // LHE_HT will be loaded only for inclusive W and boosted W
    if (bIsMC && (sProcessName.find("WJetsToLNu") != std::string::npos) ) {
        LHE_HT = new TTreeReaderValue<Float_t>(*fReader, "LHE_HT");
    }
}

void Data::Clear() {
    // Clean up dynamically allocated members
    delete fChain;
    delete fReader;
    
    // Clean up all TTreeReader components
    delete GenWeight;
    delete Pileup_nPU;
    delete Pileup_nTrueInt;
    delete NPV;
    delete LHE_HT;
    delete nGenPart;
    delete GenPart_eta;
    delete GenPart_phi;
    delete GenPart_pt;
    delete GenPart_mass;
    delete GenPart_pdgId;
    delete GenPart_status;
    delete GenPart_statusFlags;
    delete GenPart_genPartIdxMother;
    delete GenMET_phi;
    delete GenMET_pt;
    delete L1PreFiringWeight_Nom;
    delete HLT_IsoMu24;
    if (HLT_IsoTkMu24) delete HLT_IsoTkMu24;
    delete HLT_IsoMu27;
    delete HLT_Mu50;
    if (HLT_TkMu50) delete HLT_TkMu50;
    if (HLT_TkMu100) delete HLT_TkMu100;
    if (HLT_OldMu100) delete HLT_OldMu100;
    delete Flag_goodVertices;
    delete Flag_globalSuperTightHalo2016Filter;
    delete Flag_HBHENoiseFilter;
    delete Flag_HBHENoiseIsoFilter;
    delete Flag_EcalDeadCellTriggerPrimitiveFilter;
    delete Flag_BadPFMuonFilter;
    delete Flag_BadPFMuonDzFilter;
    delete Flag_hfNoisyHitsFilter;
    // delete Flag_BadChargedCandidateFilter;
    delete Flag_eeBadScFilter;
    delete Flag_ecalBadCalibFilter;
    delete nMuon;
    delete Muon_pt;
    delete Muon_eta;
    delete Muon_phi;
    delete Muon_mass;
    delete Muon_nTrackerLayers;
    delete Muon_nStations;
    delete Muon_charge;
    delete Muon_tightId;
    delete Muon_mediumId;
    delete Muon_looseId;
    delete Muon_highPtId;
    delete Muon_isGlobal;
    delete Muon_isTracker;
    delete Muon_isPFcand;
    delete Muon_pfRelIso04_all;
    delete PuppiMET_phi;
    delete PuppiMET_pt;
    delete PuppiMET_sumEt;
}

Data::~Data() {
    Clear();
}