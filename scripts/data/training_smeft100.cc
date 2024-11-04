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


    // Initialize Pythia with MadGraph LHE file
    Pythia pythia;
    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");
    pythia.readString("Beams:frameType = 4");
    pythia.readString("Beams:LHEF = '/home/ubuntu/MG5_aMC_v3_6_0/SMEFT_run2/Events/run_01/unweighted_events.lhe'");
    pythia.readString("Beams:eCM = 100.e3");
    pythia.readString("HiggsSM:all  = off");
    pythia.readString("HiggsSM:gg2H = on"); // Enable gg -> H (ggH)
    pythia.readString("HiggsSM:ff2Hff(t:ZZ) = on"); // Enable VBF (VBF: quark initiated)
    pythia.readString("HiggsSM:ff2Hff(t:W+W-) = on"); // Enable VBF (quark initiated)
    pythia.readString("HiggsSM:ffbar2Hffbar(t:ZZ) = on"); // Enable VH production (associated with Z)
    pythia.readString("HiggsSM:ffbar2Hffbar(t:W+W-) = on"); // Enable VH production (associated with W)
    pythia.readString("HiggsSM:qqbar2Httbar = on"); // Enable ttH production
    pythia.readString("25:onMode = on");

    //SMEFT Parameters

    pythia.init();

    std::cout << "Checkpoint: Pythia initialized." << std::endl;

    // Anti-kt jet clustering with R = 0.4
    double R = 0.4;
    JetDefinition jet_def(antikt_algorithm, R);

    int nEvents = 10000;
    int totalHCount = 0;

    //Outfile headers
    outFile << "ProductionChannel,DecayProducts,InvMasses\n";

    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        // Identify Higgs production process in LHE-initiated event
        int id1 = pythia.info.id1();  // PDG code of incoming particle 1
        int id2 = pythia.info.id2();  // PDG code of incoming particle 2
        int productionChannel = 0;

        // Infer production channel from incoming partons
        if (abs(id1) == 21 && abs(id2) == 21) {
            productionChannel = 1;  // gg -> H (gluon fusion)
        } else if (abs(id1) <= 6 && abs(id2) <= 6 && id1 * id2 < 0) {
            // Check specific quark combinations for VBF (u, d, c, s)
            if ((abs(id1) == 1 || abs(id1) == 2) && (abs(id2) == 1 || abs(id2) == 2)) {
                productionChannel = 2;  // VBF
            } else {
                productionChannel = 3;  // VH (associated production)
            }
        } else if ((abs(id1) == 6 && abs(id2) == -6) || (abs(id1) == -6 && abs(id2) == 6)) {
            productionChannel = 4;  // tt -> H (ttH production)
        }

        for (int j = 0; j < pythia.event.size(); j++) {
            if (pythia.event[j].id() == 25 && pythia.event[j].status() == -62) {
                // Higgs decay logic
                totalHCount++;

                // Store decay products and their momenta
                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;
                std::map<int, int> particleIdToIndexMap;

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                        particleIdToIndexMap[pythia.event[k].id()] = k;
                    }
                }

                if (decayProducts.size() >= 2) {
                    outFile << productionChannel << ",";

                    for (size_t d = 0; d < decayProducts.size(); d++) {
                        outFile << decayProducts[d];
                        if (d < decayProducts.size() - 1) outFile << ";";
                    }
                    outFile << ",";

                    double invMass = invariantMass(momenta);
                    outFile << invMass;
                    outFile << "\n";
                }
            }
        }
    }


    //Finished
    outFile.close();
    std::cout << "Checkpoint: Output file closed, program completed." << std::endl;
    return 0;
}
