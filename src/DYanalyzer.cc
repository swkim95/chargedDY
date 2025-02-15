#include "DYanalyzer.h"

////////////////////////////////////////////////////////////
//////////////// Actual event loop is here! ////////////////
////////////////////////////////////////////////////////////
void DYanalyzer::Analyze() {
    // Check if DYanalyzer is initialized
    if (!bIsInit) {
        throw std::runtime_error("[Runtime Error] DYanalyzer::Analyze() - DYanalyzer is not initialized");
    }

    // Declare object classes
    Muons* muons = new Muons(cData);
    Electrons* electrons = new Electrons(cData);
    MET* met = new MET(cData);
    GenPtcs* genPtcs = nullptr;
    if (bIsMC) {
        genPtcs = new GenPtcs(cData, bIsInclusiveW, bIsBoostedW, bIsOffshellW, bIsOffshellWToTauNu);
    }

    ////////////////////////////////////////////////////////////
    ////////////////////// Event loop //////////////////////////
    ////////////////////////////////////////////////////////////
    std::cout << "[Info] DYanalyzer::Analyze() - Start event loop" << std::endl;
    int iEvt = 0;
    while (cData->ReadNextEntry()) {
        // Print progress and set current event number
        if (iEvt % 10000 == 0) PrintProgress(iEvt);
        iEvt++;

        // Reset object classes
        muons->Reset();
        electrons->Reset();
        met->Reset();
        // Initialize object classes
        muons->Init();
        electrons->Init();
        met->Init();
        // Initialize genPtcs if MC
        if (bIsMC) {
            genPtcs->Reset();
            genPtcs->Init();
        }

        // Get corrected PFMET
        std::pair<Double_t, Double_t> correctedPFMET = met->GetPFMETXYCorr(sProcessName, sEra, bIsMC, **(cData->NPV));
        Double_t dPFMET_corr = correctedPFMET.first;
        Double_t dPFMET_corr_phi = correctedPFMET.second;

        // Set event weight
        // For data, event weight is 1.0
        Double_t eventWeight = 1.0;
        if (bIsMC) {
            eventWeight = **(cData->GenWeight) < 0 ? -1.0 : 1.0;
        }

        ////////////////////////////////////////////////////////////
        ////////////////////// Corrections /////////////////////////
        ////////////////////////////////////////////////////////////
        // Do PU correction
        if (bIsMC && bDoPUCorrection) {
            // Get PU weight
            Float_t nTrueInt = **(cData->Pileup_nTrueInt);
            eventWeight *= cPU->GetPUWeight(nTrueInt);
        }
        // Do L1 pre-firing correction
        if (bIsMC && bDoL1PreFiringCorrection) {
            eventWeight *= **(cData->L1PreFiringWeight_Nom);
        }
        // Do Rocco before EffSF calculation
        // If DoRocco, then Obj selection should be done after Rocco
        if (bDoRocco) {
            // Loop over muons
            std::vector<MuonHolder>& muonCollection = muons->GetMuons();
            for (MuonHolder& singleMuon : muonCollection) {
                // Rocco for MC
                if (bIsMC) {
                    // Gen - Reco muon matching using dR
                    Double_t min_dR = 999.;
                    Bool_t matchedToGenMuon = false;
                    Int_t matchedGenMuonIdx = -1;
                    // Get gen muons
                    std::vector<GenPtcHolder> genMuonCollection = genPtcs->GetGenMuons();
                    for (Int_t idx = 0; idx < genMuonCollection.size(); idx++) {
                        GenPtcHolder& singleGenMuon = genMuonCollection[idx];
                        // Get dR between gen muon and reco muon
                        Double_t dR = (singleGenMuon.GetGenPtcVec()).DeltaR(singleMuon.GetMuonOrgVec());
                        if (dR < min_dR && dR < 0.1) {
                            min_dR = dR;
                            matchedToGenMuon = true;
                            matchedGenMuonIdx = idx;
                        }
                    }
                    // If well matched
                    if (matchedToGenMuon) {
                        Double_t roccoSF = cRochesterCorrection->kSpreadMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), genMuonCollection[matchedGenMuonIdx].GetGenPtcVec().Pt(), 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                    }
                    // If not matched
                    else {
                        Double_t randomSeed = gRandom->Rndm(); // Random seed btw 0 ~ 1
                        Double_t roccoSF = cRochesterCorrection->kSmearMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), singleMuon.GetTrackerLayers(), randomSeed, 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                    }
                }
                // Rocco for data
                else {
                    Double_t roccoSF = cRochesterCorrection->kScaleDT(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), 5, 0);
                    singleMuon.SetRoccoSF(roccoSF);
                }
            }
        }

        // Do object selection here
        muons->DoObjSel();
        electrons->DoObjSel();

        // Only calculate eff SF for tight muons
        // Do efficiency SF correction
        if (bIsMC && (bDoIDSF || bDoIsoSF || bDoTrigSF)) {
            std::vector<double> efficiencySF = {1.0, 1.0, 1.0};

            // Set efficiency SF over all muons
            std::vector<MuonHolder> tightMuonCollection = muons->GetTightMuons();
            if (tightMuonCollection.size() > 0) {
                MuonHolder& leadingMuon = tightMuonCollection[0];
                // Get efficiency SF
                efficiencySF = cEfficiencySF->GetEfficiency(leadingMuon.Pt(), leadingMuon.Eta());
                // Apply efficiency SF
                leadingMuon.SetEfficiencySF(efficiencySF);
            }

            if (bDoIDSF)   eventWeight *= efficiencySF[0]; // ID SF
            if (bDoIsoSF)  eventWeight *= efficiencySF[1]; // Iso SF
            if (bDoTrigSF) eventWeight *= efficiencySF[2]; // Trig SF
        }

        ////////////////////////////////////////////////////////////
        ////// Sum up event weight here (after all corrections) ////
        ////////////////////////////////////////////////////////////
        dSumOfGenEvtWeight += eventWeight;
        // Fill PU related histograms (NPU, NTrueInt only available for MC)
        // This should be done before gen-lv patching and muon filtering
        // (since PU has nothing to do with gen-lv patching and muon filtering)
        hNPV->Fill(**(cData->NPV), eventWeight);
        if (bIsMC) {
            hNPU->Fill(**(cData->Pileup_nPU), eventWeight);
            hNTrueInt->Fill(**(cData->Pileup_nTrueInt), eventWeight);
        }

        ////////////////////////////////////////////////////////////
        ////// Apply Gen-lv patching and Gen-lv muon filtering /////
        ////////////////////////////////////////////////////////////
        if (bIsMC && bDoGenPatching) {
            Bool_t passedGenPatching = genPtcs->PassGenPatching(dHT_cut_high, dW_mass_cut_high);
            Bool_t passedMuonFiltering = genPtcs->PassMuonFiltering();

            // Skip event if Gen-lv patching failed
            if (!passedGenPatching) continue;

            // Fill Gen-lv inclusive W histograms before muon filtering
            if (genPtcs->IsInclusiveW()) {
                hGen_W_pT->Fill(genPtcs->GetGenW().Pt(), eventWeight);
                hGen_W_eta->Fill(genPtcs->GetGenW().Eta(), eventWeight);
                hGen_W_phi->Fill(genPtcs->GetGenW().Phi(), eventWeight);
                hGen_W_mass->Fill(genPtcs->GetGenW().M(), eventWeight);
            }

            // Skip event if muon filtering failed
            if (!passedMuonFiltering) continue;
        }

        ////////////////////////////////////////////////////////////
        ///////// Fill histograms before event selection ///////////
        ////////////////////////////////////////////////////////////
        // Fill Gen-lv histograms for MC
        if (bIsMC) {
            // Fill Gen W for W->mu+nu channel
            // or fill Gen W for W->tau+nu->mu+nu channel
            if ( genPtcs->IsWToMuNu() || genPtcs->IsWToTauNuToMuNu() ) {
                hGen_WToMuNu_pT->Fill(genPtcs->GetGenW().Pt(), eventWeight);
                hGen_WToMuNu_eta->Fill(genPtcs->GetGenW().Eta(), eventWeight);
                hGen_WToMuNu_phi->Fill(genPtcs->GetGenW().Phi(), eventWeight);
                hGen_WToMuNu_mass->Fill(genPtcs->GetGenW().M(), eventWeight);
            }

            // Fill Gen Muon
            std::vector<GenPtcHolder> genMuonCollection = genPtcs->GetGenMuons();
            // Sort genMuonCollection in descending order by Pt
            if (genMuonCollection.size() > 0) {
                // std::vector<GenPtcHolder>::iterator type
                auto maxElemIt = std::max_element(genMuonCollection.begin(), genMuonCollection.end(),
                    [](const GenPtcHolder &a, const GenPtcHolder &b) {
                        return a.GetGenPtcVec().Pt() < b.GetGenPtcVec().Pt();
                    }
                );
                GenPtcHolder &leadingGenMuon = *maxElemIt;

                hGen_Muon_pT->Fill(leadingGenMuon.GetGenPtcVec().Pt(), eventWeight);
                hGen_Muon_phi->Fill(leadingGenMuon.GetGenPtcVec().Phi(), eventWeight);
                hGen_Muon_eta->Fill(leadingGenMuon.GetGenPtcVec().Eta(), eventWeight);
            }

            // Fill Gen Neutrino
            std::vector<GenPtcHolder> genNeutrinoCollection = genPtcs->GetGenNeutrinos();
            // Sort genNeutrinoCollection in descending order by Pt
            if (genNeutrinoCollection.size() > 0) {
                auto maxElemIt = std::max_element(genNeutrinoCollection.begin(), genNeutrinoCollection.end(),
                    [](const GenPtcHolder &a, const GenPtcHolder &b) {
                        return a.GetGenPtcVec().Pt() < b.GetGenPtcVec().Pt();
                    }
                );
                GenPtcHolder &leadingGenNeutrino = *maxElemIt;
                hGen_Nu_pT->Fill(leadingGenNeutrino.GetGenPtcVec().Pt(), eventWeight);
                hGen_Nu_phi->Fill(leadingGenNeutrino.GetGenPtcVec().Phi(), eventWeight);
                hGen_Nu_eta->Fill(leadingGenNeutrino.GetGenPtcVec().Eta(), eventWeight);
            }

            // Fill Gen MET
            hGen_MET_phi->Fill(cData->GenMET_phi->At(0), eventWeight);
            hGen_MET_pT->Fill(cData->GenMET_pt->At(0), eventWeight);

            // Fill LHE HT
            if (genPtcs->IsInclusiveW() || genPtcs->IsBoostedW()) {
                hLHE_HT->Fill(**(cData->LHE_HT), eventWeight);
            }
        }

        // Fill Object level histograms
        // Fill muon (tight muon only)
        if (muons->GetTightMuons().size() > 0) {
            MuonHolder& leadingMuon = muons->GetTightMuons()[0];
            TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();
            
            hMuon_pT->Fill(leadingMuonVec.Pt(), eventWeight);
            hMuon_phi->Fill(leadingMuonVec.Phi(), eventWeight);
            hMuon_eta->Fill(leadingMuonVec.Eta(), eventWeight);
            hMuon_mass->Fill(leadingMuonVec.M(), eventWeight);
        }

        // Fill MET
        hMET_phi->Fill(met->GetPuppiMET_phi(), eventWeight);
        hMET_pT->Fill(met->GetPuppiMET_pt(), eventWeight);
        hMET_sumET->Fill(met->GetPuppiMET_sumEt(), eventWeight);

        hPFMET_phi->Fill(met->GetMET_phi(), eventWeight);
        hPFMET_pT->Fill(met->GetMET_pt(), eventWeight);
        hPFMET_sumET->Fill(met->GetMET_sumEt(), eventWeight);

        hPFMET_corr_phi->Fill(dPFMET_corr_phi, eventWeight);
        hPFMET_corr_pT->Fill(dPFMET_corr, eventWeight);

        // Reco and fill W MT (tight muon only)
        if (muons->GetTightMuons().size() > 0) {
            MuonHolder& leadingMuon = muons->GetTightMuons()[0];
            TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();

            Double_t deltaPhi = met->GetPuppiMET_phi() - leadingMuonVec.Phi();
            if (deltaPhi > M_PI) deltaPhi -= 2 * M_PI;
            if (deltaPhi < -M_PI) deltaPhi += 2 * M_PI;
            Double_t W_MT = std::sqrt( 2 * leadingMuonVec.Pt() * met->GetPuppiMET_pt() * (1 - std::cos(deltaPhi)) );
            
            hDeltaPhi_Mu_MET->Fill(deltaPhi, eventWeight);
            hW_MT->Fill(W_MT, eventWeight);

            // Using PFMET
            Double_t deltaPhi_PFMET = met->GetMET_phi() - leadingMuonVec.Phi();
            if (deltaPhi_PFMET > M_PI) deltaPhi_PFMET -= 2 * M_PI;
            if (deltaPhi_PFMET < -M_PI) deltaPhi_PFMET += 2 * M_PI;
            Double_t W_MT_PFMET = std::sqrt( 2 * leadingMuonVec.Pt() * met->GetMET_pt() * (1 - std::cos(deltaPhi_PFMET)) );
            
            hDeltaPhi_Mu_PFMET->Fill(deltaPhi_PFMET, eventWeight);
            hW_MT_PFMET->Fill(W_MT_PFMET, eventWeight);

            // Using corrected PFMET
            Double_t deltaPhi_PFMET_corr = dPFMET_corr_phi - leadingMuonVec.Phi();
            if (deltaPhi_PFMET_corr > M_PI) deltaPhi_PFMET_corr -= 2 * M_PI;
            if (deltaPhi_PFMET_corr < -M_PI) deltaPhi_PFMET_corr += 2 * M_PI;
            Double_t W_MT_PFMET_corr = std::sqrt( 2 * leadingMuonVec.Pt() * dPFMET_corr * (1 - std::cos(deltaPhi_PFMET_corr)) );
            
            hDeltaPhi_Mu_PFMET_corr->Fill(deltaPhi_PFMET_corr, eventWeight);
            hW_MT_PFMET_corr->Fill(W_MT_PFMET_corr, eventWeight);
        }

        ////////////////////////////////////////////////////////////
        //////////////// Event selection here //////////////////////
        ////////////////////////////////////////////////////////////
        // 1. Trigger
        // 2016APV : IsoMu24 || IsoTkMu24
        // 2016 : IsoMu24 || IsoTkMu24
        // 2017 : IsoMu27
        // 2018 : IsoMu24
        Bool_t passedTrigger = false;
        if (sEra == "2016APV") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));
        } else if (sEra == "2016") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));
        } else if (sEra == "2017") {
            passedTrigger = **(cData->HLT_IsoMu27);
        } else if (sEra == "2018") {
            passedTrigger = **(cData->HLT_IsoMu24);
        }
        if (!passedTrigger) continue;

        // 2. Noise filter
        Bool_t flag_goodVertices                       =  **(cData->Flag_goodVertices);
        Bool_t flag_globalSuperTightHalo2016Filter     =  **(cData->Flag_globalSuperTightHalo2016Filter);
        Bool_t flag_HBHENoiseFilter                    =  **(cData->Flag_HBHENoiseFilter);
        Bool_t flag_HBHENoiseIsoFilter                 =  **(cData->Flag_HBHENoiseIsoFilter);
        Bool_t flag_EcalDeadCellTriggerPrimitiveFilter =  **(cData->Flag_EcalDeadCellTriggerPrimitiveFilter);
        Bool_t flag_BadPFMuonFilter                    =  **(cData->Flag_BadPFMuonFilter);
        Bool_t flag_BadPFMuonDzFilter                  =  **(cData->Flag_BadPFMuonDzFilter);
        Bool_t flag_hfNoisyHitsFilter                  =  **(cData->Flag_hfNoisyHitsFilter);
        Bool_t flag_BadChargedCandidateFilter          =  **(cData->Flag_BadChargedCandidateFilter);
        Bool_t flag_eeBadScFilter                      =  **(cData->Flag_eeBadScFilter);
        Bool_t flag_ecalBadCalibFilter                 =  **(cData->Flag_ecalBadCalibFilter);
        bool passed_filter = (  flag_goodVertices                      &&
                                flag_globalSuperTightHalo2016Filter    &&
                                flag_HBHENoiseFilter                   &&
                                flag_HBHENoiseIsoFilter                &&
                                flag_EcalDeadCellTriggerPrimitiveFilter&&
                                flag_BadPFMuonFilter                   &&
                                flag_BadPFMuonDzFilter                 &&
                                flag_hfNoisyHitsFilter                 &&
                                flag_BadChargedCandidateFilter         &&
                                flag_eeBadScFilter                     &&
                                flag_ecalBadCalibFilter);
        if (!passed_filter) continue; // Skip event if noise filter failed

        // 3. Require only single tight muon
        if( muons->GetTightMuons().size() != 1 ) continue;

        // 4. Additional loose lepton veto
        // 4-1. Additional loose muon veto
        if( muons->GetLooseMuons().size() > 0 ) continue;
        // 4-2. Additional electron veto -> TODO: Implement this and see the effect
        if (electrons->GetLooseElectrons().size() > 0) continue;

        // 5. Require reconstructed W mass > 40 GeV (for high pT region)
        // Using PUPPI MET for this cut
        MuonHolder& leadingMuon = muons->GetTightMuons()[0];
        TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();
        Double_t deltaPhi = met->GetPuppiMET_phi() - leadingMuonVec.Phi();
        if (deltaPhi > M_PI) deltaPhi -= 2 * M_PI;
        if (deltaPhi < -M_PI) deltaPhi += 2 * M_PI;
        Double_t W_MT = std::sqrt( 2 * leadingMuonVec.Pt() * met->GetPuppiMET_pt() * (1 - std::cos(deltaPhi)) );
        if (W_MT < 40.) continue;

        ////////////////////////////////////////////////////////////
        //////// Fill histograms after event selection /////////////
        ////////////////////////////////////////////////////////////
        // Fill PU related histograms (NPU, NTrueInt only available for MC)
        hNPV_after->Fill(**(cData->NPV), eventWeight);
        
        // Fill Gen-lv histograms after event selection
        if (bIsMC) {
            // Fill PU related histograms (NPU, NTrueInt only available for MC)
            hNPU_after->Fill(**(cData->Pileup_nPU), eventWeight);
            hNTrueInt_after->Fill(**(cData->Pileup_nTrueInt), eventWeight);

            // Fill Gen Muon
            std::vector<GenPtcHolder> genMuonCollection = genPtcs->GetGenMuons();
            // Sort genMuonCollection in descending order by Pt
            if (genMuonCollection.size() > 0) {
                auto maxElemIt = std::max_element(genMuonCollection.begin(), genMuonCollection.end(),
                    [](const GenPtcHolder &a, const GenPtcHolder &b) {
                        return a.GetGenPtcVec().Pt() < b.GetGenPtcVec().Pt();
                    }
                );
                GenPtcHolder &leadingGenMuon = *maxElemIt;
                hGen_Muon_pT_after->Fill(leadingGenMuon.GetGenPtcVec().Pt(), eventWeight);
                hGen_Muon_phi_after->Fill(leadingGenMuon.GetGenPtcVec().Phi(), eventWeight);
                hGen_Muon_eta_after->Fill(leadingGenMuon.GetGenPtcVec().Eta(), eventWeight);
            }

            // Fill Gen Neutrino
            std::vector<GenPtcHolder> genNeutrinoCollection = genPtcs->GetGenNeutrinos();
            // Sort genNeutrinoCollection in descending order by Pt
            if (genNeutrinoCollection.size() > 0) {
                auto maxElemIt = std::max_element(genNeutrinoCollection.begin(), genNeutrinoCollection.end(),
                    [](const GenPtcHolder &a, const GenPtcHolder &b) {
                        return a.GetGenPtcVec().Pt() < b.GetGenPtcVec().Pt();
                    }
                );
                GenPtcHolder &leadingGenNeutrino = *maxElemIt;
                hGen_Nu_pT_after->Fill(leadingGenNeutrino.GetGenPtcVec().Pt(), eventWeight);
                hGen_Nu_phi_after->Fill(leadingGenNeutrino.GetGenPtcVec().Phi(), eventWeight);
                hGen_Nu_eta_after->Fill(leadingGenNeutrino.GetGenPtcVec().Eta(), eventWeight);
            }

            // Fill Gen MET
            hGen_MET_phi_after->Fill(cData->GenMET_phi->At(0), eventWeight);
            hGen_MET_pT_after->Fill(cData->GenMET_pt->At(0), eventWeight);

            // Fill LHE HT
            if (genPtcs->IsInclusiveW() || genPtcs->IsBoostedW()) {
                hLHE_HT_after->Fill(**(cData->LHE_HT), eventWeight);
            }
            // Fill inclusive Gen W
            // But since this is after muon filtering, it should be same with WToMuNu histograms
            if (genPtcs->FoundW()){
                hGen_W_pT_after->Fill(genPtcs->GetGenW().Pt(), eventWeight);
                hGen_W_eta_after->Fill(genPtcs->GetGenW().Eta(), eventWeight);
                hGen_W_phi_after->Fill(genPtcs->GetGenW().Phi(), eventWeight);
                hGen_W_mass_after->Fill(genPtcs->GetGenW().M(), eventWeight);
            }
            // Fill Gen W for W->mu+nu channel
            // or fill Gen W for W->tau+nu->mu+nu channel
            if ( genPtcs->IsWToMuNu() || genPtcs->IsWToTauNuToMuNu() ) {
                hGen_WToMuNu_pT_after->Fill(genPtcs->GetGenW().Pt(), eventWeight);
                hGen_WToMuNu_eta_after->Fill(genPtcs->GetGenW().Eta(), eventWeight);
                hGen_WToMuNu_phi_after->Fill(genPtcs->GetGenW().Phi(), eventWeight);
                hGen_WToMuNu_mass_after->Fill(genPtcs->GetGenW().M(), eventWeight);
            }
        }

        // Fill Object level histograms
        // Since this is after event selection, there's only one tight muon
        // MuonHolder& leadingMuon = muons->GetTightMuons()[0];
        // TLorentzVector leadingMuonVec = leadingMuon.GetRoccoSF() == -1. ? leadingMuon.GetMuonOrgVec() : leadingMuon.GetMuonRoccoVec();

        // Fill muon (tight muon only)
        hMuon_pT_after->Fill(leadingMuonVec.Pt(), eventWeight);
        hMuon_phi_after->Fill(leadingMuonVec.Phi(), eventWeight);
        hMuon_eta_after->Fill(leadingMuonVec.Eta(), eventWeight);
        hMuon_mass_after->Fill(leadingMuonVec.M(), eventWeight);

        // Fill MET
        hMET_phi_after->Fill(met->GetPuppiMET_phi(), eventWeight);
        hMET_pT_after->Fill(met->GetPuppiMET_pt(), eventWeight);
        hMET_sumET_after->Fill(met->GetPuppiMET_sumEt(), eventWeight);

        hPFMET_phi_after->Fill(met->GetMET_phi(), eventWeight);
        hPFMET_pT_after->Fill(met->GetMET_pt(), eventWeight);
        hPFMET_sumET_after->Fill(met->GetMET_sumEt(), eventWeight);

        hPFMET_corr_phi_after->Fill(dPFMET_corr_phi, eventWeight);
        hPFMET_corr_pT_after->Fill(dPFMET_corr, eventWeight);

        // Reco and fill W MT (tight muon only)
        // Double_t deltaPhi = met->GetPuppiMET_phi() - leadingMuonVec.Phi();
        // if (deltaPhi > M_PI) deltaPhi -= 2 * M_PI;
        // if (deltaPhi < -M_PI) deltaPhi += 2 * M_PI;

        // Double_t W_MT = std::sqrt( 2 * leadingMuonVec.Pt() * met->GetPuppiMET_pt() * (1 - std::cos(deltaPhi)) );
        hDeltaPhi_Mu_MET_after->Fill(deltaPhi, eventWeight);
        hW_MT_after->Fill(W_MT, eventWeight);

        // Using PFMET
        Double_t deltaPhi_PFMET = met->GetMET_phi() - leadingMuonVec.Phi();
        if (deltaPhi_PFMET > M_PI) deltaPhi_PFMET -= 2 * M_PI;
        if (deltaPhi_PFMET < -M_PI) deltaPhi_PFMET += 2 * M_PI;
        Double_t W_MT_PFMET = std::sqrt( 2 * leadingMuonVec.Pt() * met->GetMET_pt() * (1 - std::cos(deltaPhi_PFMET)) );
        
        hDeltaPhi_Mu_PFMET_after->Fill(deltaPhi_PFMET, eventWeight);
        hW_MT_PFMET_after->Fill(W_MT_PFMET, eventWeight);

        // Using corrected PFMET
        Double_t deltaPhi_PFMET_corr = dPFMET_corr_phi - leadingMuonVec.Phi();
        if (deltaPhi_PFMET_corr > M_PI) deltaPhi_PFMET_corr -= 2 * M_PI;
        if (deltaPhi_PFMET_corr < -M_PI) deltaPhi_PFMET_corr += 2 * M_PI;
        Double_t W_MT_PFMET_corr = std::sqrt( 2 * leadingMuonVec.Pt() * dPFMET_corr * (1 - std::cos(deltaPhi_PFMET_corr)) );
        
        hDeltaPhi_Mu_PFMET_corr_after->Fill(deltaPhi_PFMET_corr, eventWeight);
        hW_MT_PFMET_corr_after->Fill(W_MT_PFMET_corr, eventWeight);
    } // End of event loop

    //////////////////////////////////////////////////////////
    ///////// Fill histograms after event loop ///////////////
    //////////////////////////////////////////////////////////
    hGenEvtWeight->SetBinContent(1, dSumOfGenEvtWeight);

    std::cout << "[Info] DYanalyzer::Analyze() - End of event loop" << std::endl;
    std::cout << "[Info] DYanalyzer::Analyze() - Total sum of weight: " << std::fixed << std::setprecision(2) << dSumOfGenEvtWeight << std::endl;
}

