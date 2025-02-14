#ifndef Muon_h
#define Muon_h

// DYanalysis classes
#include "Data.h"

// ROOT classes
#include "TLorentzVector.h"

// C++ classes
#include <string>
#include <vector>
#include <iostream>

// Class of a single muon
class MuonHolder {
    private :
        // Basic infos
        TLorentzVector MuonOrgVec; // Muon fourvector without Rocco
        Int_t iMuonIdx = -1;
        Int_t iMuonCharge = -1;
        // Variables used for IDs
        Int_t iNStations = -1;
        Int_t iTrackerLayers = -1;
        // Isolation
        Float_t fPfRelIso03_all = -1;
        Float_t fPfRelIso03_chg = -1;
        Float_t fPfRelIso04_all = -1;
        Float_t fTkRelIso = -1; // dR 0.3 for highPt, trkIso/tunePpt
        // For TuneP pT
        Float_t fTunePRelPt = -1; // tunePpT / pt -> multiply with muon pT will return TuneP pT
        // Rochester correction
        Double_t dMuonRoccoSF = -1;
        // Efficiency SFs
        Float_t fMuonIDSF = -1;
        Float_t fMuonIsoSF = -1;
        Float_t fMuonTrigSF = -1;
        // What muon is this?
        Bool_t bIsGlobal = false;
        Bool_t bIsPFcand = false;
        Bool_t bIsStandalone = false;
        Bool_t bIsTracker = false;
        // Muon IDs
        Bool_t bLooseId = false;
        Bool_t bMediumId = false;
        Bool_t bMediumPromptId = false;
        Bool_t bSoftId = false;
        Bool_t bTightId = false;
        Bool_t bTrkHighPtId = false;
        Bool_t bGlbHighPtId = false;
        // True if passed obj selection
        Bool_t bTightObjSel = false; // Obj sel components : how to configure them?
        Bool_t bLooseObjSel = false; // Obj sel components : how to configure them?

    public :

        MuonHolder() {};
        ~MuonHolder() {};

        MuonHolder(const TLorentzVector& vec, Int_t idx, Int_t charge)
            : MuonOrgVec(vec), iMuonIdx(idx), iMuonCharge(charge)
        {};

        void SetIndex(Int_t idx) { iMuonIdx = idx; }
        void SetCharge(Int_t charge) { iMuonCharge = charge; }
        void SetNStations(Int_t nStations) { iNStations = nStations; }
        void SetTrackerLayers(Int_t trackerLayers) { iTrackerLayers = trackerLayers; }
        void SetPfRelIso03_all(Float_t iso) { fPfRelIso03_all = iso; }
        void SetPfRelIso03_chg(Float_t iso) { fPfRelIso03_chg = iso; }
        void SetPfRelIso04_all(Float_t iso) { fPfRelIso04_all = iso; }
        void SetTkRelIso(Float_t iso) { fTkRelIso = iso; }
        void SetTunePRelPt(Float_t pt) { fTunePRelPt = pt; }
        void SetRoccoSF(Double_t sf) { dMuonRoccoSF = sf; }
        void SetEfficiencySF(std::vector<double>& sf) { fMuonIDSF = sf[0]; fMuonIsoSF = sf[1]; fMuonTrigSF = sf[2]; }
        void SetIDSF(Double_t sf) { fMuonIDSF = sf; }
        void SetIsoSF(Double_t sf) { fMuonIsoSF = sf; }
        void SetTrigSF(Double_t sf) { fMuonTrigSF = sf; }

