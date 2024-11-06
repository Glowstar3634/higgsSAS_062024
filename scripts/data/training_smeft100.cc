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
                int id1 = pythia.event[pythia.event[j].mother1()].id();
                int id2 = pythia.event[pythia.event[j].mother2()].id();

                // Infer production channel from incoming partons
                if (abs(id1) == 21 && abs(id2) == 21) {
                    productionChannel = 1;  // gg -> H (gluon fusion)
                } else if (abs(id1) <= 6 && abs(id2) <= 6 && id1 * id2 < 0) {
                    if ((abs(id1) == 1 || abs(id1) == 2) && (abs(id2) == 1 || abs(id2) == 2)) {
                        productionChannel = 2;  // VBF
                    } else {
                        productionChannel = 3;  // VH
                    }
                } else if ((abs(id1) == 6 && abs(id2) == -6) || (abs(id1) == -6 && abs(id2) == 6)) {
                    productionChannel = 4;  // ttH production
                }

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

                    std::vector<Particle> particles;
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
