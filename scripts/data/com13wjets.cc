#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <map>
#include "Pythia8/Pythia.h"
#include "fastjet/ClusterSequence.hh"

using namespace Pythia8;
using namespace fastjet;

double invariantMass(const std::vector<Vec4>& momenta) {
    Vec4 total;
    for (const auto& p : momenta) {
        total += p;
    }
    return total.mCalc();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_file>" << std::endl;
        return 1;
    }

    std::ofstream outFile(argv[1]);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << argv[1] << std::endl;
        return 1;
    }

    int nEvents = 10000;
    Pythia pythia;

    //Pythia initialization
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");
    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 100.e3");
    pythia.readString("HiggsSM:all  = on");
    pythia.readString("25:onMode = on");
    pythia.init();

    // Jet definition (anti-kt, R=0.4)
    double R = 0.4;
    JetDefinition jet_def(antikt_algorithm, R);

    int totalHCount = 0; //total

    //Headers
    outFile << "ProductionChannel,DecayProducts,InvMasses,Jet_PT,Jet_Eta,Jet_Phi,Jet_Mass,Jet_ID\n";

    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        //Decays of detected Higgs particles only
        for (int j = 0; j < pythia.event.size(); j++) {
            std::vector<int> validStatuses = {-62};
            if (pythia.event[j].id() == 25 && (std::find(validStatuses.begin(), validStatuses.end(), pythia.event[j].status()) != validStatuses.end())) {
                totalHCount++;

                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;
                int productionChannel = pythia.info.code();

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                    }
                }

                // Output the production channel, decay products, and their invariant masses
                if (decayProducts.size() >= 2) {
                    outFile << productionChannel << ",";

                    for (size_t d = 0; d < decayProducts.size(); d++) {
                        outFile << decayProducts[d];
                        if (d < decayProducts.size() - 1) outFile << ";";
                    }
                    outFile << ",";

                    std::vector<double> invMasses;
                    size_t n = momenta.size();

                    for (size_t a = 0; a < (1 << n); a++) {
                        std::vector<Vec4> selectedMomenta;
                        for (size_t b = 0; b < n; b++) {
                            if (a & (1 << b)) {
                                selectedMomenta.push_back(momenta[b]);
                            }
                        }
                        if (selectedMomenta.size() >= 2) {
                            double invMass = invariantMass(selectedMomenta);
                            invMasses.push_back(invMass);
                        }
                    }

                    for (size_t m = 0; m < invMasses.size(); m++) {
                        outFile << invMasses[m];
                        if (m < invMasses.size() - 1) outFile << ";";
                    }
                    outFile << ",";

                    // Jet clustering on final-state particles
                    std::vector<PseudoJet> particles;
                    for (int k = 0; k < pythia.event.size(); k++) {
                        if (pythia.event[k].isFinal()) {
                            PseudoJet particle(pythia.event[k].px(), pythia.event[k].py(), pythia.event[k].pz(), pythia.event[k].e());
                            particle.set_user_index(k);
                            particles.push_back(particle);
                        }
                    }

                    if (!particles.empty()) {
                        ClusterSequence cs(particles, jet_def);
                        std::vector<PseudoJet> jets = sorted_by_pt(cs.inclusive_jets());

                        int jet_id = 0;
                        std::map<int, int> particleToJetMap; // Map particle index to its Jet_ID

                        for (const auto& jet : jets) {
                            
                            outFile << jet.pt() << "," << jet.eta() << "," << jet.phi() << "," << jet.m() << ","; //Jet data

                            // Associate particles in the jet with the Jet_ID
                            std::vector<PseudoJet> constituents = jet.constituents();
                            for (const auto& constituent : constituents) {
                                int index = constituent.user_index();
                                particleToJetMap[index] = jet_id;
                            }
                            jet_id++;
                        }

                        // Output particle-to-jet associations for each particle in the event
                        for (int k = 0; k < pythia.event.size(); k++) {
                            if (particleToJetMap.count(k)) {
                                outFile << particleToJetMap[k];
                            } else {
                                outFile << "-1"; 
                            }
                            if (k < pythia.event.size() - 1) outFile << ";";
                        }

                        outFile << "\n"; 
                    }
                }
            }
        }
    }

    outFile.close();
    return 0;
}
