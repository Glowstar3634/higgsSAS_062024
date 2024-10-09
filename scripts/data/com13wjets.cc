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

// Function to calculate invariant mass from a set of momenta
double invariantMass(const std::vector<Vec4>& momenta) {
    Vec4 total;
    for (const auto& p : momenta) {
        total += p;
    }
    return total.mCalc();
}

int main(int argc, char* argv[]) {
    // Check for correct number of arguments (output file name)
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_file>" << std::endl;
        return 1;
    }

    // Open the output file for writing
    std::ofstream outFile(argv[1]);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << argv[1] << std::endl;
        return 1;
    }

    // Initialize Pythia with proton-proton collisions at 100 TeV and enable Higgs processes
    Pythia pythia;
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");
    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 100.e3");
    pythia.readString("HiggsSM:all  = on");
    pythia.readString("25:onMode = on");
    pythia.init();

    // Anti-kt jet clustering with R = 0.4
    double R = 0.4;
    JetDefinition jet_def(antikt_algorithm, R);

    // Variables to keep track of event counts
    int nEvents = 10;
    int totalHCount = 0;

    // Write headers to the output file
    outFile << "ProductionChannel,DecayProducts,InvMasses,Jet_PT,Jet_Eta,Jet_Phi,Jet_Mass,Jet_ID\n";

    // Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        // Loop over all particles to detect Higgs decays
        for (int j = 0; j < pythia.event.size(); j++) {
            if (pythia.event[j].id() == 25 && pythia.event[j].status() == -62) {  // Higgs status -62 indicates it has decayed
                totalHCount++;

                // Store decay products and their momenta
                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;
                int productionChannel = pythia.info.code();

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                    }
                }

                // Output the production channel and decay products
                if (decayProducts.size() >= 2) {
                    outFile << productionChannel << ",";

                    for (size_t d = 0; d < decayProducts.size(); d++) {
                        outFile << decayProducts[d];
                        if (d < decayProducts.size() - 1) outFile << ";";
                    }
                    outFile << ",";

                    // Calculate and output invariant mass of the decay products
                    double invMass = invariantMass(momenta);
                    outFile << invMass << ",";

                    // Perform jet clustering on final-state particles
                    std::vector<PseudoJet> particles;
                    for (int k = 0; k < pythia.event.size(); k++) {
                        if (pythia.event[k].isFinal()) {
                            PseudoJet particle(pythia.event[k].px(), pythia.event[k].py(), pythia.event[k].pz(), pythia.event[k].e());
                            particle.set_user_index(k);
                            particles.push_back(particle);
                        }
                    }

                    // Cluster particles into jets
                    if (!particles.empty()) {
                        ClusterSequence cs(particles, jet_def);
                        std::vector<PseudoJet> jets = sorted_by_pt(cs.inclusive_jets());

                        // Select the leading jet (or another criteria)
                        if (!jets.empty()) {
                            const PseudoJet& leadingJet = jets[0]; // For example, select the leading jet
                            outFile << leadingJet.pt() << "," << leadingJet.eta() << "," << leadingJet.phi() << "," << leadingJet.m() << ","; // Output jet data

                            // Optionally, associate decay products with the leading jet
                            std::vector<PseudoJet> constituents = leadingJet.constituents();
                            std::map<int, int> particleToJetMap; // Map particle index to its Jet_ID
                            for (const auto& constituent : constituents) {
                                int index = constituent.user_index();
                                particleToJetMap[index] = 0; // Assuming jet_id is 0 for the leading jet
                            }

                            // Output particle-to-jet associations only for decay products
                            for (const auto& decayIndex : decayProducts) {
                                int finalStateIndex = pythia.event[decayIndex].index();
                                if (particleToJetMap.count(finalStateIndex)) {
                                    outFile << particleToJetMap[finalStateIndex];
                                } else {
                                    outFile << "-1"; 
                                }
                                outFile << ";";  // Separate decay products with a semicolon
                            }

                            outFile << "\n"; 
                        }
                    }
                }
            }
        }
    }

    // Close the output file
    outFile.close();
    return 0;
}