        // Set high-pT cut-based ID, 1 = tracker high pT, 2 = global high pT, which includes tracker high pT, else both are false
        void SetMuonHighPtId(UChar_t id) { 
            bTrkHighPtId = (id == 1);   
            bGlbHighPtId = (id == 2); 
        }
        void SetMuonType(Bool_t isGlobal, Bool_t isPFcand, Bool_t isStandalone, Bool_t isTracker) {
            bIsGlobal = isGlobal; bIsPFcand = isPFcand; bIsStandalone = isStandalone; bIsTracker = isTracker;
        }
        void SetMuonID(Bool_t looseId, Bool_t mediumId, Bool_t mediumPromptId, Bool_t softId, Bool_t tightId, UChar_t highPtId) {
            bLooseId = looseId; bMediumId = mediumId; bMediumPromptId = mediumPromptId; bSoftId = softId; bTightId = tightId;
            SetMuonHighPtId(highPtId);
        }
        void SetMuonID(Bool_t looseId, Bool_t mediumId, Bool_t tightId, UChar_t highPtId) {
            bLooseId = looseId; bMediumId = mediumId; bTightId = tightId;
            SetMuonHighPtId(highPtId);
        }
        void SetObjSel(Bool_t tightObjSel, Bool_t looseObjSel) {
            bTightObjSel = tightObjSel; bLooseObjSel = looseObjSel;
        }
        // Object selection
        Bool_t DoTightObjSel();
        Bool_t DoLooseObjSel();

        // Muon fourvector
        const TLorentzVector& GetMuonOrgVec() { return MuonOrgVec; }
        TLorentzVector GetMuonRoccoVec() { return dMuonRoccoSF * MuonOrgVec; }

        // Muon index
        Int_t GetIndex() { return iMuonIdx; }
        // Muon kinematics
        Double_t TunePPt() { return fTunePRelPt * MuonOrgVec.Pt(); }
        Double_t Pt() { return MuonOrgVec.Pt(); }
        Double_t Eta() { return MuonOrgVec.Eta(); }
        Double_t Phi() { return MuonOrgVec.Phi(); }
        Double_t M() { return MuonOrgVec.M(); }
        Int_t Charge() { return iMuonCharge; }
        // Muon HW infos
        Int_t GetNStations() { return iNStations; }
        Int_t GetTrackerLayers() { return iTrackerLayers; }
        // Isolation
        Float_t GetPfRelIso03_all() { return fPfRelIso03_all; }
        Float_t GetPfRelIso03_chg() { return fPfRelIso03_chg; }
        Float_t GetPfRelIso04_all() { return fPfRelIso04_all; }
        Float_t GetTkRelIso() { return fTkRelIso; }
        // Rochester correction
        Float_t GetRoccoSF() { return dMuonRoccoSF; }
        // Efficiency SFs
        Float_t GetIDSF() { return fMuonIDSF; }
        Float_t GetIsoSF() { return fMuonIsoSF; }
        Float_t GetTrigSF() { return fMuonTrigSF; }
        // Muon type
        Bool_t IsGlobal() { return bIsGlobal; }
        Bool_t IsPFcand() { return bIsPFcand; }
        Bool_t IsStandalone() { return bIsStandalone; }
        Bool_t IsTracker() { return bIsTracker; }
        // Muon IDs
        Bool_t IsLooseId() { return bLooseId; }
        Bool_t IsMediumId() { return bMediumId; }
        Bool_t IsMediumPromptId() { return bMediumPromptId; }
        Bool_t IsSoftId() { return bSoftId; }
        Bool_t IsTightId() { return bTightId; }
        Bool_t IsTrkHighPtId() { return bTrkHighPtId; }
        Bool_t IsGlbHighPtId() { return bGlbHighPtId; }
        // Object selection
        Bool_t PassTightObjSel() { return bTightObjSel; }
        Bool_t PassLooseObjSel() { return bLooseObjSel; }
};

// Class of all muons in an event
// Muons will be stored in a vector of MuonHolder
class Muons {
    private :
        std::vector<MuonHolder> vMuonVec;
        Data* cData;
        // Flags for the class
        Bool_t bIsInit = false;
        Bool_t bDidObjSel = false;

    public :
        Muons(Data* data) 
            : cData(data)
        {};
        ~Muons() {};

        void Init();
        void Reset() {
            vMuonVec.clear();
            bIsInit = false;
            bDidObjSel = false;
        };
        void DoObjSel();

        std::vector<MuonHolder>& GetMuons();
        std::vector<MuonHolder> GetTightMuons();
        std::vector<MuonHolder> GetLooseMuons();

        UInt_t GetNMuons() { return **(cData->nMuon); }
};

#endif