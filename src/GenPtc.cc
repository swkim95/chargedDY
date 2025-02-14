#include "GenPtc.h"

////////////////////////////////////////////////////////////
///////////////// GenPtcHolder functions ///////////////////
////////////////////////////////////////////////////////////

// This is for debugging purposes
void GenPtcHolder::PrintGenPtcInfo() {
    std::cout << "GenPtc_idx: " << iGenPtcIdx << std::endl;
    std::cout << "GenPtc_charge: " << iGenPtcCharge << std::endl;
    std::cout << "GenPtc_pdgId: " << iGenPtcPDGID << std::endl;
    std::cout << "GenPtc_status: " << iGenPtcStatus << std::endl;
    std::cout << "GenPtc_statusFlags: " << iGenPtcStatusFlags << std::endl;
    std::cout << "GenPtc_motherIdx: " << iGenPtcMotherIdx << std::endl;
    std::cout << "GenPtc_motherPDGID: " << iGenPtcMotherPDGID << std::endl;
    std::cout << "GenPtc_motherStatus: " << iGenPtcMotherStatus << std::endl;
}

// This is for debugging purposes
void GenPtcHolder::PrintGenPtcStatusFlags() {
    // Define an array of the corresponding flag names
    const char* flagNames[] = {
        "isPrompt",
        "isDecayedLeptonHadron",
        "isTauDecayProduct",
        "isPromptTauDecayProduct",
        "isDirectTauDecayProduct",
        "isDirectPromptTauDecayProduct",
        "isDirectHadronDecayProduct",
        "isHardProcess",
        "fromHardProcess",
        "isHardProcessTauDecayProduct",
        "isDirectHardProcessTauDecayProduct",
        "fromHardProcessBeforeFSR",
        "isFirstCopy",
        "isLastCopy",
        "isLastCopyBeforeFSR"
    };

    // Print the header
    std::cout << "GenPart_statusFlags: " << std::bitset<15>(iGenPtcStatusFlags) << std::endl;
    std::cout << "Flags:" << std::endl;

    // Iterate through each bit and print its value and name
    for (int i = 0; i < 15; ++i) {
        if (((iGenPtcStatusFlags >> i) & 1))
        std::cout << flagNames[i] << ": " << ((iGenPtcStatusFlags >> i) & 1) << std::endl;
    }
}

bool GenPtcHolder::IsPrompt() {
    // Check if the least significant bit (bit 0) is set
    return (iGenPtcStatusFlags & 0b0000000000000001) != 0;
}

bool GenPtcHolder::IsHardProcess() {
    return (iGenPtcStatusFlags & (1 << 7)) != 0;
}

bool GenPtcHolder::IsFromHardProcess() {
    // Check if bit 8 (fromHardProcess) is set
    return (iGenPtcStatusFlags & (1 << 8)) != 0;
}

bool GenPtcHolder::IsTauDecayProduct() {
    // Check if bit 3 (isTauDecayProduct) is set
    return (iGenPtcStatusFlags & (1 << 2)) != 0;
}

bool GenPtcHolder::IsPromptTauDecayProduct() {
    // Check if bit 4 (isPromptTauDecayProduct) is set
    return (iGenPtcStatusFlags & (1 << 3)) != 0;
}

bool GenPtcHolder::IsDirectTauDecayProduct() {
    // Check if bit 5 (isDirectTauDecayProduct) is set
    return (iGenPtcStatusFlags & (1 << 4)) != 0;
}  

bool GenPtcHolder::IsDirectPromptTauDecayProduct() {
    // Check if bit 6 (isDirectPromptTauDecayProduct) is set
    return (iGenPtcStatusFlags & (1 << 5)) != 0;
}

