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

int identifyProductionChannel(const std::vector<int>& incomingPartonIDs) {
    if (incomingPartonIDs.size() < 2) return 0;

    int id1 = std::abs(incomingPartonIDs[0]);
    int id2 = std::abs(incomingPartonIDs[1]);
    std::cout << "id1: " << id1 << ", id2: " << id2 << std::endl;

    // ggH (Gluon-Gluon Fusion)
    if (id1 == 21 && id2 == 21) {
        return 1;  // HiggsSM:gg2H (gg -> H^0 via gluon fusion)
    }
    
    // VBF via ZZ fusion (both partons are quarks, but not identical flavors)
    else if ((id1 <= 6 && id2 <= 6) && (id1 != id2) && id1 != 2 && id2 != 2) {
        return 2;  // HiggsSM:ff2Hff(t:ZZ) (ff' -> H^0 ff' via ZZ fusion)
    }

    // VBF via WW fusion (one of the partons is an up-type quark, the other down-type)
    else if ((id1 == 2 || id2 == 2) && id1 <= 6 && id2 <= 6 && id1 != id2) {
        return 3;  // HiggsSM:ff2Hff(t:WW) (ff' -> H^0 ff' via WW fusion)
    }

    // VH (Associated Production with Z boson)
    else if ((id1 <= 6 && id2 <= 6) && (id1 == 23 || id2 == 23)) {
        return 4;  // HiggsSM:ffbar2HZ (f fbar -> H^0 Z^0)
    }

    // VH (Associated Production with W boson)
    else if ((id1 <= 6 && id2 <= 6) && (id1 == 24 || id2 == 24)) {
        return 5;  // HiggsSM:ffbar2HW (f fbar -> H^0 W^+-)
    }

    // ttH (Associated Production with Top Quarks - gluon fusion)
    else if (id1 == 6 && id2 == 6) {
        return 6;  // HiggsSM:gg2Httbar (gg -> H^0 ttbar)
    }

    // ttH (Associated Production with Top Quarks - quark fusion)
    else if (id1 <= 5 && id2 <= 5) {
        return 7;  // HiggsSM:qqbar2Httbar (qqbar -> H^0 ttbar)
    }

    return 0;  // Unknown or not a primary production channel
}


std::vector<int> parseLHEProductionChannels(const std::string& filename) {
    std::ifstream inFile(filename);
    std::vector<int> productionChannels;
    std::string line;

    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open LHE file: " << filename << std::endl;
        return productionChannels;
    }

    while (std::getline(inFile, line)) {
        if (line.find("<event>") != std::string::npos) {
            std::getline(inFile, line); // Header line
            std::vector<int> incomingPartonIDs;

            // Process particles in the event to find incoming partons
            while (std::getline(inFile, line) && line.find("</event>") == std::string::npos) {
                std::istringstream iss(line);
                int id, status;
                double px, py, pz, e;

                iss >> id >> status >> px >> py >> pz >> e;
                if (status == -1) {
                    incomingPartonIDs.push_back(id);
                }
                if (incomingPartonIDs.size() == 2) break;
            }

            // Determine production channel for this event
            int productionChannel = identifyProductionChannel(incomingPartonIDs);
            productionChannels.push_back(productionChannel);
        }
    }

    inFile.close();
    return productionChannels;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <LHE_file> <output_file>" << std::endl;
        return 1;
    }
    std::ifstream inFile(argv[1]);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open LHE file: " << argv[1] << std::endl;
        return 1;
    }

    std::ofstream outFile(argv[2]);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open outfile for writing: " << argv[2] << std::endl;
        return 1;
    }

    // Step 1: Parse all events to get production channels
    std::vector<int> productionChannels = parseLHEProductionChannels(argv[1]);
    if (productionChannels.empty()) {
        std::cerr << "Error: Could not determine production channels from LHE file." << std::endl;
        return 1;
    }

    // Initialize Pythia with MadGraph LHE file
    Pythia pythia;
    std::cout << std::string(argv[1]) << std::endl;
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");
    pythia.readString("Beams:frameType = 4");
    pythia.readString("Beams:LHEF = " + std::string(argv[1]));
    pythia.readString("Beams:eCM = 100.e3");
    pythia.readString("25:onMode = on");

    pythia.init();

    std::cout << "Checkpoint: Pythia initialized." << std::endl;

    // Anti-kt jet clustering with R = 0.4
    double R = 0.4;
    JetDefinition jet_def(antikt_algorithm, R);

    int nEvents = 25000;
    int totalHCount = 0;

    // Outfile headers
    outFile << "HiggsBoson, ProductionChannel, DecayProducts, InvMasses, pT, Rapidity, JetMultiplicity\n";

    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        int productionChannel = 0;

        for (int j = 0; j < pythia.event.size(); j++) {
            if ((pythia.event[j].id() == 25 || pythia.event[j].id() == 35 || pythia.event[j].id() == 36 || pythia.event[j].id() == 37) && pythia.event[j].status() == -62) {
                totalHCount++;

                productionChannel = productionChannels[i];

                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                    }
                }

                if (decayProducts.size() >= 2) {
                    double invMass = invariantMass(momenta);
                    double pT = pythia.event[j].pT();
                    double rapidity = pythia.event[j].y();

                    std::vector<PseudoJet> particles;
                    for (int k = 0; k < pythia.event.size(); k++) {
                        if (pythia.event[k].isFinal()) {
                            PseudoJet particle(pythia.event[k].px(), pythia.event[k].py(), pythia.event[k].pz(), pythia.event[k].e());
                            particle.set_user_index(k);
                            particles.push_back(particle);
                        }
                    }

                    // FastJet clustering
                    ClusterSequence clustSeq(particles, jet_def);
                    std::vector<PseudoJet> jets = clustSeq.inclusive_jets();
                    int jetMultiplicity = 0;
                    for (const auto& jet : jets) {
                        if (jet.pt() > 30.0) {
                            jetMultiplicity++;
                        }
                    }

                    // Output all data
                    outFile << pythia.event[j].id() << "," << productionChannel << ",";
                    for (size_t d = 0; d < decayProducts.size(); d++) {
                        outFile << decayProducts[d];
                        if (d < decayProducts.size() - 1) outFile << ";";
                    }
                    outFile << "," << invMass << "," << pT << "," << rapidity << "," << jetMultiplicity << "\n";
                }
            }
        }
    }

    // Finished
    outFile.close();
    std::cout << "Checkpoint: Output file closed, program completed." << std::endl;
    return 0;
}
