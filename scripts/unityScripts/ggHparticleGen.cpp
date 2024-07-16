#include <iostream>
#include <cmath>
#include <fstream>
#include "Pythia8/Pythia.h"

extern "C" __declspec(dllexport) void GenerateEvents() {
    void generateParticleData(const char* outputFilePath) {
        using namespace Pythia8;
        int nEvents = 25;
        Pythia pythia;

        pythia.readString("Random:setSeed = on");
        pythia.readString("Random:seed = " + std::to_string(time(0)));
        pythia.readString("Beams:idA = 2212");
        pythia.readString("Beams:idB = 2212");
        pythia.readString("Beams:eCM = 13.e3");
        pythia.readString("HardQCD:all = on");
        pythia.readString("HiggsSM:gg2H = on");
        
        pythia.init();

        std::ofstream outFile(outputFilePath);
        outFile << "event_id,particle_id,mass,px,py,pz,eta" << std::endl;

        for (int i = 0; i < nEvents; i++) {
            if (!pythia.next()) { continue; }

            int entries = pythia.event.size();

            for (int j = 0; j < entries; j++) {
                int id = pythia.event[j].id();
                if (id == 25 || id == 35 || id == 36 || id == 37) {
                    double mass = pythia.event[j].m();
                    double px = pythia.event[j].px();
                    double py = pythia.event[j].py();
                    double pz = pythia.event[j].pz();
                    double eta = pythia.event[j].eta();

                    outFile << i << "," << id << "," << mass << "," << px << "," << py << "," << pz << "," << eta << std::endl;
                }
            }
        }

        outFile.close();
    }
}