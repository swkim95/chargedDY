#include "MET.h"

void MET::Init() {
    if (bIsInit) {
        std::cerr << "[Warning] MET::Init() - MET is already initialized" << std::endl;
        return;
    }

    fMET_pt = **(cData->MET_pt);
    fMET_phi = **(cData->MET_phi);
    fMET_sumEt = **(cData->MET_sumEt);

    fPuppiMET_pt = **(cData->PuppiMET_pt);
    fPuppiMET_phi = **(cData->PuppiMET_phi);
    fPuppiMET_sumEt = **(cData->PuppiMET_sumEt);

    bIsInit = true;
}

// PF MET xy-Shift Correction
// Following code: https://lathomas.web.cern.ch/METStuff/XYCorrections/XYMETCorrection_withUL17andUL18andUL16.h
// Use offlineSlimmedPrimaryVertices as input npv (ref: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETRun2Corrections#xy_Shift_Correction_MET_phi_modu)
// Ref1: https://github.com/cms-sw/cmssw/blob/6177d0bc79e19c4308339bbd745bad4cb989237c/PhysicsTools/NanoAOD/plugins/VertexTableProducer.cc
// Ref2: https://github.com/cms-sw/cmssw/blob/6177d0bc79e19c4308339bbd745bad4cb989237c/PhysicsTools/NanoAOD/python/vertices_cff.py 
// -> pvTable->addColumnValue<uint8_t>("npvs", pvsIn->size(), "total number of reconstructed primary vertices"); // Ref1
// -> auto pvsIn = iEvent.getHandle(pvs_); // Ref1
// -> pvs_(consumes<std::vector<reco::Vertex>>(params.getParameter<edm::InputTag>("pvSrc"))), // Ref1
// -> pvSrc = cms.InputTag("offlineSlimmedPrimaryVertices"), // Ref2
// Therefore, offlineSlimmedPrimaryVertices == PV_npvs in NanoAOD
std::pair<Double_t, Double_t> MET::GetPFMETXYCorr(std::string processName, std::string era, Bool_t isMC, Int_t n_PV) {

    if (!bIsInit) {
        std::cerr << "[Warning] MET::GetPFMETXYCorr() - MET is not initialized" << std::endl;
        return std::make_pair(-999, -999);
    }
    
    Int_t NPV = n_PV; 
    if (NPV > 100) NPV = 100; 

    Double_t METxcorr = 0.;
    Double_t METycorr = 0.;

    // Data 2016APV (B~F)
    if ( (!isMC) && (processName == "SingleMuon_Run2016B_APV_ver2") ) { METxcorr = -(-0.0214894*NPV +-0.188255); METycorr = -(0.0876624*NPV +0.812885);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016C_APV") )      { METxcorr = -(-0.032209*NPV +0.067288);   METycorr = -(0.113917*NPV +0.743906);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016D_APV") )      { METxcorr = -(-0.0293663*NPV +0.21106);   METycorr = -(0.11331*NPV +0.815787);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016E_APV") )      { METxcorr = -(-0.0132046*NPV +0.20073);   METycorr = -(0.134809*NPV +0.679068);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016F_APV") )      { METxcorr = -(-0.0543566*NPV +0.816597);  METycorr = -(0.114225*NPV +1.17266);}
    // Data 2016 (F, G, H)
    if ( (!isMC) && (processName == "SingleMuon_Run2016F") ) { METxcorr = -(0.134616*NPV +-0.89965); METycorr = -(0.0397736*NPV +1.0385);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016G") ) { METxcorr = -(0.121809*NPV +-0.584893); METycorr = -(0.0558974*NPV +0.891234);}
    if ( (!isMC) && (processName == "SingleMuon_Run2016H") ) { METxcorr = -(0.0868828*NPV +-0.703489); METycorr = -(0.0888774*NPV +0.902632);}
    // Data 2017 (B~F)
    if ( (!isMC) && (processName == "SingleMuon_Run2017B") ) { METxcorr = -(-0.211161*NPV +0.419333); METycorr = -(0.251789*NPV +-1.28089);}
    if ( (!isMC) && (processName == "SingleMuon_Run2017C") ) { METxcorr = -(-0.185184*NPV +-0.164009); METycorr = -(0.200941*NPV +-0.56853);}
    if ( (!isMC) && (processName == "SingleMuon_Run2017D") ) { METxcorr = -(-0.201606*NPV +0.426502); METycorr = -(0.188208*NPV +-0.58313);}
    if ( (!isMC) && (processName == "SingleMuon_Run2017E") ) { METxcorr = -(-0.162472*NPV +0.176329); METycorr = -(0.138076*NPV +-0.250239);}
    if ( (!isMC) && (processName == "SingleMuon_Run2017F") ) { METxcorr = -(-0.210639*NPV +0.72934);  METycorr = -(0.198626*NPV +1.028);}
    // Data 2018 (A~D)
    if ( (!isMC) && (processName == "SingleMuon_Run2018A") ) { METxcorr = -(0.263733*NPV +-1.91115); METycorr = -(0.0431304*NPV +-0.112043);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018B") ) { METxcorr = -(0.400466*NPV +-3.05914); METycorr = -(0.146125*NPV +-0.533233);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018C") ) { METxcorr = -(0.430911*NPV +-1.42865); METycorr = -(0.0620083*NPV +-1.46021);}
    if ( (!isMC) && (processName == "SingleMuon_Run2018D") ) { METxcorr = -(0.457327*NPV +-1.56856); METycorr = -(0.0684071*NPV +-0.928372);}
    // MC
    if (isMC && era == "2016APV") { METxcorr = -(-0.188743*NPV +0.136539);  METycorr = -(0.0127927*NPV +0.117747); }
    if (isMC && era == "2016")    { METxcorr = -(-0.153497*NPV +-0.231751); METycorr = -(0.00731978*NPV +0.243323); }
    if (isMC && era == "2017")    { METxcorr = -(-0.300155*NPV +1.90608);   METycorr = -(0.300213*NPV +-2.02232); }
    if (isMC && era == "2018")    { METxcorr = -(-0.183518*NPV +0.546754);  METycorr = -(0.192263*NPV +-0.42121); }
    
    Double_t CorrectedMET_x = fMET_pt *std::cos(fMET_phi) + METxcorr;
    Double_t CorrectedMET_y = fMET_pt *std::sin(fMET_phi) + METycorr;

    Double_t CorrectedMET = std::sqrt(CorrectedMET_x*CorrectedMET_x + CorrectedMET_y*CorrectedMET_y);
    Double_t CorrectedMETPhi;

    if     (CorrectedMET_x==0 && CorrectedMET_y>0) CorrectedMETPhi = TMath::Pi();
    else if(CorrectedMET_x==0 && CorrectedMET_y<0 )CorrectedMETPhi = -TMath::Pi();
    else if(CorrectedMET_x >0)                     CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x);
    else if(CorrectedMET_x <0 && CorrectedMET_y>0) CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x) + TMath::Pi();
    else if(CorrectedMET_x <0 && CorrectedMET_y<0) CorrectedMETPhi = TMath::ATan(CorrectedMET_y/CorrectedMET_x) - TMath::Pi();
    else CorrectedMETPhi = 0;

    return std::make_pair(CorrectedMET, CorrectedMETPhi);
}
