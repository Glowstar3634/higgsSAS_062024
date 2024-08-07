#include <iostream>
#include <fstream>
#include <cmath>
#include "Pythia8/Pythia.h"

using namespace Pythia8;

int main(int argc, char* argv[]){
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <output_file>" << std::endl;
        return 1;
    }

    std::ofstream outFile(argv[1]);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << argv[1] << std::endl;
        return 1;
    }

    int nEvents = 100;
    Pythia8::Pythia pythia;

    pythia.readString("Random:setSeed = on");
    pythia.readString("Random:seed = 0");

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 13.e3");

    pythia.readString("HardQCD:all = on");
    pythia.readString("HiggsSM:gg2H = on");

    pythia.init();

    for(int i = 0; i < nEvents; i++) {
        if(!pythia.next()) continue;

        int entries = pythia.event.size();
        outFile << "Event: " << i << "\n";
        outFile << "Event size: " << entries << "\n";

        for(int j = 0; j < entries; j++) {
            int id = pythia.event[j].id();
            double mass = pythia.event[j].m();
            double px = pythia.event[j].px();
            double py = pythia.event[j].py();
            double pz = pythia.event[j].pz();
            double pabs = sqrt(pow(px, 2) + pow(py, 2) + pow(pz, 2));
            double eta = pythia.event[j].eta();
            if (id == 25 || id == 35 || id == 36 || id == 37) {  // Look for Higgs bosons
                outFile << "Higgs Entry: " << id 
                        << " > Mass: " << mass 
                        << " > Momentum: " << pabs 
                        << " > Eta: " << eta 
                        << "\n";
            } else {
                outFile << "Entry: " << id 
                        << " > Mass: " << mass 
                        << " > Momentum: " << pabs 
                        << " > Eta: " << eta 
                        << "\n";
            }
        }
    }

    outFile.close();
    return 0;
}