////////////////////////////////////////////////////////////
//////////////// Z peak study event loop ///////////////////
////////////////////////////////////////////////////////////
void DYanalyzer::Analyze_Z() {
    // Check if DYanalyzer is initialized
    if (!bIsInit) {
        throw std::runtime_error("[Runtime Error] DYanalyzer::Analyze() - DYanalyzer is not initialized");
    }

    // Declare object classes
    Muons* muons = new Muons(cData);
    Electrons* electrons = new Electrons(cData);
    MET* met = new MET(cData);
    GenPtcs* genPtcs = nullptr;
    if (bIsMC) {
        genPtcs = new GenPtcs(cData, bIsInclusiveW, bIsBoostedW, bIsOffshellW, bIsOffshellWToTauNu);
    }

    ////////////////////////////////////////////////////////////
    ////////////////////// Event loop //////////////////////////
    ////////////////////////////////////////////////////////////
    std::cout << "[Info] DYanalyzer::Analyze_Z() - Start event loop" << std::endl;
    int iEvt = 0;
    while (cData->ReadNextEntry()) {
        // Print progress and set current event number
        if (iEvt % 10000 == 0) PrintProgress(iEvt);
        iEvt++;

        // Reset object classes
        muons->Reset();
        electrons->Reset();
        met->Reset();
        // Initialize object classes
        muons->Init();
        electrons->Init();
        met->Init();
        // Initialize genPtcs if MC
        if (bIsMC) {
            genPtcs->Reset();
            genPtcs->Init();
        }

        // Get corrected PFMET
        std::pair<Double_t, Double_t> correctedPFMET = met->GetPFMETXYCorr(sProcessName, sEra, bIsMC, **(cData->NPV));
        Double_t dPFMET_corr = correctedPFMET.first;
        Double_t dPFMET_corr_phi = correctedPFMET.second;

        // Set event weight
        // For data, event weight is 1.0
        Double_t eventWeight = 1.0;
        if (bIsMC) {
            eventWeight = **(cData->GenWeight) < 0 ? -1.0 : 1.0;
        }

        ////////////////////////////////////////////////////////////
        ////////////////////// Corrections /////////////////////////
        ////////////////////////////////////////////////////////////
        // Do PU correction
        if (bIsMC && bDoPUCorrection) {
            // Get PU weight
            Float_t nTrueInt = **(cData->Pileup_nTrueInt);
            eventWeight *= cPU->GetPUWeight(nTrueInt);
        }
        // Do L1 pre-firing correction
        if (bIsMC && bDoL1PreFiringCorrection) {
            eventWeight *= **(cData->L1PreFiringWeight_Nom);
        }
        // Do Rocco before EffSF calculation
        // If DoRocco, then Obj selection should be done after Rocco
        if (bDoRocco) {
            // Loop over muons
            std::vector<MuonHolder>& muonCollection = muons->GetMuons();
            for (MuonHolder& singleMuon : muonCollection) {
                // Rocco for MC
                if (bIsMC) {
                    // Gen - Reco muon matching using dR
                    Double_t min_dR = 999.;
                    Bool_t matchedToGenMuon = false;
                    Int_t matchedGenMuonIdx = -1;
                    // Get gen muons
                    std::vector<GenPtcHolder> genMuonCollection = genPtcs->GetGenMuons();
                    for (Int_t idx = 0; idx < genMuonCollection.size(); idx++) {
                        GenPtcHolder& singleGenMuon = genMuonCollection[idx];
                        // Get dR between gen muon and reco muon
                        Double_t dR = (singleGenMuon.GetGenPtcVec()).DeltaR(singleMuon.GetMuonOrgVec());
                        if (dR < min_dR && dR < 0.1) {
                            min_dR = dR;
                            matchedToGenMuon = true;
                            matchedGenMuonIdx = idx;
                        }
                    }
                    // If well matched
                    if (matchedToGenMuon) {
                        Double_t roccoSF = cRochesterCorrection->kSpreadMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), genMuonCollection[matchedGenMuonIdx].GetGenPtcVec().Pt(), 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                    }
                    // If not matched
                    else {
                        Double_t randomSeed = gRandom->Rndm(); // Random seed btw 0 ~ 1
                        Double_t roccoSF = cRochesterCorrection->kSmearMC(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), singleMuon.GetTrackerLayers(), randomSeed, 5, 0);
                        singleMuon.SetRoccoSF(roccoSF);
                    }
                }
                // Rocco for data
                else {
                    Double_t roccoSF = cRochesterCorrection->kScaleDT(singleMuon.Charge(), singleMuon.Pt(), singleMuon.Eta(), singleMuon.Phi(), 5, 0);
                    singleMuon.SetRoccoSF(roccoSF);
                }
            }
        }
        // Do object selection here
        muons->DoZObjSel();

        // Only calculate eff SF for tight muons
        // Do efficiency SF correction
        if (bIsMC && (bDoIDSF || bDoIsoSF || bDoTrigSF)) {
            std::vector<double> leading_efficiencySF = {1.0, 1.0, 1.0, 1.0};
            std::vector<double> subLeading_efficiencySF = {1.0, 1.0, 1.0, 1.0};

            // Set efficiency SF over all muons
            std::vector<MuonHolder> ZCandidateCollection = muons->GetMuonsFromZ();
            if (ZCandidateCollection.size() > 1) {
                MuonHolder& leadingMuon = ZCandidateCollection[0];
                MuonHolder& subLeadingMuon = ZCandidateCollection[1];
                // Get efficiency SF
                leading_efficiencySF = cEfficiencySF->GetZEfficiency(leadingMuon.Pt(), leadingMuon.Eta());
                subLeading_efficiencySF = cEfficiencySF->GetZEfficiency(subLeadingMuon.Pt(), subLeadingMuon.Eta());
                // Apply efficiency SF
                leadingMuon.SetZEfficiencySF(leading_efficiencySF);
                subLeadingMuon.SetZEfficiencySF(subLeading_efficiencySF);
            }
            // Consider both leading and sub-leading muons
            if(bDoIDSF) {
                eventWeight *= leading_efficiencySF[0]; // ID SF
                eventWeight *= subLeading_efficiencySF[0]; // ID SF
            }
            if (bDoIsoSF) {
                eventWeight *= leading_efficiencySF[1]; // Iso SF
                eventWeight *= subLeading_efficiencySF[1]; // Iso SF
            }
            if (bDoTrigSF) {
                Double_t data_SF = 1. - (1. - leading_efficiencySF[2]) * (1. - subLeading_efficiencySF[2]);
                Double_t mc_SF = 1. - (1. - leading_efficiencySF[3]) * (1. - subLeading_efficiencySF[3]);
                Double_t TrigSF = 0.;
                if (mc_SF != 0.)
                    TrigSF = data_SF / mc_SF;
                eventWeight *= TrigSF; // Trig SF
            }
        }

        ////////////////////////////////////////////////////////////
        ////// Sum up event weight here (after all corrections) ////
        ////////////////////////////////////////////////////////////
        dSumOfGenEvtWeight += eventWeight;
        // Fill PU related histograms (NPU, NTrueInt only available for MC)
        // This should be done before gen-lv patching and muon filtering
        // (since PU has nothing to do with gen-lv patching and muon filtering)
        hNPV->Fill(**(cData->NPV), eventWeight);
        if (bIsMC) {
            hNPU->Fill(**(cData->Pileup_nPU), eventWeight);
            hNTrueInt->Fill(**(cData->Pileup_nTrueInt), eventWeight);
        }

        ////////////////////////////////////////////////////////////
        ////// Apply Gen-lv patching and Gen-lv muon filtering /////
        ////////////////////////////////////////////////////////////
        if (bIsMC && bDoGenPatching) {
            Bool_t passedGenPatching = genPtcs->PassGenPatching(dHT_cut_high, dW_mass_cut_high);
            Bool_t passedMuonFiltering = genPtcs->PassMuonFiltering();

            // Skip event if Gen-lv patching failed
            if (!passedGenPatching) continue;

            // Fill Gen-lv inclusive W histograms before muon filtering
            if (genPtcs->IsInclusiveW()) {
                hGen_W_pT->Fill(genPtcs->GetGenW().Pt(), eventWeight);
                hGen_W_eta->Fill(genPtcs->GetGenW().Eta(), eventWeight);
                hGen_W_phi->Fill(genPtcs->GetGenW().Phi(), eventWeight);
                hGen_W_mass->Fill(genPtcs->GetGenW().M(), eventWeight);
            }

            // Skip event if muon filtering failed
            if (!passedMuonFiltering) continue;
        }

        ////////////////////////////////////////////////////////////
        //////////////// Event selection here //////////////////////
        ////////////////////////////////////////////////////////////
        // 1. Trigger
        // 2016APV : IsoMu24 || IsoTkMu24
        // 2016 : IsoMu24 || IsoTkMu24
        // 2017 : IsoMu27
        // 2018 : IsoMu24
        Bool_t passedTrigger = false;
        if (sEra == "2016APV") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));
        } else if (sEra == "2016") {
            passedTrigger = (**(cData->HLT_IsoMu24) || **(cData->HLT_IsoTkMu24));
        } else if (sEra == "2017") {
            passedTrigger = **(cData->HLT_IsoMu27);
        } else if (sEra == "2018") {
            passedTrigger = **(cData->HLT_IsoMu24);
        }
        if (!passedTrigger) continue;

        // 2. Noise filter
        Bool_t flag_goodVertices                       =  **(cData->Flag_goodVertices);
        Bool_t flag_globalSuperTightHalo2016Filter     =  **(cData->Flag_globalSuperTightHalo2016Filter);
        Bool_t flag_HBHENoiseFilter                    =  **(cData->Flag_HBHENoiseFilter);
        Bool_t flag_HBHENoiseIsoFilter                 =  **(cData->Flag_HBHENoiseIsoFilter);
        Bool_t flag_EcalDeadCellTriggerPrimitiveFilter =  **(cData->Flag_EcalDeadCellTriggerPrimitiveFilter);
        Bool_t flag_BadPFMuonFilter                    =  **(cData->Flag_BadPFMuonFilter);
        Bool_t flag_BadPFMuonDzFilter                  =  **(cData->Flag_BadPFMuonDzFilter);
        Bool_t flag_hfNoisyHitsFilter                  =  **(cData->Flag_hfNoisyHitsFilter);
        Bool_t flag_BadChargedCandidateFilter          =  **(cData->Flag_BadChargedCandidateFilter);
        Bool_t flag_eeBadScFilter                      =  **(cData->Flag_eeBadScFilter);
        Bool_t flag_ecalBadCalibFilter                 =  **(cData->Flag_ecalBadCalibFilter);
        bool passed_filter = (  flag_goodVertices                      &&
                                flag_globalSuperTightHalo2016Filter    &&
                                flag_HBHENoiseFilter                   &&
                                flag_HBHENoiseIsoFilter                &&
                                flag_EcalDeadCellTriggerPrimitiveFilter&&
                                flag_BadPFMuonFilter                   &&
                                flag_BadPFMuonDzFilter                 &&
                                flag_hfNoisyHitsFilter                 &&
                                flag_BadChargedCandidateFilter         &&
                                flag_eeBadScFilter                     &&
                                flag_ecalBadCalibFilter);
        if (!passed_filter) continue; // Skip event if noise filter failed

        // 3. Require leading and sub-leading muons
        std::vector<MuonHolder> ZCandidateCollection = muons->GetMuonsFromZ();
        std::sort(ZCandidateCollection.begin(), ZCandidateCollection.end(),
                [](const MuonHolder &a, const MuonHolder &b) {
                    return a.GetMuonRoccoVec().Pt() > b.GetMuonRoccoVec().Pt();
                });
        if( ZCandidateCollection.size() < 2 ) continue;

        // 4. mass_mumu > 10 GeV
        MuonHolder& leadingMuon = ZCandidateCollection[0];
        MuonHolder& subLeadingMuon = ZCandidateCollection[1];
        Double_t mass_mumu = (leadingMuon.GetMuonRoccoVec() + subLeadingMuon.GetMuonRoccoVec()).M();
        if (mass_mumu < 10.) continue;

        Double_t mass_mumu_org = (leadingMuon.GetMuonOrgVec() + subLeadingMuon.GetMuonOrgVec()).M();

        hDilepton_org_mass_after->Fill(mass_mumu_org, eventWeight);
        hDilepton_rocco_mass_after->Fill(mass_mumu, eventWeight);
    }
}

