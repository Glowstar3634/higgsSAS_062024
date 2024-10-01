#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include "Pythia8/Pythia.h"

using namespace Pythia8;

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

    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 30.e3");

    pythia.readString("HiggsSM:all  = on");
    pythia.readString("25:onMode = on");

    pythia.init();

    int totalHCount = 0;

    outFile << "ProductionChannel,DecayProducts,InvMasses\n";

    //Event loop
    for (int i = 0; i < nEvents; i++) {
        if (!pythia.next()) continue;

        //Analyze decays of detected Higgs particles only
        for (int j = 0; j < pythia.event.size(); j++) {
            std::vector<int> validStatuses = {-62}; //decayed (placed in array for future alteration)
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

                //Output decay products and their invariant masses
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
                    outFile << "\n";
                }
            }
        }
    }

    outFile.close();
    return 0;
}
