#ifndef GetPtc_h
#define GetPtc_h

// DYanalysis classes
#include "Data.h"

// ROOT classes
#include "TLorentzVector.h"

// C++ classes
#include <iostream>
#include <vector>
#include <string>
#include <bitset>

// Class of a single gen particle
class GenPtcHolder {
    private :
        TLorentzVector GenPtcVec;
        Int_t iGenPtcIdx = -1;
        Int_t iGenPtcCharge = -1;
        Int_t iGenPtcPDGID = -1;
        Int_t iGenPtcStatus = -1;
        Int_t iGenPtcStatusFlags = -1;
        Int_t iGenPtcMotherIdx = -1;
        Int_t iGenPtcMotherPDGID = -1;
        Int_t iGenPtcMotherStatus = -1;

    public :
        GenPtcHolder(const TLorentzVector& vec, Int_t idx, Int_t charge, Int_t pdgId, Int_t status, Int_t statusFlags)
            : GenPtcVec(vec), iGenPtcIdx(idx), iGenPtcCharge(charge), iGenPtcPDGID(pdgId), iGenPtcStatus(status), iGenPtcStatusFlags(statusFlags)
        {};
        ~GenPtcHolder() {};

        // Setters
        void SetGenPtcIdx(Int_t idx) { iGenPtcIdx = idx; }
        void SetGenPtcCharge(Int_t charge) { iGenPtcCharge = charge; }
        void SetGenPtcPDGID(Int_t pdgId) { iGenPtcPDGID = pdgId; }
        void SetGenPtcStatus(Int_t status) { iGenPtcStatus = status; }
        void SetGenPtcStatusFlags(Int_t statusFlags) { iGenPtcStatusFlags = statusFlags; }
        void SetGenPtcMotherIdx(Int_t motherIdx) { iGenPtcMotherIdx = motherIdx; }
        void SetGenPtcMotherPDGID(Int_t motherPDGID) { iGenPtcMotherPDGID = motherPDGID; }
        void SetGenPtcMotherStatus(Int_t motherStatus) { iGenPtcMotherStatus = motherStatus; }

        // Getters
        Double_t Pt() { return GenPtcVec.Pt(); }
        Double_t Eta() { return GenPtcVec.Eta(); }
        Double_t Phi() { return GenPtcVec.Phi(); }
        Double_t M() { return GenPtcVec.M(); }
        Int_t Charge() { return iGenPtcCharge; }

        // Getters for GenPtcHolder
        const TLorentzVector& GetGenPtcVec() const { return GenPtcVec; }
        Int_t GetGenPtcIdx() { return iGenPtcIdx; }
        Int_t GetGenPtcPDGID() { return iGenPtcPDGID; }
        Int_t GetGenPtcStatus() { return iGenPtcStatus; }
        Int_t GetGenPtcStatusFlags() { return iGenPtcStatusFlags; }
        Int_t GetGenPtcMotherIdx() { return iGenPtcMotherIdx; }
        Int_t GetGenPtcMotherPDGID() { return iGenPtcMotherPDGID; }
        Int_t GetGenPtcMotherStatus() { return iGenPtcMotherStatus; }

        void PrintGenPtcInfo();
        void PrintGenPtcStatusFlags();

        // Getters for process flags
        Bool_t IsPrompt();
        Bool_t IsHardProcess();
        Bool_t IsFromHardProcess();
        Bool_t IsTauDecayProduct();
        Bool_t IsPromptTauDecayProduct();
        Bool_t IsDirectTauDecayProduct();
        Bool_t IsDirectPromptTauDecayProduct();

        // PDGID checkers
        // Elec : 11
        // Electron Neutrino : 14
        // Muon : 13
        // Muon Neutrino : 12
        // Tau : 15
        // Tau Neutrino : 16
        // Neutrino is also a lepton, but not included in the checkers below
        Bool_t IsLepton() { return ( std::abs(iGenPtcPDGID) == 11 || std::abs(iGenPtcPDGID) == 13 || std::abs(iGenPtcPDGID) == 15 ); }
        Bool_t IsNeutrino() { return ( std::abs(iGenPtcPDGID) == 12 || std::abs(iGenPtcPDGID) == 14 || std::abs(iGenPtcPDGID) == 16 ); }
        Bool_t IsElectron() { return ( std::abs(iGenPtcPDGID) == 11 ); }
        Bool_t IsMuon() { return ( std::abs(iGenPtcPDGID) == 13 ); }
        Bool_t IsTau() { return ( std::abs(iGenPtcPDGID) == 15 ); }
        Bool_t IsElectronNeutrino() { return ( std::abs(iGenPtcPDGID) == 12 ); }
        Bool_t IsMuonNeutrino() { return ( std::abs(iGenPtcPDGID) == 14 ); }
        Bool_t IsTauNeutrino() { return ( std::abs(iGenPtcPDGID) == 16 ); }
};

