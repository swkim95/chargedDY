#ifndef MET_h
#define MET_h

#include "Data.h"

#include <string>
#include <utility>
#include <cmath>

#include "TMath.h"

class MET {
    private :
        Data* cData;
        Bool_t bIsInit = false;

        Float_t fMET_pt = -999;
        Float_t fMET_phi = -999;
        Float_t fMET_sumEt = -999;

        Float_t fPuppiMET_pt = -999;
        Float_t fPuppiMET_phi = -999;
        Float_t fPuppiMET_sumEt = -999;

    public :
        MET(Data* data) 
            : cData(data)
        {};
        ~MET() {};
        
        void Init();
        void Reset() {
            bIsInit = false;
            fMET_pt = -999;
            fMET_phi = -999;
            fMET_sumEt = -999;
            fPuppiMET_pt = -999;
            fPuppiMET_phi = -999;
            fPuppiMET_sumEt = -999;
        };
        
        Float_t GetMET_pt() { return fMET_pt; }
        Float_t GetMET_phi() { return fMET_phi; }
        Float_t GetMET_sumEt() { return fMET_sumEt; }

        Float_t GetPuppiMET_pt() { return fPuppiMET_pt; }
        Float_t GetPuppiMET_phi() { return fPuppiMET_phi; }
        Float_t GetPuppiMET_sumEt() { return fPuppiMET_sumEt; }

        std::pair<Double_t, Double_t> GetPFMETXYCorr(std::string processName, std::string era, Bool_t isMC, Int_t NPV);
};

#endif