////////////////////////////////////////////////////////////
//////////////// Class initialization //////////////////////
////////////////////////////////////////////////////////////
void DYanalyzer::Init() {
    // Declare classes
    cData = new Data(sProcessName, sEra, sInputFileList, bIsMC);
    cPU = new PU(sEra);
    cEfficiencySF = new EfficiencySF(sEra, sHistName_ID, sHistName_Iso, sHistName_Trig);    
    cRochesterCorrection = new RoccoR(sRoccoFileName); // Rocco is initialized here

    // Initialize classes
    cData->Init();
    cPU->Init();
    cEfficiencySF->Init();

    // Set total events
    nTotalEvents = cData->GetTotalEvents(); 

    // Check process name and determine whether to perform Gen-lv patching
    this->CheckGenPatching();
    // Declare histograms
    this->DeclareHistograms();
    // Print initialization information
    this->PrintInitInfo();

    bIsInit = true;
}

// Check process name and determine whether to perform Gen-lv patching
void DYanalyzer::CheckGenPatching() {
    // Regex for WToMuNu
    // Captures process names like
    // WToMuNu_M-100, WToMuNu_M-200_v1, WToMuNu_M-500_ext-v1, ...
    std::regex WToMuNuRegex("^WToMuNu_M-(\\d+)(?:_.*)?$");
    std::smatch WToMuNuMatch;
    // Regex for WToTauNu
    // Captures process names like
    // WToTauNu_M-100, WToTauNu_M-200_v1, WToTauNu_M-500_ext-v1, ...
    std::regex WToTauNuRegex("^WToTauNu_M-(\\d+)(?:_.*)?$");
    std::smatch WToTauNuMatch;

    if (sProcessName == "WJetsToLNu") {
        dW_mass_cut_high = 100;
        dHT_cut_high = 100;
        bIsInclusiveW = true;
    } else if (sProcessName.find("WJetsToLNu_HT") != std::string::npos) {
        dW_mass_cut_high = 100;
        // dHT_cut_high = 100; // No need to restrict HT for boosted W
        bIsBoostedW = true;
    } else if (std::regex_match(sProcessName, WToMuNuMatch, WToMuNuRegex)) {
        dW_mass_cut_low = std::stod(WToMuNuMatch[1]);
        if(dW_mass_cut_low == 100) dW_mass_cut_high = 200;
        else if(dW_mass_cut_low == 200) dW_mass_cut_high = 500;
        else if(dW_mass_cut_low == 500) dW_mass_cut_high = 1000;
        else if(dW_mass_cut_low == 1000) dW_mass_cut_high = 2000;
        else dW_mass_cut_high = 1e9;
        bIsOffshellW = true;
    } else if (std::regex_match(sProcessName, WToTauNuMatch, WToTauNuRegex)) {
        dW_mass_cut_low = std::stod(WToTauNuMatch[1]);
        if(dW_mass_cut_low == 100) dW_mass_cut_high = 200;
        else if(dW_mass_cut_low == 200) dW_mass_cut_high = 500;
        else if(dW_mass_cut_low == 500) dW_mass_cut_high = 1000;
        else if(dW_mass_cut_low == 1000) dW_mass_cut_high = 2000;
        else dW_mass_cut_high = 1e9;
        bIsOffshellWToTauNu = true;
    }

    // Do Gen-lv patching if any of the conditions is true
    if (bIsInclusiveW || bIsBoostedW || bIsOffshellW || bIsOffshellWToTauNu)
        bDoGenPatching = true;
    else bDoGenPatching = false;
}