////////////////////////////////////////////////////////////
///////////////////// GenPtcs functions ////////////////////
////////////////////////////////////////////////////////////
void GenPtcs::Init() {
    if (bIsInit) {
        std::cerr << "[Warning] GenPtcs::Init() - GenPtcs is already initialized" << std::endl;
        return;
    }
    // Check whether to perform Gen-lv patching
    bDoPatching = (bIsInclusiveW || bIsBoostedW || bIsOffshellW || bIsOffshellWToTauNu);
    // Initialize the class
    vGenPtcVec.clear();
    vGenPtcVec.reserve(**(cData->nGenPart)); // Preallocate memory
    // Initialize the gen particles
    for (Int_t idx = 0; idx < **(cData->nGenPart); idx++) {
        // Define Gen particle fourvector
        TLorentzVector vec;
        vec.SetPtEtaPhiM(cData->GenPart_pt->At(idx), cData->GenPart_eta->At(idx), cData->GenPart_phi->At(idx), cData->GenPart_mass->At(idx));
        // Define the gen particle holder
        // FIXME: This is a hack to get the charge of the particle, only works for elec, muon and tau.
        GenPtcHolder genPtc(vec, idx, (int) -1 * (cData->GenPart_pdgId->At(idx) / std::abs(cData->GenPart_pdgId->At(idx))), cData->GenPart_pdgId->At(idx), cData->GenPart_status->At(idx), cData->GenPart_statusFlags->At(idx));
        // Set the mother index and PDGID and status
        genPtc.SetGenPtcMotherIdx(cData->GenPart_genPartIdxMother->At(idx));
        genPtc.SetGenPtcMotherPDGID(cData->GenPart_pdgId->At(cData->GenPart_genPartIdxMother->At(idx)));
        genPtc.SetGenPtcMotherStatus(cData->GenPart_status->At(cData->GenPart_genPartIdxMother->At(idx)));
        vGenPtcVec.push_back(genPtc);

        // Do Gen-lv patching for W samples (inclusive W, boosted W, offshell W->Mu+Nu, offshell W->Tau+Nu)
        if (bDoPatching) {

            // Check particle ID and status
            Bool_t isLepton = genPtc.IsLepton();
            Bool_t isNeutrino = genPtc.IsNeutrino();
            Bool_t isMuon = genPtc.IsMuon();
            Bool_t isTau = genPtc.IsTau();
            Bool_t isMuonNeutrino = genPtc.IsMuonNeutrino();
            Bool_t isTauNeutrino = genPtc.IsTauNeutrino();
            Bool_t isPrompt = genPtc.IsPrompt();
            Bool_t isFromHardProcess = genPtc.IsFromHardProcess();
            Bool_t isHardProcess = genPtc.IsHardProcess();

            // Select only one lepton using isHardProcess
            if (isLepton && isPrompt && isFromHardProcess && isHardProcess) {
                LeptonFromW = genPtc.GetGenPtcVec();
                iFoundLepton++;
                if (isMuon) iFoundMuon++; // Found muon with prompt and hard process
                if (isTau) iFoundTau++; // Found tau with prompt and hard process
            }
            // Select only one neutrino using isHardProcess
            else if (genPtc.IsNeutrino() && isPrompt && isFromHardProcess && isHardProcess) {
                NeutrinoFromW = genPtc.GetGenPtcVec();
                iFoundNeutrino++;
                if (isMuonNeutrino) iFoundMuonNeutrino++; // Found neutrino with prompt and hard process
                if (isTauNeutrino) iFoundTauNeutrino++; // Found tau neutrino with prompt and hard process
            }
            // Found W boson
            // if ( iFoundLepton && iFoundNeutrino && !(iFoundW) ) {
            if ( iFoundLepton && iFoundNeutrino ) {
                GenW = LeptonFromW + NeutrinoFromW;
                iFoundW++;
                if (iFoundMuon && iFoundMuonNeutrino) {
                    iIsWToMuNu++;
                }
            }
            // For inclusive W and boosted W, and if it satisfied foundTau or foundTauNeutrino
            // For offshell W->tau+nu, if it satisfied bIsOffshellWToTauNu
            // Then, find if there's a muon and muon neutrino from tau decay
            // If there's a muon and muon neutrino from tau decay, it means W->tau+nu->mu+nu+nu+nu
            if( iFoundTau || iFoundTauNeutrino || bIsOffshellWToTauNu) {
                // Tau from W decay is prompt tau
                // We need to find muon and neutrino from tau decay
                // Such muon satisfies isDirectPromptTauDecayProduct
                if ( genPtc.IsMuon() && genPtc.IsDirectPromptTauDecayProduct() ) {
                    iFoundMuonFromTauDecay++;
                }
                if ( genPtc.IsMuonNeutrino() && genPtc.IsDirectPromptTauDecayProduct() ) {
                    iFoundMuonNeutrinoFromTauDecay++;
                }
            }
            if ( iFoundMuonFromTauDecay && iFoundMuonNeutrinoFromTauDecay ) {
                iIsWToTauNuToMuNu++;
            }
        }
    }
    // After looping through all gen particles, and if Gen-lv patching is performed,
    // Check if the number of leptons and neutrinos from W boson is correct
    // There should be only one lepton and one neutrino from W boson
    // There should be only one muon and one muon neutrino from tau decay
    if (bDoPatching) {
        if ( (iFoundLepton != 1) || (iFoundNeutrino != 1) ) {
            std::cerr << "ERROR: Found more than one lepton or neutrino from W boson, or could not find W boson" << std::endl;
            std::cerr << "Evt num : " << cData->GetReader()->GetCurrentEntry() << std::endl;
            std::cerr << "Leptons : " << iFoundLepton << " Neutrinos : " << iFoundNeutrino << std::endl;
        }
        if ( (iFoundTau || iFoundTauNeutrino || bIsOffshellWToTauNu) && (iFoundMuonFromTauDecay > 1) || (iFoundMuonNeutrinoFromTauDecay > 1) ) {
            std::cerr << "ERROR: Found more than one muon or neutrino from tau decay" << std::endl;
            std::cerr << "Evt num : " << cData->GetReader()->GetCurrentEntry() << std::endl;
            std::cerr << "Muons : " << iFoundMuonFromTauDecay << " Neutrinos : " << iFoundMuonNeutrinoFromTauDecay << std::endl;
        }
    }

    // Set the initialization flag
    bIsInit = true;
}

