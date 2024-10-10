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

// Recursive function to trace a particle to its final state descendants
void traceToFinalState(const Event& event, int index, std::vector<int>& finalStateParticles) {
    // Check if the particle is final state
    if (event[index].isFinal()) {
        finalStateParticles.push_back(index);
        return;
    }

    // Recursively follow daughters if they exist
    for (int d = event[index].daughter1(); d <= event[index].daughter2(); ++d) {
        if (d > 0 && d < event.size()) {
            traceToFinalState(event, d, finalStateParticles);
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "Checkpoint: Starting program." << std::endl;

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

    std::cout << "Checkpoint: Output file opened successfully." << std::endl;

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

    std::cout << "Checkpoint: Pythia initialized." << std::endl;

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
        std::cout << "Checkpoint: Starting event " << i << std::endl;
        if (!pythia.next()) continue;

        // Loop over all particles to detect Higgs decays
        for (int j = 0; j < pythia.event.size(); j++) {
            if (pythia.event[j].id() == 25 && pythia.event[j].status() == -62) {  // Higgs status -62 indicates it has decayed
                totalHCount++;
                std::cout << "Checkpoint: Higgs found in event " << i << ", index " << j << std::endl;

                // Store decay products and their momenta
                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;
                int productionChannel = pythia.info.code();

                std::map<int, int> particleIdToIndexMap;

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                        particleIdToIndexMap[pythia.event[k].id()] = k;
                    }
                }

                std::cout << "Checkpoint: Decay products and momenta collected." << std::endl;

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

                    std::cout << "Checkpoint: Invariant mass calculated." << std::endl;

                    // Perform jet clustering on final-state particles
                    std::vector<PseudoJet> particles;
                    for (int k = 0; k < pythia.event.size(); k++) {
                        if (pythia.event[k].isFinal()) {
                            PseudoJet particle(pythia.event[k].px(), pythia.event[k].py(), pythia.event[k].pz(), pythia.event[k].e());
                            particle.set_user_index(k);
                            particles.push_back(particle);
                        }
                    }

                    std::cout << "Checkpoint: Final state particles prepared for jet clustering." << std::endl;

                    // Cluster particles into jets
                    if (!particles.empty()) {
                        ClusterSequence cs(particles, jet_def);
                        std::vector<PseudoJet> jets = sorted_by_pt(cs.inclusive_jets());
                        std::map<int, int> particleToJetMap; // Map particle index to Jet_ID

                        for (size_t jetId = 0; jetId < jets.size(); ++jetId) {
                            std::vector<PseudoJet> constituents = jets[jetId].constituents();
                            for (const auto& constituent : constituents) {
                                particleToJetMap[constituent.user_index()] = jetId;
                            }
                        }

                        std::cout << "Checkpoint: Jet clustering completed." << std::endl;

                        // For each decay product, trace its final state descendants and check if they belong to a jet
                        std::set<int> jetIDsWithDecayProducts;
                        for (int decayIndex : decayProducts) {
                            int indexInEvent = particleIdToIndexMap[decayIndex];
                            std::vector<int> finalStateParticles;
                            traceToFinalState(pythia.event, indexInEvent, finalStateParticles);

                            std::cout << "Checkpoint: Family traced for decay product " << decayIndex << std::endl;

                            // Check if any final state particle is in a jet
                            for (int finalStateIndex : finalStateParticles) {
                                if (particleToJetMap.count(finalStateIndex)) {
                                    jetIDsWithDecayProducts.insert(particleToJetMap[finalStateIndex]);
                                    particleToJetMap[decayIndex] = particleToJetMap[finalStateIndex];
                                }
                            }
                        }

                        // Output jet data for each decay product's daughter particles
                        for (const std::string& property : {"pt", "eta", "phi", "m"}) {
                            for (int decayIndex = 0 ; decayIndex < decayProducts.size(); decayIndex++) {
                                if (particleToJetMap.count(decayProducts[decayIndex])) {
                                    int jetId = particleToJetMap[decayProducts[decayIndex]];
                                    if (jetId < jets.size()) {  // Check if jetId is within the valid range
                                        PseudoJet jet = jets[jetId];
                                        if (property == "pt") {
                                            outFile << jet.pt();
                                        } else if (property == "eta") {
                                            outFile << jet.eta();
                                        } else if (property == "phi") {
                                            outFile << jet.phi();
                                        } else if (property == "m") {
                                            outFile << jet.m();
                                        }
                                    } else {
                                        outFile << "-1";  // If jetId is out of range
                                    }
                                } else {
                                    outFile << "-1";  // If decayIndex doesn't map to a jet
                                }

                                if (decayIndex != 1) outFile << ";";
                            }
                            outFile << ",";  // Move to the next column for the next property
                        }

                        // Output Jet ID for each decay product
                        for (int decayIndex = 0 ; decayIndex < decayProducts.size(); decayIndex++) {
                            if (particleToJetMap.count(decayProducts[decayIndex])) {
                                outFile << particleToJetMap[decayProducts[decayIndex]];
                            } else {
                                outFile << "-1";
                            }

                            if (decayIndex != 1) outFile << ";";
                        }
                        outFile << "\n";
                    }
                }
            }
        }
    }

    // Close the output file
    outFile.close();
    std::cout << "Checkpoint: Output file closed, program completed." << std::endl;
    return 0;
}
