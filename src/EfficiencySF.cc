#include "EfficiencySF.h"

EfficiencySF::~EfficiencySF() {
    Clear();
}

void EfficiencySF::Init() {
    if (bIsInit) {
        std::cerr << "[Warning] EfficiencySF::Init() - EfficiencySF is already initialized" << std::endl;
        return;
    }

    // Open files
    fFile_ID = new TFile(Form("../muonSF/%s_ID.root", sEra.c_str()), "READ");
    fFile_Iso = new TFile(Form("../muonSF/%s_Iso.root", sEra.c_str()), "READ");
    fFile_Trig = new TFile(Form("../muonSF/%s_Trig.root", sEra.c_str()), "READ");

    // Get histograms
    hHist_ID   = (TH2F*) fFile_ID->Get(sHistName_ID.c_str());
    hHist_Iso  = (TH2F*) fFile_Iso->Get(sHistName_Iso.c_str());
    hHist_Trig = (TH2F*) fFile_Trig->Get(sHistName_Trig.c_str());
    hHist_Trig_Data = (TH2F*) fFile_Trig->Get(sHistName_Trig_Data.c_str());
    hHist_Trig_MC   = (TH2F*) fFile_Trig->Get(sHistName_Trig_MC.c_str());

    // X-axis : Eta, Y-axis : pT
    tAxis_ID_pt    = hHist_ID  ->GetYaxis(); tAxis_ID_eta   = hHist_ID  ->GetXaxis();
    tAxis_Iso_pt   = hHist_Iso ->GetYaxis(); tAxis_Iso_eta  = hHist_Iso ->GetXaxis();
    tAxis_Trig_pt  = hHist_Trig->GetYaxis(); tAxis_Trig_eta = hHist_Trig->GetXaxis();

    // Get the corresponding pT and Eta range from histograms
    // Axis minimum : Low edge of the first bin
    // Axis maximum : Up edge of the last bin = Low edge of the overflow bin (last bin + 1)
    dValidPtMin_ID  = tAxis_ID_pt ->GetBinLowEdge(1);
    dValidPtMax_ID  = tAxis_ID_pt ->GetBinLowEdge(tAxis_ID_pt->GetNbins() + 1);
    dValidEtaMin_ID = tAxis_ID_eta->GetBinLowEdge(1);
    dValidEtaMax_ID = tAxis_ID_eta->GetBinLowEdge(tAxis_ID_eta->GetNbins() + 1);
    
    dValidPtMin_Iso  = tAxis_Iso_pt ->GetBinLowEdge(1);
    dValidPtMax_Iso  = tAxis_Iso_pt ->GetBinLowEdge(tAxis_Iso_pt->GetNbins() + 1);
    dValidEtaMin_Iso = tAxis_Iso_eta->GetBinLowEdge(1);
    dValidEtaMax_Iso = tAxis_Iso_eta->GetBinLowEdge(tAxis_Iso_eta->GetNbins() + 1);
    
    dValidPtMin_Trig  = tAxis_Trig_pt ->GetBinLowEdge(1);
    dValidPtMax_Trig  = tAxis_Trig_pt ->GetBinLowEdge(tAxis_Trig_pt->GetNbins() + 1);
    dValidEtaMin_Trig = tAxis_Trig_eta->GetBinLowEdge(1);
    dValidEtaMax_Trig = tAxis_Trig_eta->GetBinLowEdge(tAxis_Trig_eta->GetNbins() + 1);

    // Print initialization information
    PrintInitInfo();

    bIsInit = true;
}

void EfficiencySF::Clear() {
    // Do not manually delete the axes because they are owned by the histograms.
    delete hHist_ID;
    delete hHist_Iso;
    delete hHist_Trig;

    fFile_ID->Close();
    fFile_Iso->Close();
    fFile_Trig->Close();
}

