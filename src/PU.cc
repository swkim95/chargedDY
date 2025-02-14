#include "PU.h"

PU::~PU() {
    Clear();
}

void PU::Clear() {
    delete hDataPUHist;
    delete hMCPUHist;
    delete hPUWeights;

    fDataPUFile->Close();
    fMCPUFile->Close();
}

void PU::Init() {
    if (bIsInit) {
        std::cerr << "PU::Init() - PU is already initialized" << std::endl;
        return;
    };

    // 2016APV and 2016 use same PU scenario
    if (sEra.find("2016") != std::string::npos)
        sEra = "2016";

    fDataPUFile = TFile::Open( ("../pileup/PileupHistogram-goldenJSON-13tev-" + sEra + "-69200ub-99bins.root").c_str() );
    fMCPUFile = TFile::Open( ("../pileup/pileup_" + sEra + "_UL_MC_99bins.root").c_str() );

    hDataPUHist = (TH1D*)fDataPUFile->Get("pileup");
    hMCPUHist = (TH1D*)fMCPUFile->Get("pileup");

    hPUWeights = static_cast<TH1D*>(hDataPUHist->Clone());
    hPUWeights->Scale(1. / hDataPUHist->Integral()); // Normalize data PU hist
    
    hPUWeights->Divide(hMCPUHist); // Divide data PU hist by MC PU hist

    PrintInitInfo();

    bIsInit = true;
}

void PU::PrintInitInfo() {
    std::cout << "-----------------------------------------------" << std::endl;
    std::cout << "[Info] PU::PrintInitInfo() - PU is initialized" << std::endl;
    std::cout << "[Info] PU::PrintInitInfo() - Era: " << sEra << std::endl;
    std::cout << "[Info] PU::PrintInitInfo() - Data PU file: " << fDataPUFile->GetName() << std::endl;
    std::cout << "[Info] PU::PrintInitInfo() - MC PU file: " << fMCPUFile->GetName() << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;
}

Double_t PU::GetPUWeight(Float_t nTrueInt) {
    if (!bIsInit) {
        std::cerr << "PU::GetPUWeight() - PU is not initialized" << std::endl;
        return 0;
    }

    // Check if nTrueInt is within valid range
    Float_t fPUWeight_min = hPUWeights->GetBinLowEdge(1);
    Float_t fPUWeight_max = hPUWeights->GetBinLowEdge(hPUWeights->GetNbinsX() + 1);
    if (nTrueInt < fPUWeight_min || nTrueInt >= fPUWeight_max) {
        std::cerr << "PU::GetPUWeight() - True interaction number out of range" << std::endl;
        return 0;
    }

    Int_t bin = hPUWeights->GetXaxis()->FindBin(nTrueInt);
    return hPUWeights->GetBinContent(bin);
}