#ifndef PU_h
#define PU_h

#include "TFile.h"
#include "TH1.h"

#include <string>
#include <vector>
#include <iostream>

class PU {
    private :
        TFile* fDataPUFile = nullptr;
        TFile* fMCPUFile = nullptr;

        TH1D* hDataPUHist = nullptr;
        TH1D* hMCPUHist = nullptr;
        TH1D* hPUWeights = nullptr;

        std::string sEra;

        Bool_t bIsInit = false;

    public :
        PU(const std::string& era): sEra(era)
        {};
        virtual ~PU();

        void Init();
        void Clear();
        void PrintInitInfo();

        Double_t GetPUWeight(Float_t nTrueInt);
};

#endif