void EfficiencySF::PrintInitInfo() {
    std::cout << "----------------------------------------------------------" << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - EfficiencySF is initialized" << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Era: " << sEra << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - ID File: " << fFile_ID->GetName() << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - ID Histogram Name: " << sHistName_ID << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Iso File: " << fFile_Iso->GetName() << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Iso Histogram Name: " << sHistName_Iso << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Trig File: " << fFile_Trig->GetName() << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Trig Histogram Name: " << sHistName_Trig << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - ID pT range: " << dValidPtMin_ID << " - " << dValidPtMax_ID << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - ID eta range: " << dValidEtaMin_ID << " - " << dValidEtaMax_ID << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Iso pT range: " << dValidPtMin_Iso << " - " << dValidPtMax_Iso << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Iso eta range: " << dValidEtaMin_Iso << " - " << dValidEtaMax_Iso << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Trig pT range: " << dValidPtMin_Trig << " - " << dValidPtMax_Trig << std::endl;
    std::cout << "[Info] EfficiencySF::PrintInitInfo() - Trig eta range: " << dValidEtaMin_Trig << " - " << dValidEtaMax_Trig << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
}   

std::vector<Double_t> EfficiencySF::GetEfficiency(Double_t pt, Double_t eta) {
    // Check if initialized
    if (!bIsInit) {
        std::cerr << "[ERROR] EfficiencySF::GetEfficiency() - Not initialized" << std::endl;
        return std::vector<Double_t> {0., 0., 0.};
    }

    // Set variables
    Double_t abs_eta = std::abs(eta);
    
    // Clamp : std::clamp(value, min, max) : return value if value is between min and max, otherwise return min or max
    // Clamp the pt and eta to the valid range
    // For lower edge, it is okay to find the bin with the lower edge
    // But for the upper edge, FindBin(upper edge) will return the overflow bin number so we subtract a small number from the upper edge
    // Double_t clampedPt_ID  = std::clamp(pt, dValidPtMin_ID, dValidPtMax_ID - dValidPtMax_ID/1000.);
    Double_t clampedPt_ID = (pt >= dValidPtMax_ID) ? dValidPtMax_ID - dValidPtMax_ID/1000. : (pt < dValidPtMin_ID) ? dValidPtMin_ID : pt;
    // Double_t clampedEta_ID = std::clamp(abs_eta, dValidEtaMin_ID, dValidEtaMax_ID - dValidEtaMax_ID/1000.);
    Double_t clampedEta_ID = (abs_eta >= dValidEtaMax_ID) ? dValidEtaMax_ID - dValidEtaMax_ID/1000. : (abs_eta < dValidEtaMin_ID) ? dValidEtaMin_ID : abs_eta;

    // Double_t clampedPt_Iso  = std::clamp(pt, dValidPtMin_Iso, dValidPtMax_Iso - dValidPtMax_Iso/1000.);
    // Double_t clampedEta_Iso = std::clamp(abs_eta, dValidEtaMin_Iso, dValidEtaMax_Iso - dValidEtaMax_Iso/1000.);
    Double_t clampedPt_Iso = (pt >= dValidPtMax_Iso) ? dValidPtMax_Iso - dValidPtMax_Iso/1000. : (pt < dValidPtMin_Iso) ? dValidPtMin_Iso : pt;
    Double_t clampedEta_Iso = (abs_eta >= dValidEtaMax_Iso) ? dValidEtaMax_Iso - dValidEtaMax_Iso/1000. : (abs_eta < dValidEtaMin_Iso) ? dValidEtaMin_Iso : abs_eta;

    // Double_t clampedPt_Trig  = std::clamp(pt, dValidPtMin_Trig, dValidPtMax_Trig - dValidPtMax_Trig/1000.);
    // Double_t clampedEta_Trig = std::clamp(abs_eta, dValidEtaMin_Trig, dValidEtaMax_Trig - dValidEtaMax_Trig/1000.);
    Double_t clampedPt_Trig = (pt >= dValidPtMax_Trig) ? dValidPtMax_Trig - dValidPtMax_Trig/1000. : (pt < dValidPtMin_Trig) ? dValidPtMin_Trig : pt;
    Double_t clampedEta_Trig = (abs_eta >= dValidEtaMax_Trig) ? dValidEtaMax_Trig - dValidEtaMax_Trig/1000. : (abs_eta < dValidEtaMin_Trig) ? dValidEtaMin_Trig : abs_eta;

    // Get the bin number using the clamped values
    Int_t bin_ID   = hHist_ID  ->FindBin(clampedEta_ID, clampedPt_ID);
    Int_t bin_Iso  = hHist_Iso ->FindBin(clampedEta_Iso, clampedPt_Iso);
    Int_t bin_Trig = hHist_Trig->FindBin(clampedEta_Trig, clampedPt_Trig);
    // Get the bin content
    Double_t eff_ID   = hHist_ID  ->GetBinContent(bin_ID);
    Double_t eff_Iso  = hHist_Iso ->GetBinContent(bin_Iso);
    Double_t eff_Trig = hHist_Trig->GetBinContent(bin_Trig);
    // For Trigger SF, if pt is underflow, efficiency is 0.
    // This is to use trigger efficiency after its turn-on curve
    if (pt < dValidPtMin_Trig) {
        eff_Trig = 0.;
    }
    
    // // For debugging
    // std::cout << "----------------------------------------------------------" << std::endl;
    // std::cout << "pt: " << pt << ", eta: " << eta << ", eff_ID: " << eff_ID << ", eff_Iso: " << eff_Iso << ", eff_Trig: " << eff_Trig << std::endl;
    // std::cout << "ID bin: " << bin_ID << ", clampedEta_ID: " << clampedEta_ID << ", clampedPt_ID: " << clampedPt_ID << std::endl;
    // std::cout << "Iso bin: " << bin_Iso << ", clampedEta_Iso: " << clampedEta_Iso << ", clampedPt_Iso: " << clampedPt_Iso << std::endl;
    // std::cout << "Trig bin: " << bin_Trig << ", clampedEta_Trig: " << clampedEta_Trig << ", clampedPt_Trig: " << clampedPt_Trig << std::endl;
    // std::cout << "----------------------------------------------------------" << std::endl;
    // return vector of efficiencies
    return std::vector<Double_t> {eff_ID, eff_Iso, eff_Trig};
}