// Class of all gen particles in an event
class GenPtcs {
    private :
        std::vector<GenPtcHolder> vGenPtcVec;
        Data* cData;
        // Flags for the class
        Bool_t bIsInit = false;
        Bool_t bDoPatching = false;

        // Flags for the class
        // For All Gen-lv patching channel
        Int_t iFoundW = 0; // # of reconstructed Gen-lv W boson (l+nu)
        Int_t iFoundLepton = 0; // # of Gen-lv lepton
        Int_t iFoundNeutrino = 0; // # of Gen-lv neutrino
        Int_t iFoundMuon = 0; // # of Gen-lv muon
        Int_t iFoundMuonNeutrino = 0; // # of Gen-lv muon neutrino
        Int_t iIsWToMuNu = 0; // # of reconstructed Gen-lv W boson (mu+nu)
        Int_t iFoundTau = 0; // To find W to tau+nu to muon+nu+nu+nu
        Int_t iFoundTauNeutrino = 0; // To find W to tau+nu to muon+nu+nu+nu
        // For W->tau+nu channel only
        Int_t iFoundMuonFromTauDecay = 0; // find muon from tau decay
        Int_t iFoundMuonNeutrinoFromTauDecay = 0; // find muon neutrino from tau decay
        Int_t iIsWToTauNuToMuNu = 0; // find W->tau+nu->mu+nu+nu+nu

        // Finding Gen-lv muon and neutrinos
        TLorentzVector GenW;
        TLorentzVector LeptonFromW;
        TLorentzVector NeutrinoFromW;

        // Flags for the process
        Bool_t bIsInclusiveW = false;
        Bool_t bIsBoostedW = false;
        Bool_t bIsOffshellW = false;
        Bool_t bIsOffshellWToTauNu = false;

    public :
        GenPtcs(Data* data, Bool_t isInclusiveW, Bool_t isBoostedW, Bool_t isOffshellW, Bool_t isOffshellWToTauNu)
            : cData(data), bIsInclusiveW(isInclusiveW), bIsBoostedW(isBoostedW), bIsOffshellW(isOffshellW), bIsOffshellWToTauNu(isOffshellWToTauNu)
        {};
        ~GenPtcs() {};
        void Init();
        void Reset() {
            vGenPtcVec.clear();
            bIsInit = false;
            bDoPatching = false;
            iFoundW = 0;
            iFoundLepton = 0;
            iFoundNeutrino = 0;
            iFoundMuon = 0;
            iFoundMuonNeutrino = 0;
            iIsWToMuNu = 0;
            iFoundTau = 0;
            iFoundTauNeutrino = 0;
            iFoundMuonFromTauDecay = 0;
            iFoundMuonNeutrinoFromTauDecay = 0;
            iIsWToTauNuToMuNu = 0;
            GenW = TLorentzVector();
            LeptonFromW = TLorentzVector(); 
            NeutrinoFromW = TLorentzVector();
        };
        void PrintGenPtcChain();

        // Getters for GenPtcHolder
        std::vector<GenPtcHolder>& GetGenPtcs();
        std::vector<GenPtcHolder> GetGenMuons();
        std::vector<GenPtcHolder> GetGenElectrons();
        std::vector<GenPtcHolder> GetGenTaus();
        std::vector<GenPtcHolder> GetGenNeutrinos();

        // Getters
        UInt_t GetNGenPtcs() { return **(cData->nGenPart); }

        Int_t FoundW() {return iFoundW;}
        Int_t FoundLepton() {return iFoundLepton;}
        Int_t FoundNeutrino() {return iFoundNeutrino;}
        Int_t FoundMuon() {return iFoundMuon;}
        Int_t FoundMuonNeutrino() {return iFoundMuonNeutrino;}
        Int_t FoundTau() {return iFoundTau;}
        Int_t FoundTauNeutrino() {return iFoundTauNeutrino;}
        Int_t FoundMuonFromTauDecay() {return iFoundMuonFromTauDecay;}
        Int_t FoundMuonNeutrinoFromTauDecay() {return iFoundMuonNeutrinoFromTauDecay;}
        Int_t IsWToMuNu() {return iIsWToMuNu;}
        Int_t IsWToTauNuToMuNu() {return iIsWToTauNuToMuNu;}

        // Getters for process flags
        Bool_t IsInclusiveW() {return bIsInclusiveW;}
        Bool_t IsBoostedW() {return bIsBoostedW;}
        Bool_t IsOffshellW() {return bIsOffshellW;}
        Bool_t IsOffshellWToTauNu() {return bIsOffshellWToTauNu;}

        // Getters for Gen-lv muon and neutrinos
        const TLorentzVector& GetGenW() {return GenW;}
        const TLorentzVector& GetLeptonFromW() {return LeptonFromW;}
        const TLorentzVector& GetNeutrinoFromW() {return NeutrinoFromW;}

        // Gen-lv patching and muon filtering
        Bool_t PassGenPatching(Double_t HT_cut_high, Double_t W_mass_cut_high);
        Bool_t PassMuonFiltering();
};





#endif