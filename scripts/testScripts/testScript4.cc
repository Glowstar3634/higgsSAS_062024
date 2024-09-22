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

    // Higgs production and decay settings (gg > H > 2p)
    pythia.readString("HiggsSM:gg2H = on"); 
    pythia.readString("25:onMode = off");
    pythia.readString("25:onIfMatch = 22 22");

    pythia.init();

    // Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        std::vector<std::vector<Particle>> higgsPhotonPairs; // Vector for storing pairs of photons for each Higgs

        for (int j = 0; j < pythia.event.size(); j++) {
            if (pythia.event[j].id() == 25) { 
                std::vector<Particle> photons;

                for (int k = 0; k < pythia.event.size(); k++) {
                    if (pythia.event[k].id() == 22 && pythia.event[k].mother1() == j) {
                        photons.push_back(pythia.event[k]);
                    }
                }

                if (photons.size() == 2) {
                    higgsPhotonPairs.push_back(photons); // Save this pair
                }
            }
        }

        //Invariant mass calculations
        for (const auto& pair : higgsPhotonPairs) {
            double E1 = pair[0].e();
            double px1 = pair[0].px();
            double py1 = pair[0].py();
            double pz1 = pair[0].pz();

            double E2 = pair[1].e();
            double px2 = pair[1].px();
            double py2 = pair[1].py();
            double pz2 = pair[1].pz();

            double E_tot = E1 + E2;
            double px_tot = px1 + px2;
            double py_tot = py1 + py2;
            double pz_tot = pz1 + pz2;
            double invariantMass = sqrt(E_tot * E_tot - (px_tot * px_tot + py_tot * py_tot + pz_tot * pz_tot));

            outFile << "Invariant Mass of Photon Product Pair: " << invariantMass << "\n";
        }
    }


    outFile.close();
    return 0;
}