Bool_t GenPtcs::PassGenPatching(Double_t HT_cut_high, Double_t W_mass_cut_high) {
    Bool_t passed = true;

    if (!bIsInit) {
        std::cerr << "[ERROR] GenPtcs::PassGenPatching() - GenPtcs are not initialized" << std::endl;
        return false;
    }

    if (!bDoPatching) {
        passed = true; // If not performing Gen-lv patching, always pass the patching
    }
    else {
        // TODO: What if the W boson is not found? consider this case also.
        Double_t wMass = GenW.M();
        if (bIsInclusiveW) {
            Double_t lheHT = **(cData->LHE_HT);
            if (wMass > W_mass_cut_high || lheHT > HT_cut_high) passed = false;
        } else if (bIsBoostedW) {
            if (wMass > W_mass_cut_high) passed = false;
        } else if (bIsOffshellW) {
            if (wMass >= W_mass_cut_high) passed = false;
        } else if (bIsOffshellWToTauNu) {
            if (wMass >= W_mass_cut_high) passed = false;
        }

        // Debug
        // std::cout << "GenW M(): " << GenW.M()  << "GeV, Mass cut high: " << W_mass_cut_high << "GeV, passed: " << passed << std::endl;
    }

    return passed;
}

Bool_t GenPtcs::PassMuonFiltering() {
    Bool_t passed = true;

    if (!bIsInit) {
        std::cerr << "[ERROR] GenPtcs::PassMuonFiltering() - GenPtcs are not initialized" << std::endl;
        return false;
    }

    if (!bDoPatching){
        passed = true; // If not performing Gen-lv patching, always pass the filtering
    }
    else {
        passed = (iIsWToMuNu || iIsWToTauNuToMuNu);
    }
    return passed;
}

std::vector<GenPtcHolder>& GenPtcs::GetGenPtcs() {
    if (!bIsInit) {
        std::cerr << "[ERROR] GenPtcs::GetGenPtcs() - GenPtcs are not initialized" << std::endl;
        return vGenPtcVec;
    }
    return vGenPtcVec;
}

std::vector<GenPtcHolder> GenPtcs::GetGenMuons() {
    std::vector<GenPtcHolder> vGenMuons;
    for (auto& genPtc : vGenPtcVec) {
        if (std::abs(genPtc.GetGenPtcPDGID()) == 13) {
            vGenMuons.push_back(genPtc);
        }
    }
    return vGenMuons;
}

std::vector<GenPtcHolder> GenPtcs::GetGenElectrons() {
    std::vector<GenPtcHolder> vGenElectrons;
    for (auto& genPtc : vGenPtcVec) {
        if (std::abs(genPtc.GetGenPtcPDGID()) == 11) {
            vGenElectrons.push_back(genPtc);
        }
    }
    return vGenElectrons;
}

std::vector<GenPtcHolder> GenPtcs::GetGenTaus() {
    std::vector<GenPtcHolder> vGenTaus;
    for (auto& genPtc : vGenPtcVec) {
        if (std::abs(genPtc.GetGenPtcPDGID()) == 15) {
            vGenTaus.push_back(genPtc);
        }
    }
    return vGenTaus;
}

std::vector<GenPtcHolder> GenPtcs::GetGenNeutrinos() {
    std::vector<GenPtcHolder> vGenNeutrinos;
    for (auto& genPtc : vGenPtcVec) {
        if (std::abs(genPtc.GetGenPtcPDGID()) == 12 || std::abs(genPtc.GetGenPtcPDGID()) == 14 || std::abs(genPtc.GetGenPtcPDGID()) == 16) {
            vGenNeutrinos.push_back(genPtc);
        }
    }
    return vGenNeutrinos;
}

void GenPtcs::PrintGenPtcChain() {
    for (auto& genPtc : vGenPtcVec) {
        Int_t pdgId = genPtc.GetGenPtcPDGID();
        Int_t idx   = genPtc.GetGenPtcIdx();
        Int_t status = genPtc.GetGenPtcStatus();
        Int_t statusFlags = genPtc.GetGenPtcStatusFlags();
        Int_t motherIdx = genPtc.GetGenPtcMotherIdx();
        Int_t motherPDGID = genPtc.GetGenPtcMotherPDGID();
        Int_t motherStatus = genPtc.GetGenPtcMotherStatus();

        if (std::abs(pdgId) == 11) {
            std::cout << "Idx: " << idx << " Elec status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 13) {
            std::cout << "Idx: " << idx << " Muon status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 15) {
            std::cout << "Idx: " << idx << " Tau status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 12) {
            std::cout << "Idx: " << idx << " Nu(e) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 14) {
            std::cout << "Idx: " << idx << " Nu(m) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 16) {
            std::cout << "Idx: " << idx << " Nu(t) status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 22) {
            std::cout << "Idx: " << idx << " Gamma status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 23) {
            std::cout << "Idx: " << idx << " Z boson status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else if (std::abs(pdgId) == 24) {
            std::cout << "Idx: " << idx << " W boson status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
        else {
            std::cout << "Idx: " << idx << " PDGID: " << pdgId << " status: " << status << " status flag: " << statusFlags << " mother idx: " << motherIdx << " mother pdgId: " << motherPDGID << " mother status: " << motherStatus << std::endl;
        }
    }
}