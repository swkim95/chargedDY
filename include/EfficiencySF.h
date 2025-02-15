#ifndef EfficiencySF_h
#define EfficiencySF_h

// ROOT classes
#include "TFile.h"
#include "TH2F.h"
#include "TAxis.h"

// C++ classes
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

class EfficiencySF {
    private :
        
        TFile* fFile_ID = nullptr;
        TFile* fFile_Iso = nullptr;
        TFile* fFile_Trig = nullptr;

        TH2F* hHist_ID = nullptr;
        TH2F* hHist_Iso = nullptr;
        TH2F* hHist_Trig = nullptr;

        // Z peak study
        TH2F* hHist_Trig_Data = nullptr;
        TH2F* hHist_Trig_MC   = nullptr;

        TAxis* tAxis_ID_pt    = nullptr;
        TAxis* tAxis_ID_eta   = nullptr;
        TAxis* tAxis_Iso_pt   = nullptr;
        TAxis* tAxis_Iso_eta  = nullptr;
        TAxis* tAxis_Trig_pt  = nullptr;
        TAxis* tAxis_Trig_eta = nullptr;

        Double_t dValidPtMin_ID;
        Double_t dValidPtMax_ID;
        Double_t dValidEtaMin_ID;
        Double_t dValidEtaMax_ID;

        Double_t dValidPtMin_Iso;
        Double_t dValidPtMax_Iso;
        Double_t dValidEtaMin_Iso;
        Double_t dValidEtaMax_Iso;
        
        Double_t dValidPtMin_Trig;
        Double_t dValidPtMax_Trig;
        Double_t dValidEtaMin_Trig;
        Double_t dValidEtaMax_Trig;

        std::string sHistName_ID;
        std::string sHistName_Iso;
        std::string sHistName_Trig;

        std::string sHistName_Trig_Data;
        std::string sHistName_Trig_MC;

        Bool_t bIsInit = false;
        std::string sEra;

    public :
                
        EfficiencySF(const std::string& era, const std::string& histName_ID, const std::string& histName_Iso, const std::string& histName_Trig) :
            sEra(era), sHistName_ID(histName_ID), sHistName_Iso(histName_Iso), sHistName_Trig(histName_Trig) {
            sHistName_Trig_Data = sHistName_Trig + "_efficiencyData";
            sHistName_Trig_MC   = sHistName_Trig + "_efficiencyMC";
        };
        virtual ~EfficiencySF();

        void SetEra(const std::string& era) {sEra = era;}
        void SetHistName(const std::string& histName_ID, const std::string& histName_Iso, const std::string& histName_Trig) {
            sHistName_ID = histName_ID;
            sHistName_Iso = histName_Iso;
            sHistName_Trig = histName_Trig;
            sHistName_Trig_Data = sHistName_Trig + "_efficiencyData";
            sHistName_Trig_MC   = sHistName_Trig + "_efficiencyMC";
        }

        void Init();
        void Clear();
        void PrintInitInfo();

        std::vector<Double_t> GetEfficiency(Double_t pt, Double_t eta);
        std::vector<Double_t> GetZEfficiency(Double_t pt, Double_t eta);
};

#endif