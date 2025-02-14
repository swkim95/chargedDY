#ifndef DYanalyzer_h
#define DYanalyzer_h

// DYanalyzer classes
#include "Data.h"
#include "PU.h"
#include "EfficiencySF.h"
#include "RoccoR.h"
#include "GenPtc.h"
#include "Muon.h"
#include "Electron.h"
#include "MET.h"

// ROOT classes
#include "TRandom.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"

// C++ classes
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <iomanip>

class DYanalyzer {
    private :
        // Classes that will be created only once, at the beginning of the analysis
        Data* cData; // Class for loading Ntuple
        PU* cPU; // Class for loading PU files and calculating PU weights
        EfficiencySF* cEfficiencySF; // Class for loading efficiency SF files and calculating SFs
        RoccoR* cRochesterCorrection; // Class for loading Rochester correction file and applying correction
        
        // Flags for the class
        std::string sInputFileList;
        std::string sEra;
        std::string sProcessName;
        std::string sRoccoFileName;
        std::string sHistName_ID;
        std::string sHistName_Iso;
        std::string sHistName_Trig;

        Bool_t bIsInit = false;

        Bool_t bIsMC = false;
        Bool_t bDoGenPatching = false;
        Bool_t bDoPUCorrection = false;
        Bool_t bDoL1PreFiringCorrection = false;
        Bool_t bDoIDSF = false;
        Bool_t bDoIsoSF = false;
        Bool_t bDoTrigSF = false;
        Bool_t bDoRocco = false;

        // Check process name and determine whether to perform Gen-lv patching
        Bool_t bIsInclusiveW = false;
        Bool_t bIsBoostedW = false;
        Bool_t bIsOffshellW = false;
        Bool_t bIsOffshellWToTauNu = false;

        // Default HT cut and W mass cut
        Double_t dHT_cut_high = 1e9; //Default to infinity
        Double_t dW_mass_cut_low = 0.;
        Double_t dW_mass_cut_high = 1e9; //Default to infinity

        Long64_t nTotalEvents = 0;

        Double_t dSumOfGenEvtWeight = 0;

    public :
        DYanalyzer( const std::string& inputFileList, const std::string& processName, const std::string& era,
                    const std::string& HistName_ID, const std::string& HistName_Iso, const std::string& HistName_Trig,
                    const std::string& RoccoFileName,
                    Bool_t isMC, Bool_t doPUCorrection, Bool_t doL1PreFiringCorrection, Bool_t doRocco, Bool_t bDoIDSF, Bool_t bDoIsoSF, Bool_t bDoTrigSF)
            :   sInputFileList(inputFileList), sProcessName(processName), sEra(era), 
                sHistName_ID(HistName_ID), sHistName_Iso(HistName_Iso), sHistName_Trig(HistName_Trig),
                sRoccoFileName(RoccoFileName),
                bIsMC(isMC), bDoPUCorrection(doPUCorrection), bDoL1PreFiringCorrection(doL1PreFiringCorrection), bDoRocco(doRocco), bDoIDSF(bDoIDSF), bDoIsoSF(bDoIsoSF), bDoTrigSF(bDoTrigSF)
        {};
        virtual ~DYanalyzer();

        // Initialize classes
        void Init();
        // Clear all pointers
        void Clear();
        // Declare histograms
        void DeclareHistograms();
        // Run event loop
        void Analyze();
        void Analyze_Z() {}; // TODO: For Z peak mass study
        // Print initialization information
        void PrintInitInfo();
        // Check process name and determine whether to perform Gen-lv patching
        void CheckGenPatching();
        // Simple utility to print progress
        void PrintProgress(const int currentStep);
        // Write histograms to file
        void WriteHistograms(TFile* f_output);

        // Getters
        Bool_t IsMC() {return bIsMC;}
        Bool_t DoPUCorrection() {return bDoPUCorrection;}
        Bool_t DoL1PreFiringCorrection() {return bDoL1PreFiringCorrection;}
        Bool_t DoIDSF() {return bDoIDSF;}
        Bool_t DoIsoSF() {return bDoIsoSF;}
        Bool_t DoTrigSF() {return bDoTrigSF;}
        Bool_t DoRocco() {return bDoRocco;}
        Bool_t DoGenPatching() {return bDoGenPatching;}

        Bool_t IsInclusiveW() {return bIsInclusiveW;}
        Bool_t IsBoostedW() {return bIsBoostedW;}
        Bool_t IsOffshellW() {return bIsOffshellW;}
        Bool_t IsOffshellWToTauNu() {return bIsOffshellWToTauNu;}

        Double_t GetHT_cut_high() {return dHT_cut_high;}
        Double_t GetW_mass_cut_high() {return dW_mass_cut_high;}

        Long64_t GetTotalEvents() {return nTotalEvents;}
        Double_t GetSumOfGenEvtWeight() {return dSumOfGenEvtWeight;}

        // Getters for classes
        Data& GetData() {return *cData;}
        PU& GetPU() {return *cPU;}
        EfficiencySF& GetEffSF() {return *cEfficiencySF;}
        RoccoR& GetRocco() {return *cRochesterCorrection;}

        // Setters
        void SetDoPUCorrection(Bool_t doPUCorrection) {bDoPUCorrection = doPUCorrection;}
        void SetDoL1PreFiringCorrection(Bool_t doL1PreFiringCorrection) {bDoL1PreFiringCorrection = doL1PreFiringCorrection;}
        void SetDoIDSF(Bool_t doIDSF) {bDoIDSF = doIDSF;}
        void SetDoIsoSF(Bool_t doIsoSF) {bDoIsoSF = doIsoSF;}
        void SetDoTrigSF(Bool_t doTrigSF) {bDoTrigSF = doTrigSF;}
        void SetDoRocco(Bool_t doRocco) {bDoRocco = doRocco;}
        void SetDoGenPatching(Bool_t doGenPatching) {bDoGenPatching = doGenPatching;}

