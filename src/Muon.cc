#include "Muon.h"

//////////////////////////////////////////////////////////////
///////////////// MuonHolder functions ///////////////////////
//////////////////////////////////////////////////////////////

// For highPt muons, MuonOrgVec is already set to have TuneP pT (Defined in Muon::Init())
// TODO: Need to update the cut values flexibly
Bool_t MuonHolder::DoTightObjSel() {
    // Tight obj sel components : 
    // 1. muon highPt ID (global)
    // 2. muon isolation (trk rel Iso)
    // 3. muon TuneP pT
    // 4. muon eta
    // If RoccoSF is not set, use the original muon fourvector
    TLorentzVector vec = (dMuonRoccoSF == -1) ? MuonOrgVec : MuonOrgVec * dMuonRoccoSF;
    if (   (vec.Pt() > 53.0)
        && (std::abs(vec.Eta()) < 2.4)
        && (bGlbHighPtId)
        && (fTkRelIso < 0.1) ) {
        return true;
    }
    else {
        return false;
    }
}

Bool_t MuonHolder::DoLooseObjSel() {
    // Loose obj sel components : 
    // 1. muon TuneP pT
    // 2. muon eta
    // 3. muon ID (loose)
    // If RoccoSF is not set, use the original muon fourvector
    TLorentzVector vec = (dMuonRoccoSF == -1) ? MuonOrgVec : MuonOrgVec * dMuonRoccoSF;
    if (   (vec.Pt() > 25.0)
        && (std::abs(vec.Eta()) < 2.4)
        && (bLooseId) ) {
        return true;
    }
    else {
        return false;
    }
}

//////////////////////////////////////////////////////////////
///////////////////// Muons functions ////////////////////////
//////////////////////////////////////////////////////////////

void Muons::Init() {
    // Check if muons are already initialized
    if (bIsInit) {
        std::cerr << "[Warning] Muons::Init() - Muons are already initialized" << std::endl;
        return;
    }

    // Loop over all muons, set their properties and collect them in a vector
    vMuonVec.clear();
    for (Int_t idx = 0; idx < **(cData->nMuon); idx++) {
        // Define muon fourvector
        // Use TuneP pT instead of pT for highPt muons
        TLorentzVector vec;
        vec.SetPtEtaPhiM(cData->Muon_pt->At(idx) * cData->Muon_tunepRelPt->At(idx), cData->Muon_eta->At(idx), cData->Muon_phi->At(idx), cData->Muon_mass->At(idx));
        // Define muon holder
        MuonHolder muon(vec, idx, cData->Muon_charge->At(idx));
        // Set muon TuneP pT
        muon.SetTunePRelPt(cData->Muon_tunepRelPt->At(idx));
        // Set muon type
        muon.SetMuonType(cData->Muon_isGlobal->At(idx), cData->Muon_isPFcand->At(idx), cData->Muon_isStandalone->At(idx), cData->Muon_isTracker->At(idx));
        // Set muon ID
        muon.SetMuonID(cData->Muon_looseId->At(idx), cData->Muon_mediumId->At(idx), cData->Muon_tightId->At(idx), cData->Muon_highPtId->At(idx));
        // Set muon isolation
        muon.SetTkRelIso(cData->Muon_tkRelIso->At(idx));
        muon.SetPfRelIso03_all(cData->Muon_pfRelIso03_all->At(idx));
        muon.SetPfRelIso03_chg(cData->Muon_pfRelIso03_chg->At(idx));
        muon.SetPfRelIso04_all(cData->Muon_pfRelIso04_all->At(idx));
        // Set muon nStations and tracker layers
        muon.SetNStations(cData->Muon_nStations->At(idx));
        muon.SetTrackerLayers(cData->Muon_nTrackerLayers->At(idx));
        // RoccoSF and EffSF should be calculated in the event loop
        vMuonVec.push_back(muon);
    }
    bIsInit = true;
}

void Muons::DoObjSel() {
    // Check if muons are initialized
    if (!bIsInit) {
        std::cerr << "[ERROR] Muons::DoObjSel() - Muons are not initialized" << std::endl;
        return;
    }

    // Check if object selection is already done
    if (bDidObjSel) {
        std::cerr << "[Warning] Muons::DoObjSel() - Object selection is already done" << std::endl;
        return;
    }

    // Do object selection
    for (auto& muon : vMuonVec) {
        // Set obj sel
        if (muon.DoTightObjSel()) { 
            muon.SetObjSel(true, false); // Is tight muon
        }
        else if (muon.DoLooseObjSel()) {
            muon.SetObjSel(false, true); // Is loose muon
        }
        else {
            muon.SetObjSel(false, false);
        }
    }   
    bDidObjSel = true;
}

// Get all muons
std::vector<MuonHolder>& Muons::GetMuons() {
    // Check if muons are initialized
    if (!bIsInit) {
        std::cerr << "[ERROR] Muons::GetMuons() - Muons are not initialized" << std::endl;
        return vMuonVec;
    }
    return vMuonVec;
}

std::vector<MuonHolder> Muons::GetTightMuons() {
    // Check if object selection is done
    if (!bDidObjSel) {
        std::cerr << "[ERROR] Muons::GetTightMuons() - Object selection is not done" << std::endl;
        return std::vector<MuonHolder> {};
    }

    std::vector<MuonHolder> selectedMuons;
    for (auto& muon : vMuonVec) {
        if (muon.PassTightObjSel()) {
            selectedMuons.push_back(muon);
        }
    }
    return selectedMuons;
}

std::vector<MuonHolder> Muons::GetLooseMuons() {
    // Check if object selection is done
    if (!bDidObjSel) {
        std::cerr << "[ERROR] Muons::GetLooseMuons() - Object selection is not done" << std::endl;
        return std::vector<MuonHolder> {};
    }

    std::vector<MuonHolder> looseMuons;
    for (auto& muon : vMuonVec) {
        if (muon.PassLooseObjSel()) {
            looseMuons.push_back(muon);
        }
    }
    return looseMuons;
}