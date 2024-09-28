#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pythia8/Pythia.h"

using namespace Pythia8;

double invariantMass(Vec4 p1, Vec4 p2) {
    Vec4 total = p1 + p2;
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

    int nEvents = 100000;
    Pythia pythia;

    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13.e3");

    pythia.readString("HiggsSM:gg2H = on");
    pythia.readString("HiggsSM:qqbar2Hqq = on");
    pythia.readString("HiggsSM:qq2H = on");
    pythia.readString("HiggsSM:gg2Hg = on");
    pythia.readString("25:onMode = on");

    pythia.init();

    int totalHCount = 0;

    // Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        // Analyze Higgs decays
        for (int j = 0; j < pythia.event.size(); j++) {
            std::vector<int> validStatuses = {-62}; // Decayed Higgs
            if (pythia.event[j].id() == 25 && (std::find(validStatuses.begin(), validStatuses.end(), pythia.event[j].status()) != validStatuses.end())) {
                totalHCount++;

                std::vector<int> decayProducts;
                std::vector<Vec4> momenta;
                int productionChannel = pythia.info.code(); // Record the process ID

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j || pythia.event[k].mother2() == j) {  
                        decayProducts.push_back(pythia.event[k].id());
                        momenta.push_back(pythia.event[k].p());
                    }
                }

                // Output production channel
                outFile << "ProductionChannel: " << productionChannel << ",";

                // Output decay products and their invariant masses
                for (int i = 0; i < decayProducts.size(); i++) {
                    if (i == decayProducts.size() - 1) {
                        outFile << decayProducts[i] << "\n";
                    } else {
                        outFile << decayProducts[i] << ",";
                    }
                }

                // Calculate invariant masses for each pair of products
                for (size_t a = 0; a < momenta.size(); a++) {
                    for (size_t b = a + 1; b < momenta.size(); b++) {
                        double invMass = invariantMass(momenta[a], momenta[b]);
                        outFile << "InvMass_" << decayProducts[a] << "_" << decayProducts[b] << ": " << invMass << "\n";
                    }
                }
            }
        }
    }

    // Output decay channel statistics
    outFile << "\nTotalHiggsDecays: " << totalHCount << "\n";

    outFile.close();
    return 0;
}
