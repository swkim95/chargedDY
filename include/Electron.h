#ifndef Electron_h
#define Electron_h

// DYanalysis classes
#include "Data.h"

// ROOT classes
#include "TLorentzVector.h"

// C++ classes
#include <string>
#include <vector>
#include <iostream>

class ElectronHolder {
    private :
        TLorentzVector ElectronOrgVec;
        Int_t iElectronIdx = -1;
        Int_t iElectronCharge = -1;
        // cut-based ID Fall17 V2 (0:fail, 1:veto, 2:loose, 3:medium, 4:tight)
        Int_t iElectronCutBased = -1;
        // deltaEtaSC + ele.Eta() == eta of the SC
        Float_t fElectronDeltaEtaSC = -1;

        Bool_t bLooseId = false;
        Bool_t bMediumId = false;
        Bool_t bTightId = false;

        Bool_t bPassLooseObjSel = false;

    public :
        ElectronHolder() {};
        ~ElectronHolder() {};

        ElectronHolder(const TLorentzVector& vec, Int_t idx, Int_t charge)
            : ElectronOrgVec(vec), iElectronIdx(idx), iElectronCharge(charge)
        {};

        void SetDeltaEtaSC(Float_t deltaEtaSC) { fElectronDeltaEtaSC = deltaEtaSC; }
        void SetCutBasedIds(Int_t cutBased) {
            iElectronCutBased = cutBased;
            bLooseId = (cutBased >= 2);
            bMediumId = (cutBased >= 3);
            bTightId = (cutBased == 4);
        }
        void SetObjSel(Bool_t passLooseObjSel) { bPassLooseObjSel = passLooseObjSel; }

        Bool_t DoLooseObjSel();
        
        // Getters
        const TLorentzVector& GetElectronOrgVec() { return ElectronOrgVec; }

        Int_t GetIndex() { return iElectronIdx; }
        Int_t GetCharge() { return iElectronCharge; }
        Int_t GetCutBased() { return iElectronCutBased; }
        Float_t GetDeltaEtaSC() { return fElectronDeltaEtaSC; }
        // IDs
        Bool_t IsLooseId() { return bLooseId; }
        Bool_t IsMediumId() { return bMediumId; }
        Bool_t IsTightId() { return bTightId; }
        // Object selection
        Bool_t PassLooseObjSel() { return bPassLooseObjSel; }
};

class Electrons {
    private :
        std::vector<ElectronHolder> vElectronVec;
        Data* cData;
        // Flags for the class
        Bool_t bIsInit = false;
        Bool_t bDidObjSel = false;

    public :
        Electrons(Data* data) 
            : cData(data)
        {};
        ~Electrons() {};

        void Init();
        void Reset() {
            vElectronVec.clear();
            bIsInit = false;
            bDidObjSel = false;
        };
        void DoObjSel();
        
        std::vector<ElectronHolder>& GetElectrons();
        std::vector<ElectronHolder> GetLooseElectrons();

        UInt_t GetNElectrons() { return **(cData->nElectron); }
};

#endif