// Print initialization information
void DYanalyzer::PrintInitInfo() {
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - DYanalyzer is initialized" << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Process name: " << sProcessName << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Era: " << sEra << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Is MC: " << bIsMC << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do gen patching: " << bDoGenPatching << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do PU correction: " << bDoPUCorrection << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do L1 pre-firing correction: " << bDoL1PreFiringCorrection << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do ID SF: " << bDoIDSF << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do Iso SF: " << bDoIsoSF << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do Trig SF: " << bDoTrigSF << std::endl;
    std::cout << "[Info] DYanalyzer::PrintInitInfo() - Do rocco correction: " << bDoRocco << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;
}

// Simple utility to print progress
void DYanalyzer::PrintProgress(const int currentStep) {    
    float progress = (float)currentStep / nTotalEvents;
    int barWidth = 70;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; i++) {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "]  " << currentStep << "/" << nTotalEvents << "  " << int(progress * 100.0) << "%\r";
    std::cout.flush();
}

// Declare histograms
void DYanalyzer::DeclareHistograms() {
    ////////////////////////////////////////////////////////////
    // GenLevel event weights, before and after each correction
    ////////////////////////////////////////////////////////////
    hGenEvtWeight = new TH1D("hGenEvtWeight", "hGenEvtWeight", 1,0,1);
    hGenEvtWeight->Sumw2();
    ////////////////////////////////////////////////////////////
    // Before event selection
    ////////////////////////////////////////////////////////////

    // GenLevel Object histograms
    hGen_Muon_pT  = new TH1D("hGen_Muon_pT",  "hGen_Muon_pT",  3000, 0, 3000);
    hGen_Muon_pT->Sumw2();
    hGen_Muon_phi = new TH1D("hGen_Muon_phi", "hGen_Muon_phi", 72, -M_PI, M_PI);
    hGen_Muon_phi->Sumw2();
    hGen_Muon_eta = new TH1D("hGen_Muon_eta", "hGen_Muon_eta", 50, -2.5, 2.5);
    hGen_Muon_eta->Sumw2();

    hGen_Nu_pT  = new TH1D("hGen_Nu_pT",  "hGen_Nu_pT",  3000, 0, 3000);
    hGen_Nu_pT->Sumw2();
    hGen_Nu_phi = new TH1D("hGen_Nu_phi", "hGen_Nu_phi", 72, -M_PI, M_PI);
    hGen_Nu_phi->Sumw2();
    hGen_Nu_eta = new TH1D("hGen_Nu_eta", "hGen_Nu_eta", 50, -2.5, 2.5);
    hGen_Nu_eta->Sumw2();

    hGen_MET_phi   = new TH1D("hGen_MET_phi", "hGen_MET_phi", 72, -M_PI, M_PI);
    hGen_MET_phi->Sumw2();
    hGen_MET_pT    = new TH1D("hGen_MET_pT", "hGen_MET_pT", 3000, 0, 3000);
    hGen_MET_pT->Sumw2();
    
    // For GenLevel W decaying to muon and neutrino
    hGen_WToMuNu_pT    = new TH1D("hGen_WToMuNu_pT", "hGen_WToMuNu_pT", 3000, 0, 3000);
    hGen_WToMuNu_pT->Sumw2();
    hGen_WToMuNu_eta   = new TH1D("hGen_WToMuNu_eta", "hGen_WToMuNu_eta", 50, -2.5, 2.5);
    hGen_WToMuNu_eta->Sumw2();
    hGen_WToMuNu_phi   = new TH1D("hGen_WToMuNu_phi", "hGen_WToMuNu_phi", 72, -M_PI, M_PI);
    hGen_WToMuNu_phi->Sumw2();
    hGen_WToMuNu_mass  = new TH1D("hGen_WToMuNu_mass", "hGen_WToMuNu_mass", 3000, 0, 3000);
    hGen_WToMuNu_mass->Sumw2();
    hGen_WToMuNu_MT    = new TH1D("hGen_WToMuNu_MT", "hGen_WToMuNu_MT", 3000, 0, 3000);
    hGen_WToMuNu_MT->Sumw2();

    // For GenLevel inclusive decaying W
    hGen_W_pT    = new TH1D("hGen_W_pT", "hGen_W_pT", 3000, 0, 3000);
    hGen_W_pT->Sumw2();
    hGen_W_eta   = new TH1D("hGen_W_eta", "hGen_W_eta", 50, -2.5, 2.5);
    hGen_W_eta->Sumw2();
    hGen_W_phi   = new TH1D("hGen_W_phi", "hGen_W_phi", 72, -M_PI, M_PI);
    hGen_W_phi->Sumw2();
    hGen_W_mass  = new TH1D("hGen_W_mass", "hGen_W_mass", 3000, 0, 3000);
    hGen_W_mass->Sumw2();
    hGen_W_MT    = new TH1D("hGen_W_MT", "hGen_W_MT", 3000, 0, 3000);
    hGen_W_MT->Sumw2();
    
    // For LHE HT
    hLHE_HT = new TH1D("hLHE_HT", "hLHE_HT", 3000, 0, 3000);
    hLHE_HT->Sumw2();

    // Object histograms
    hMuon_pT  = new TH1D("hMuon_pT", "hMuon_pT", 3000, 0, 3000);
    hMuon_pT->Sumw2();
    hMuon_phi = new TH1D("hMuon_phi", "hMuon_phi", 72, -M_PI, M_PI);
    hMuon_phi->Sumw2();
    hMuon_eta = new TH1D("hMuon_eta", "hMuon_eta", 50, -2.5, 2.5);
    hMuon_eta->Sumw2();
    hMuon_mass = new TH1D("hMuon_mass", "hMuon_mass", 1000, 0, 1);
    hMuon_mass->Sumw2();

    hMET_phi   = new TH1D("hMET_phi", "hMET_phi", 72, -M_PI, M_PI);
    hMET_phi->Sumw2();
    hMET_pT    = new TH1D("hMET_pT", "hMET_pT", 3000, 0, 3000);
    hMET_pT->Sumw2();
    hMET_sumET = new TH1D("hMET_sumET", "hMET_sumET", 3000, 0, 3000);
    hMET_sumET->Sumw2();

    hPFMET_phi   = new TH1D("hPFMET_phi", "hPFMET_phi", 72, -M_PI, M_PI);
    hPFMET_phi->Sumw2();
    hPFMET_pT    = new TH1D("hPFMET_pT", "hPFMET_pT", 3000, 0, 3000);
    hPFMET_pT->Sumw2();
    hPFMET_sumET = new TH1D("hPFMET_sumET", "hPFMET_sumET", 3000, 0, 3000);
    hPFMET_sumET->Sumw2();

    hPFMET_corr_phi   = new TH1D("hPFMET_corr_phi", "hPFMET_corr_phi", 72, -M_PI, M_PI);
    hPFMET_corr_phi->Sumw2();
    hPFMET_corr_pT    = new TH1D("hPFMET_corr_pT", "hPFMET_corr_pT", 3000, 0, 3000);
    hPFMET_corr_pT->Sumw2();
    hPFMET_corr_sumET = new TH1D("hPFMET_corr_sumET", "hPFMET_corr_sumET", 3000, 0, 3000);
    hPFMET_corr_sumET->Sumw2();

    // Reconstructed W histograms
    hDeltaPhi_Mu_MET = new TH1D("hDeltaPhi_Mu_MET", "hDeltaPhi_Mu_MET", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_MET->Sumw2();
    hW_MT            = new TH1D("hW_MT", "hW_MT", 3000, 0, 3000);
    hW_MT->Sumw2();

    hDeltaPhi_Mu_PFMET = new TH1D("hDeltaPhi_Mu_PFMET", "hDeltaPhi_Mu_PFMET", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_PFMET->Sumw2();
    hW_MT_PFMET        = new TH1D("hW_MT_PFMET", "hW_MT_PFMET", 3000, 0, 3000);
    hW_MT_PFMET->Sumw2();

    hDeltaPhi_Mu_PFMET_corr = new TH1D("hDeltaPhi_Mu_PFMET_corr", "hDeltaPhi_Mu_PFMET_corr", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_PFMET_corr->Sumw2();
    hW_MT_PFMET_corr        = new TH1D("hW_MT_PFMET_corr", "hW_MT_PFMET_corr", 3000, 0, 3000);
    hW_MT_PFMET_corr->Sumw2();

    // For NPV, NPU, NTrueInt before event selection
    // For NTrueInt -> Only present in MC
    hNPV = new TH1D("hNPV", "hNPV", 100, 0, 100);
    hNPV->Sumw2();
    hNPU = new TH1D("hNPU", "hNPU", 100, 0, 100);
    hNPU->Sumw2();
    hNTrueInt = new TH1D("hNTrueInt", "hNTrueInt", 100, 0, 100);
    hNTrueInt->Sumw2();


    ////////////////////////////////////////////////////////////
    // After event selection 
    ////////////////////////////////////////////////////////////

    // GenLevel Object histograms    
    hGen_Muon_pT_after = new TH1D("hGen_Muon_pT_after", "hGen_Muon_pT_after", 3000, 0, 3000);
    hGen_Muon_pT_after->Sumw2();
    hGen_Muon_phi_after = new TH1D("hGen_Muon_phi_after", "hGen_Muon_phi_after", 72, -M_PI, M_PI);
    hGen_Muon_phi_after->Sumw2();
    hGen_Muon_eta_after = new TH1D("hGen_Muon_eta_after", "hGen_Muon_eta_after", 50, -2.5, 2.5);
    hGen_Muon_eta_after->Sumw2();

    hGen_Nu_pT_after = new TH1D("hGen_Nu_pT_after", "hGen_Nu_pT_after", 3000, 0, 3000);
    hGen_Nu_pT_after->Sumw2();
    hGen_Nu_phi_after = new TH1D("hGen_Nu_phi_after", "hGen_Nu_phi_after", 72, -M_PI, M_PI);
    hGen_Nu_phi_after->Sumw2();
    hGen_Nu_eta_after = new TH1D("hGen_Nu_eta_after", "hGen_Nu_eta_after", 50, -2.5, 2.5);
    hGen_Nu_eta_after->Sumw2();

    hGen_MET_phi_after   = new TH1D("hGen_MET_phi_after", "hGen_MET_phi_after", 72, -M_PI, M_PI);
    hGen_MET_phi_after->Sumw2();
    hGen_MET_pT_after    = new TH1D("hGen_MET_pT_after", "hGen_MET_pT_after", 3000, 0, 3000);
    hGen_MET_pT_after->Sumw2();
    
    // For GenLevel W decaying to muon and neutrino
    hGen_WToMuNu_pT_after    = new TH1D("hGen_WToMuNu_pT_after", "hGen_WToMuNu_pT_after", 3000, 0, 3000);
    hGen_WToMuNu_pT_after->Sumw2();
    hGen_WToMuNu_eta_after   = new TH1D("hGen_WToMuNu_eta_after", "hGen_WToMuNu_eta_after", 50, -2.5, 2.5);
    hGen_WToMuNu_eta_after->Sumw2();
    hGen_WToMuNu_phi_after   = new TH1D("hGen_WToMuNu_phi_after", "hGen_WToMuNu_phi_after", 72, -M_PI, M_PI);
    hGen_WToMuNu_phi_after->Sumw2();
    hGen_WToMuNu_mass_after  = new TH1D("hGen_WToMuNu_mass_after", "hGen_WToMuNu_mass_after", 3000, 0, 3000);
    hGen_WToMuNu_mass_after->Sumw2();
    hGen_WToMuNu_MT_after    = new TH1D("hGen_WToMuNu_MT_after", "hGen_WToMuNu_MT_after", 3000, 0, 3000);
    hGen_WToMuNu_MT_after->Sumw2();

    // For GenLevel inclusive decaying W
    hGen_W_pT_after    = new TH1D("hGen_W_pT_after", "hGen_W_pT_after", 3000, 0, 3000);
    hGen_W_pT_after->Sumw2();
    hGen_W_eta_after   = new TH1D("hGen_W_eta_after", "hGen_W_eta_after", 50, -2.5, 2.5);
    hGen_W_eta_after->Sumw2();
    hGen_W_phi_after   = new TH1D("hGen_W_phi_after", "hGen_W_phi_after", 72, -M_PI, M_PI);
    hGen_W_phi_after->Sumw2();
    hGen_W_mass_after  = new TH1D("hGen_W_mass_after", "hGen_W_mass_after", 3000, 0, 3000);
    hGen_W_mass_after->Sumw2();
    hGen_W_MT_after    = new TH1D("hGen_W_MT_after", "hGen_W_MT_after", 3000, 0, 3000);
    hGen_W_MT_after->Sumw2();
    
    // For LHE HT
    hLHE_HT_after = new TH1D("hLHE_HT_after", "hLHE_HT_after", 3000, 0, 3000);
    hLHE_HT_after->Sumw2();

    // Object histograms
    hMuon_pT_after  = new TH1D("hMuon_pT_after", "hMuon_pT_after", 3000, 0, 3000);
    hMuon_pT_after->Sumw2();
    hMuon_phi_after = new TH1D("hMuon_phi_after", "hMuon_phi_after", 72, -M_PI, M_PI);
    hMuon_phi_after->Sumw2();
    hMuon_eta_after = new TH1D("hMuon_eta_after", "hMuon_eta_after", 50, -2.5, 2.5);
    hMuon_eta_after->Sumw2();
    hMuon_mass_after = new TH1D("hMuon_mass_after", "hMuon_mass_after", 1000, 0, 1);
    hMuon_mass_after->Sumw2();

    hMET_phi_after   = new TH1D("hMET_phi_after", "hMET_phi_after", 72, -M_PI, M_PI);
    hMET_phi_after->Sumw2();
    hMET_pT_after    = new TH1D("hMET_pT_after", "hMET_pT_after", 3000, 0, 3000);
    hMET_pT_after->Sumw2();
    hMET_sumET_after = new TH1D("hMET_sumET_after", "hMET_sumET_after", 3000, 0, 3000);
    hMET_sumET_after->Sumw2();
    
    hPFMET_phi_after   = new TH1D("hPFMET_phi_after", "hPFMET_phi_after", 72, -M_PI, M_PI);
    hPFMET_phi_after->Sumw2();
    hPFMET_pT_after    = new TH1D("hPFMET_pT_after", "hPFMET_pT_after", 3000, 0, 3000);
    hPFMET_pT_after->Sumw2();
    hPFMET_sumET_after = new TH1D("hPFMET_sumET_after", "hPFMET_sumET_after", 3000, 0, 3000);
    hPFMET_sumET_after->Sumw2();

    hPFMET_corr_phi_after   = new TH1D("hPFMET_corr_phi_after", "hPFMET_corr_phi_after", 72, -M_PI, M_PI);
    hPFMET_corr_phi_after->Sumw2();
    hPFMET_corr_pT_after    = new TH1D("hPFMET_corr_pT_after", "hPFMET_corr_pT_after", 3000, 0, 3000);
    hPFMET_corr_pT_after->Sumw2();
    hPFMET_corr_sumET_after = new TH1D("hPFMET_corr_sumET_after", "hPFMET_corr_sumET_after", 3000, 0, 3000);
    hPFMET_corr_sumET_after->Sumw2();

    // Reconstructed W histograms
    hDeltaPhi_Mu_MET_after = new TH1D("hDeltaPhi_Mu_MET_after", "hDeltaPhi_Mu_MET_after", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_MET_after->Sumw2();
    hW_MT_after            = new TH1D("hW_MT_after", "hW_MT_after", 3000, 0, 3000);
    hW_MT_after->Sumw2();

    hDeltaPhi_Mu_PFMET_after = new TH1D("hDeltaPhi_Mu_PFMET_after", "hDeltaPhi_Mu_PFMET_after", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_PFMET_after->Sumw2();
    hW_MT_PFMET_after        = new TH1D("hW_MT_PFMET_after", "hW_MT_PFMET_after", 3000, 0, 3000);
    hW_MT_PFMET_after->Sumw2();

    hDeltaPhi_Mu_PFMET_corr_after = new TH1D("hDeltaPhi_Mu_PFMET_corr_after", "hDeltaPhi_Mu_PFMET_corr_after", 72, -M_PI, M_PI);
    hDeltaPhi_Mu_PFMET_corr_after->Sumw2();
    hW_MT_PFMET_corr_after        = new TH1D("hW_MT_PFMET_corr_after", "hW_MT_PFMET_corr_after", 3000, 0, 3000);
    hW_MT_PFMET_corr_after->Sumw2();

    // For NPV, NPU, NTrueInt before event selection
    // For NTrueInt -> Only present in MC
    hNPV_after = new TH1D("hNPV_after", "hNPV_after", 100, 0, 100);
    hNPV_after->Sumw2();
    hNPU_after = new TH1D("hNPU_after", "hNPU_after", 100, 0, 100);
    hNPU_after->Sumw2();
    hNTrueInt_after = new TH1D("hNTrueInt_after", "hNTrueInt_after", 100, 0, 100);
    hNTrueInt_after->Sumw2();

    ////////////////////////////////////////////////////////////
    // For Z peak mass study
    ////////////////////////////////////////////////////////////
    hDilepton_org_mass     = new TH1D("hDilepton_org_mass", "hDilepton_org_mass", 3000, 0, 3000);
    hDilepton_org_mass->Sumw2();
    hDilepton_rocco_mass   = new TH1D("hDilepton_rocco_mass", "hDilepton_rocco_mass", 3000, 0, 3000);
    hDilepton_rocco_mass->Sumw2();

    hDilepton_org_mass_after     = new TH1D("hDilepton_org_mass_after", "hDilepton_org_mass_after", 3000, 0, 3000);
    hDilepton_org_mass_after->Sumw2();
    hDilepton_rocco_mass_after   = new TH1D("hDilepton_rocco_mass_after", "hDilepton_rocco_mass_after", 3000, 0, 3000);
    hDilepton_rocco_mass_after->Sumw2();
}

// Write histograms to file
void DYanalyzer::WriteHistograms(TFile* f_output) {
    f_output->cd();

    ////////////////////////////////////////////////////////////
    // GenLevel event weights, before and after each correction
    ////////////////////////////////////////////////////////////
    hGenEvtWeight->Write();

    ////////////////////////////////////////////////////////////
    // Before event selection
    ////////////////////////////////////////////////////////////

    // GenLevel Object histograms
    hGen_Muon_pT->Write();
    hGen_Muon_phi->Write();
    hGen_Muon_eta->Write();

    hGen_Nu_pT->Write();
    hGen_Nu_phi->Write();
    hGen_Nu_eta->Write();

    hGen_MET_phi->Write();
    hGen_MET_pT->Write();

    // For GenLevel W decaying to muon and neutrino
    hGen_WToMuNu_pT->Write();
    hGen_WToMuNu_eta->Write();
    hGen_WToMuNu_phi->Write();
    hGen_WToMuNu_mass->Write();
    hGen_WToMuNu_MT->Write();

    // For GenLevel inclusive decaying W
    hGen_W_pT->Write();
    hGen_W_eta->Write();
    hGen_W_phi->Write();
    hGen_W_mass->Write();
    hGen_W_MT->Write();

    // For LHE HT
    hLHE_HT->Write();

    // Object histograms
    hMuon_pT->Write();
    hMuon_phi->Write();
    hMuon_eta->Write();
    hMuon_mass->Write();

    hMET_phi->Write();
    hMET_pT->Write();
    hMET_sumET->Write();

    hPFMET_phi->Write();
    hPFMET_pT->Write();
    hPFMET_sumET->Write();

    hPFMET_corr_phi->Write();
    hPFMET_corr_pT->Write();
    hPFMET_corr_sumET->Write();

    // Reconstructed W histograms
    hDeltaPhi_Mu_MET->Write();
    hW_MT->Write();

    hDeltaPhi_Mu_PFMET->Write();
    hW_MT_PFMET->Write();

    hDeltaPhi_Mu_PFMET_corr->Write();
    hW_MT_PFMET_corr->Write();

    // For NPV, NPU, NTrueInt before event selection
    // For NTrueInt -> Only present in MC
    hNPV->Write();
    hNPU->Write();
    hNTrueInt->Write();

    ////////////////////////////////////////////////////////////
    // After event selection 
    ////////////////////////////////////////////////////////////

    // GenLevel Object histograms
    hGen_Muon_pT_after->Write();
    hGen_Muon_phi_after->Write();
    hGen_Muon_eta_after->Write();

    hGen_Nu_pT_after->Write();
    hGen_Nu_phi_after->Write();
    hGen_Nu_eta_after->Write();

    hGen_MET_phi_after->Write();
    hGen_MET_pT_after->Write();

    // For GenLevel W decaying to muon and neutrino
    hGen_WToMuNu_pT_after->Write();
    hGen_WToMuNu_eta_after->Write();
    hGen_WToMuNu_phi_after->Write();
    hGen_WToMuNu_mass_after->Write();
    hGen_WToMuNu_MT_after->Write();

    // For GenLevel inclusive decaying W
    // This cannot exist because after event selection, muon filtering is already done
    hGen_W_pT_after->Write();
    hGen_W_eta_after->Write();
    hGen_W_phi_after->Write();
    hGen_W_mass_after->Write();
    hGen_W_MT_after->Write();

    // For LHE HT
    hLHE_HT_after->Write();

    // Object histograms
    hMuon_pT_after->Write();
    hMuon_phi_after->Write();
    hMuon_eta_after->Write();
    hMuon_mass_after->Write();

    hMET_phi_after->Write();
    hMET_pT_after->Write();
    hMET_sumET_after->Write();

    hPFMET_phi_after->Write();
    hPFMET_pT_after->Write();
    hPFMET_sumET_after->Write();

    hPFMET_corr_phi_after->Write();
    hPFMET_corr_pT_after->Write();
    hPFMET_corr_sumET_after->Write();

    // Reconstructed W histograms
    hDeltaPhi_Mu_MET_after->Write();
    hW_MT_after->Write();

    hDeltaPhi_Mu_PFMET_after->Write();
    hW_MT_PFMET_after->Write();

    hDeltaPhi_Mu_PFMET_corr_after->Write();
    hW_MT_PFMET_corr_after->Write();

    // For NPV, NPU, NTrueInt before event selection
    // For NTrueInt -> Only present in MC
    hNPV_after->Write();
    hNPU_after->Write();
    hNTrueInt_after->Write();

    ////////////////////////////////////////////////////////////
    // For Z peak mass study
    ////////////////////////////////////////////////////////////

    hDilepton_org_mass->Write();
    hDilepton_rocco_mass->Write();

    hDilepton_org_mass_after->Write();
    hDilepton_rocco_mass_after->Write();
}

DYanalyzer::~DYanalyzer() {
    Clear();
}

void DYanalyzer::Clear() {
    // delete classes
    delete cData;
    delete cPU;
    delete cEfficiencySF;
    delete cRochesterCorrection;
}