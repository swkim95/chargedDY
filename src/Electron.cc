#include "Electron.h"

//////////////////////////////////////////////////////////////
///////////////// ElectronHolder functions //////////////////
//////////////////////////////////////////////////////////////

Bool_t ElectronHolder::DoLooseObjSel() {
    // TODO: Need to update the cut values flexibly
    // Loose obj sel components : 
    // 1. electron pT
    // 2. electron eta
    // 3. electron ID
    // Ele : 0 ~ 1.444, 1.566 ~ 2.5
    // Loose pT : over 10 GeV
    // Loose ID : 2, 3, 4
    double pT = ElectronOrgVec.Pt();
    double absEta = std::abs(ElectronOrgVec.Eta() + fElectronDeltaEtaSC);
    if (    (pT > 25.0)
        &&  (absEta < 2.5)
        &&  (absEta < 1.444 || absEta > 1.566)
        &&  (bLooseId) ) {
        return true;
    }
    else {
        return false;
    }
}

//////////////////////////////////////////////////////////////
///////////////// Electrons functions ///////////////////////
//////////////////////////////////////////////////////////////

void Electrons::Init() {
    if (bIsInit) {
        std::cerr << "[Warning] Electrons::Init() - Electrons are already initialized" << std::endl;
        return;
    }

    vElectronVec.clear();
    for (Int_t idx = 0; idx < **(cData->nElectron); idx++) {
        // Define electron fourvector
        TLorentzVector vec;
        vec.SetPtEtaPhiM(cData->Electron_pt->At(idx), cData->Electron_eta->At(idx), cData->Electron_phi->At(idx), cData->Electron_mass->At(idx));
        // Define electron holder
        ElectronHolder electron(vec, idx, cData->Electron_charge->At(idx));
        // Set electron ID
        electron.SetCutBasedIds(cData->Electron_cutBased->At(idx));
        // Set electron deltaEtaSC
        electron.SetDeltaEtaSC(cData->Electron_deltaEtaSC->At(idx));
        // Add electron to vector
        vElectronVec.push_back(electron);
    }
    bIsInit = true;
}

void Electrons::DoObjSel() {
    // Check if electrons are initialized
    if (!bIsInit) {
        std::cerr << "[ERROR] Electrons::DoObjSel() - Electrons are not initialized" << std::endl;
        return;
    }

    // Check if object selection is already done
    if (bDidObjSel) {
        std::cerr << "[Warning] Electrons::DoObjSel() - Object selection is already done" << std::endl;
        return;
    }

    // Do object selection
    for (auto& electron : vElectronVec) {
        if (electron.GetCutBased() == -1) {
            std::cerr << "[ERROR] Electrons::DoObjSel() - Cut based ID is not set for electron " << electron.GetIndex() << std::endl;
            continue;
        }
        if (electron.DoLooseObjSel()) {
            electron.SetObjSel(true);
        }
        else {
            electron.SetObjSel(false);
        }
    }
    bDidObjSel = true;
}

std::vector<ElectronHolder>& Electrons::GetElectrons() {
    if (!bIsInit) {
        std::cerr << "[ERROR] Electrons::GetElectrons() - Electrons are not initialized" << std::endl;
        return vElectronVec;
    }
    return vElectronVec;
}

std::vector<ElectronHolder> Electrons::GetLooseElectrons() {
    if (!bDidObjSel) {
        std::cerr << "[ERROR] Electrons::GetLooseElectrons() - Object selection is not done" << std::endl;
        return vElectronVec;
    }

    std::vector<ElectronHolder> selectedElectrons;
    for (auto& electron : vElectronVec) {
        if (electron.PassLooseObjSel()) {
            selectedElectrons.push_back(electron);
        }
    }
    return selectedElectrons;
}