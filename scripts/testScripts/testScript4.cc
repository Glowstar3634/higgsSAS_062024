#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include "Pythia8/Pythia.h"

using namespace Pythia8;

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

    int nEvents = 1000;
    Pythia pythia;

    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13.e3");

    pythia.readString("HiggsSM:gg2H = on");
    pythia.readString("25:onMode = on");

    pythia.init();

    int hggCount = 0;
    int totalHCount = 0;

    // Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        // Analyze Higgs decays
        for (int j = 0; j < pythia.event.size(); j++) {
            // Check if particle is Higgs and has decayed
            if (pythia.event[j].id() == 25 && pythia.event[j].status() == -22) {
                totalHCount++;  // Count each Higgs that decays

                std::vector<int> decayProducts;
                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j) {  // Check if the particle comes from this Higgs
                        decayProducts.push_back(pythia.event[k].id());
                    }
                }

                // Check for H -> gamma gamma channel
                if (decayProducts.size() == 2 && 
                    decayProducts[0] == 22 && 
                    decayProducts[1] == 22) {
                    hggCount++;
                }

                // Output decay products for further analysis
                for (int id : decayProducts) {
                    outFile << id << " ";
                }
                outFile << "\n";
            }
        }
    }

    // Output decay channel statistics
    outFile << "\nTotalHiggsDecays: " << totalHCount << "\n";
    outFile << "H2PRatio: " << static_cast<double>(hggCount) / totalHCount << "\n";

    outFile.close();
    return 0;
}