std::vector<Double_t> EfficiencySF::GetZEfficiency(Double_t pt, Double_t eta) {
    // Check if initialized
    if (!bIsInit) {
        std::cerr << "[ERROR] EfficiencySF::GetZEfficiency() - Not initialized" << std::endl;
        return std::vector<Double_t> {0., 0., 0., 0.};
    }

    // Set variables
    Double_t abs_eta = std::abs(eta);
    
    // Clamp : std::clamp(value, min, max) : return value if value is between min and max, otherwise return min or max
    // Clamp the pt and eta to the valid range
    // For lower edge, it is okay to find the bin with the lower edge
    // But for the upper edge, FindBin(upper edge) will return the overflow bin number so we subtract a small number from the upper edge
    // Double_t clampedPt_ID  = std::clamp(pt, dValidPtMin_ID, dValidPtMax_ID - dValidPtMax_ID/1000.);
    Double_t clampedPt_ID = (pt >= dValidPtMax_ID) ? dValidPtMax_ID - dValidPtMax_ID/1000. : (pt < dValidPtMin_ID) ? dValidPtMin_ID : pt;
    // Double_t clampedEta_ID = std::clamp(abs_eta, dValidEtaMin_ID, dValidEtaMax_ID - dValidEtaMax_ID/1000.);
    Double_t clampedEta_ID = (abs_eta >= dValidEtaMax_ID) ? dValidEtaMax_ID - dValidEtaMax_ID/1000. : (abs_eta < dValidEtaMin_ID) ? dValidEtaMin_ID : abs_eta;

    // Double_t clampedPt_Iso  = std::clamp(pt, dValidPtMin_Iso, dValidPtMax_Iso - dValidPtMax_Iso/1000.);
    // Double_t clampedEta_Iso = std::clamp(abs_eta, dValidEtaMin_Iso, dValidEtaMax_Iso - dValidEtaMax_Iso/1000.);
    Double_t clampedPt_Iso = (pt >= dValidPtMax_Iso) ? dValidPtMax_Iso - dValidPtMax_Iso/1000. : (pt < dValidPtMin_Iso) ? dValidPtMin_Iso : pt;
    Double_t clampedEta_Iso = (abs_eta >= dValidEtaMax_Iso) ? dValidEtaMax_Iso - dValidEtaMax_Iso/1000. : (abs_eta < dValidEtaMin_Iso) ? dValidEtaMin_Iso : abs_eta;

    // Double_t clampedPt_Trig  = std::clamp(pt, dValidPtMin_Trig, dValidPtMax_Trig - dValidPtMax_Trig/1000.);
    // Double_t clampedEta_Trig = std::clamp(abs_eta, dValidEtaMin_Trig, dValidEtaMax_Trig - dValidEtaMax_Trig/1000.);
    Double_t clampedPt_Trig = (pt >= dValidPtMax_Trig) ? dValidPtMax_Trig - dValidPtMax_Trig/1000. : (pt < dValidPtMin_Trig) ? dValidPtMin_Trig : pt;
    Double_t clampedEta_Trig = (abs_eta >= dValidEtaMax_Trig) ? dValidEtaMax_Trig - dValidEtaMax_Trig/1000. : (abs_eta < dValidEtaMin_Trig) ? dValidEtaMin_Trig : abs_eta;

    // Get the bin number using the clamped values
    Int_t bin_ID   = hHist_ID  ->FindBin(clampedEta_ID, clampedPt_ID);
    Int_t bin_Iso  = hHist_Iso ->FindBin(clampedEta_Iso, clampedPt_Iso);
    Int_t bin_Trig_Data = hHist_Trig_Data->FindBin(clampedEta_Trig, clampedPt_Trig);
    Int_t bin_Trig_MC   = hHist_Trig_MC  ->FindBin(clampedEta_Trig, clampedPt_Trig);
    // Get the bin content
    Double_t eff_ID   = hHist_ID  ->GetBinContent(bin_ID);
    Double_t eff_Iso  = hHist_Iso ->GetBinContent(bin_Iso);
    Double_t eff_Trig_Data = hHist_Trig_Data->GetBinContent(bin_Trig_Data);
    Double_t eff_Trig_MC   = hHist_Trig_MC  ->GetBinContent(bin_Trig_MC);
    // For Trigger SF, if pt is underflow, efficiency is 0.
    // This is to use trigger efficiency after its turn-on curve
    if (pt < dValidPtMin_Trig) {
        eff_Trig_Data = 0.;
        eff_Trig_MC   = 0.;
    }
    
    // // For debugging
    // std::cout << "----------------------------------------------------------" << std::endl;
    // std::cout << "pt: " << pt << ", eta: " << eta << ", eff_ID: " << eff_ID << ", eff_Iso: " << eff_Iso << ", eff_Trig: " << eff_Trig << std::endl;
    // std::cout << "ID bin: " << bin_ID << ", clampedEta_ID: " << clampedEta_ID << ", clampedPt_ID: " << clampedPt_ID << std::endl;
    // std::cout << "Iso bin: " << bin_Iso << ", clampedEta_Iso: " << clampedEta_Iso << ", clampedPt_Iso: " << clampedPt_Iso << std::endl;
    // std::cout << "Trig bin: " << bin_Trig << ", clampedEta_Trig: " << clampedEta_Trig << ", clampedPt_Trig: " << clampedPt_Trig << std::endl;
    // std::cout << "----------------------------------------------------------" << std::endl;
    // return vector of efficiencies
    return std::vector<Double_t> {eff_ID, eff_Iso, eff_Trig_Data, eff_Trig_MC};
}