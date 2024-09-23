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

    // Enable Higgs production
    pythia.readString("HiggsSM:gg2H = on");

    // Allow the Higgs to decay into all possible modes
    pythia.readString("25:onMode = on");

    pythia.init();

    // Variables to count decay channels
    int hggCount = 0;  // Count for H → γγ
    int totalHCount = 0;

    // Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        // Analyze Higgs decays
        for (int j = 0; j < pythia.event.size(); j++) {
            if (pythia.event[j].id() == 25) {  // Check if particle is a Higgs boson
                totalHCount++;

                // Check the decay products of the Higgs
                std::vector<int> decayProducts;
                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].mother1() == j) {  // Check if the particle comes from this Higgs
                        decayProducts.push_back(pythia.event[k].id());
                    }
                }

                // Check for H → γγ channel
                if (decayProducts.size() == 2 && 
                    decayProducts[0] == 22 && 
                    decayProducts[1] == 22) {
                    hggCount++;
                }

                // Output decay products for analysis (optional)
                outFile << "Higgs Decay Products: ";
                for (int id : decayProducts) {
                    outFile << id << " ";
                }
                outFile << "\n";
            }
        }
    }

    // Output decay channel statistics
    outFile << "\nTotal Higgs Decays: " << totalHCount << "\n";
    outFile << "H → γγ Count: " << hggCount << "\n";
    outFile << "H → γγ Ratio: " << static_cast<double>(hggCount) / totalHCount << "\n";

    outFile.close();
    return 0;
}