        ////////////////////////////////////////////////////////////
        //////////////////////// Histograms ////////////////////////
        ////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////
        // GenLevel event weights, before and after each correction
        ////////////////////////////////////////////////////////////
        TH1D* hGenEvtWeight;

        ////////////////////////////////////////////////////////////
        // Before event selection
        ////////////////////////////////////////////////////////////

        // GenLevel Object histograms
        TH1D* hGen_Muon_pT;
        TH1D* hGen_Muon_phi;
        TH1D* hGen_Muon_eta;

        TH1D* hGen_Nu_pT;
        TH1D* hGen_Nu_phi;
        TH1D* hGen_Nu_eta;

        TH1D* hGen_MET_phi;
        TH1D* hGen_MET_pT;

        // For GenLevel W decaying to muon and neutrino
        TH1D* hGen_WToMuNu_pT;
        TH1D* hGen_WToMuNu_eta;
        TH1D* hGen_WToMuNu_phi;
        TH1D* hGen_WToMuNu_mass;
        TH1D* hGen_WToMuNu_MT;

        // For GenLevel inclusive decaying W
        TH1D* hGen_W_pT;
        TH1D* hGen_W_eta;
        TH1D* hGen_W_phi;
        TH1D* hGen_W_mass;
        TH1D* hGen_W_MT;

        // For LHE HT
        TH1D* hLHE_HT;

        // Object histograms
        TH1D* hMuon_pT;
        TH1D* hMuon_phi;
        TH1D* hMuon_eta;
        TH1D* hMuon_mass;

        TH1D* hMET_phi;
        TH1D* hMET_pT;
        TH1D* hMET_sumET;

        TH1D* hPFMET_phi;
        TH1D* hPFMET_pT;
        TH1D* hPFMET_sumET;

        TH1D* hPFMET_corr_phi;
        TH1D* hPFMET_corr_pT;
        TH1D* hPFMET_corr_sumET;

        // Reconstructed W histograms
        TH1D* hDeltaPhi_Mu_MET;
        TH1D* hW_MT;

        TH1D* hDeltaPhi_Mu_PFMET;
        TH1D* hW_MT_PFMET;

        TH1D* hDeltaPhi_Mu_PFMET_corr;
        TH1D* hW_MT_PFMET_corr;

        // For NPV, NPU, NTrueInt before event selection
        // For NTrueInt -> Only present in MC
        TH1D* hNPV;
        TH1D* hNPU;
        TH1D* hNTrueInt;

        ////////////////////////////////////////////////////////////
        // After event selection 
        ////////////////////////////////////////////////////////////

        // GenLevel Object histograms
        TH1D* hGen_Muon_pT_after;
        TH1D* hGen_Muon_phi_after;
        TH1D* hGen_Muon_eta_after;

        TH1D* hGen_Nu_pT_after;
        TH1D* hGen_Nu_phi_after;
        TH1D* hGen_Nu_eta_after;

        TH1D* hGen_MET_phi_after;
        TH1D* hGen_MET_pT_after;

        // For GenLevel W decaying to muon and neutrino
        TH1D* hGen_WToMuNu_pT_after;
        TH1D* hGen_WToMuNu_eta_after;
        TH1D* hGen_WToMuNu_phi_after;
        TH1D* hGen_WToMuNu_mass_after;
        TH1D* hGen_WToMuNu_MT_after;

        // For GenLevel inclusive decaying W
        // This cannot exist because after event selection, muon filtering is already done
        TH1D* hGen_W_pT_after;
        TH1D* hGen_W_eta_after;
        TH1D* hGen_W_phi_after;
        TH1D* hGen_W_mass_after;
        TH1D* hGen_W_MT_after;

        // For LHE HT
        TH1D* hLHE_HT_after;

        // Object histograms
        TH1D* hMuon_pT_after;
        TH1D* hMuon_phi_after;
        TH1D* hMuon_eta_after;
        TH1D* hMuon_mass_after;

        TH1D* hMET_phi_after;
        TH1D* hMET_pT_after;
        TH1D* hMET_sumET_after;

        TH1D* hPFMET_phi_after;
        TH1D* hPFMET_pT_after;
        TH1D* hPFMET_sumET_after;

        TH1D* hPFMET_corr_phi_after;
        TH1D* hPFMET_corr_pT_after;
        TH1D* hPFMET_corr_sumET_after;

        // Reconstructed W histograms
        TH1D* hDeltaPhi_Mu_MET_after;
        TH1D* hW_MT_after;

        TH1D* hDeltaPhi_Mu_PFMET_after;
        TH1D* hW_MT_PFMET_after;

        TH1D* hDeltaPhi_Mu_PFMET_corr_after;
        TH1D* hW_MT_PFMET_corr_after;

        // For NPV, NPU, NTrueInt before event selection
        // For NTrueInt -> Only present in MC
        TH1D* hNPV_after;
        TH1D* hNPU_after;
        TH1D* hNTrueInt_after;

        ////////////////////////////////////////////////////////////
        // For Z peak mass study
        ////////////////////////////////////////////////////////////

        TH1D* hDilepton_org_mass;
        TH1D* hDilepton_rocco_mass;

        TH1D* hDilepton_org_mass_after;
        TH1D* hDilepton_rocco_mass_after;
};

#endif