#ifndef Data_h
#define Data_h

// ROOT classes
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TTreeReaderValue.h"
#include "TChain.h"

// C++ classes
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

class Data {
    private :
        std::string sProcessName;
        std::string sEra;
        std::string sInputFileList;
        Bool_t bIsMC;

        // TTreeReader components
        TChain* fChain = nullptr;
        TTreeReader* fReader = nullptr;

        Bool_t bIsInit = false;
        Long64_t nTotalEvents = 0;
        Bool_t bNoTkMu50 = false; // For 2016B_APV_ver2, 3 files missing HLT_TkMu50, use Mu50 instead

    public :
        Data(const std::string& processName, const std::string& era, const std::string& inputFileList, Bool_t isMC)
        : sProcessName(processName), sEra(era), sInputFileList(inputFileList), bIsMC(isMC)
        {};

        virtual ~Data();

        void Clear();
        void Init();
        void LoadBranches();
        void PrintInitInfo();

        Bool_t NoTkMu50() { return bNoTkMu50; }

        // Should be called after Init()
        Long64_t GetTotalEvents() { return nTotalEvents; }
        Bool_t ReadNextEntry() {
            if (!bIsInit) {
                std::cerr << "[ERROR] Data::ReadNextEntry() - Data is not initialized" << std::endl;
                return false;
            }
            return fReader->Next();
        }

        // Getters
        TChain*      GetChain()  { return fChain; }
        TTreeReader* GetReader() { return fReader; }

        // Branches to load from Ntuple
        // Content Declaration
        // Generator level weight
        TTreeReaderValue<Float_t>* GenWeight = nullptr;
        // Pileup info
        TTreeReaderValue<Int_t>* Pileup_nPU = nullptr;
        TTreeReaderValue<Float_t>* Pileup_nTrueInt = nullptr;
        TTreeReaderValue<Int_t>* NPV = nullptr;
        // LHE info
        TTreeReaderValue<Float_t>* LHE_HT = nullptr;
        // GenPart info
        TTreeReaderValue<UInt_t>*  nGenPart = nullptr;
        TTreeReaderArray<Float_t>* GenPart_eta = nullptr;
        TTreeReaderArray<Float_t>* GenPart_phi = nullptr;
        TTreeReaderArray<Float_t>* GenPart_pt = nullptr;
        TTreeReaderArray<Float_t>* GenPart_mass = nullptr;
        TTreeReaderArray<Int_t>*   GenPart_pdgId = nullptr;
        TTreeReaderArray<Int_t>*   GenPart_status = nullptr;
        TTreeReaderArray<Int_t>*   GenPart_statusFlags = nullptr;
        TTreeReaderArray<Int_t>*   GenPart_genPartIdxMother = nullptr;
        // GenMET info
        TTreeReaderArray<Float_t>* GenMET_phi = nullptr;
        TTreeReaderArray<Float_t>* GenMET_pt = nullptr;
        // L1 pre-firing weight
        TTreeReaderValue<Float_t>* L1PreFiringWeight_Nom = nullptr;
        // HLT
        TTreeReaderValue<Bool_t>* HLT_IsoMu24 = nullptr;
        TTreeReaderValue<Bool_t>* HLT_IsoTkMu24 = nullptr;
        TTreeReaderValue<Bool_t>* HLT_IsoMu27 = nullptr;
        // HighPt trigger
        // For 2016APV: Mu50 || TkMu50 (except for 3 files in 2016B_APV_ver2)
        // For 3 files in 2016B_APV_ver2: Mu50
        // For 2016: Mu50 || TkMu50
        // For 2017: Mu50 || TkMu100 || OldMu100 (except for 2017B)
        // For 2017B: Mu50
        // For 2018: Mu50 || TkMu100 || OldMu100
        TTreeReaderValue<Bool_t>* HLT_TkMu50 = nullptr; // 2016APV, 2016 (except for 3 files in 2016B_APV_ver2)
        TTreeReaderValue<Bool_t>* HLT_Mu50 = nullptr; // 2016APV, 2016, 2017, 2018
        TTreeReaderValue<Bool_t>* HLT_TkMu100 = nullptr; // 2017, 2018 (except for 2017B)
        TTreeReaderValue<Bool_t>* HLT_OldMu100 = nullptr; // 2017, 2018 (except for 2017B)

        // Noise filter
        // Ref: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETOptionalFiltersRun2#UL_data
        // For 2016: Do not use Flag_ecalBadCalibFilter, Flag_BadChargedCandidateFilter
        // For 2017, 2018: Do not use Flag_BadChargedCandidateFilter
        TTreeReaderValue<Bool_t>* Flag_goodVertices = nullptr;
        TTreeReaderValue<Bool_t>* Flag_globalSuperTightHalo2016Filter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_HBHENoiseFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_HBHENoiseIsoFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_EcalDeadCellTriggerPrimitiveFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_BadPFMuonFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_BadPFMuonDzFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_hfNoisyHitsFilter = nullptr;
        // TTreeReaderValue<Bool_t>* Flag_BadChargedCandidateFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_eeBadScFilter = nullptr;
        TTreeReaderValue<Bool_t>* Flag_ecalBadCalibFilter = nullptr;
        // Muon info
        TTreeReaderValue<UInt_t>*  nMuon = nullptr;
        TTreeReaderArray<Float_t>* Muon_pt = nullptr;
        TTreeReaderArray<Float_t>* Muon_eta = nullptr;
        TTreeReaderArray<Float_t>* Muon_phi = nullptr;
        TTreeReaderArray<Float_t>* Muon_mass = nullptr;
        TTreeReaderArray<Int_t>*   Muon_nTrackerLayers = nullptr;
        TTreeReaderArray<Int_t>*   Muon_nStations = nullptr;
        TTreeReaderArray<Int_t>*   Muon_charge = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_tightId = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_mediumId = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_looseId = nullptr;
        TTreeReaderArray<UChar_t>* Muon_highPtId = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isGlobal = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isStandalone = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isTracker = nullptr;
        TTreeReaderArray<Bool_t>*  Muon_isPFcand = nullptr;
        TTreeReaderArray<Float_t>* Muon_tkRelIso = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso03_all = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso03_chg = nullptr;
        TTreeReaderArray<Float_t>* Muon_pfRelIso04_all = nullptr;
        TTreeReaderArray<Float_t>* Muon_tunepRelPt = nullptr;

        // Electron info
        TTreeReaderValue<UInt_t>*  nElectron = nullptr;
        TTreeReaderArray<Float_t>* Electron_pt = nullptr;
        TTreeReaderArray<Float_t>* Electron_eta = nullptr;
        TTreeReaderArray<Float_t>* Electron_phi = nullptr;
        TTreeReaderArray<Float_t>* Electron_mass = nullptr;
        TTreeReaderArray<Int_t>*   Electron_charge = nullptr;
        TTreeReaderArray<Float_t>* Electron_deltaEtaSC = nullptr;
        TTreeReaderArray<Int_t>*   Electron_cutBased = nullptr;

        // MET info
        TTreeReaderValue<Float_t>* MET_phi = nullptr;
        TTreeReaderValue<Float_t>* MET_pt = nullptr;
        TTreeReaderValue<Float_t>* MET_sumEt = nullptr;
        
        // PUPPI MET info
        TTreeReaderValue<Float_t>* PuppiMET_phi = nullptr;
        TTreeReaderValue<Float_t>* PuppiMET_pt = nullptr;
        TTreeReaderValue<Float_t>* PuppiMET_sumEt = nullptr;
};

#endif