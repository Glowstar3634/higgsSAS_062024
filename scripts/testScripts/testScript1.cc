#include <iostream>
#include "Pythia8/Pythia.h"

using namespace Pythia8;

int main(){
    int nEvents = 10;
    Pythia8::Pythia pythia;

    pythia.readString("Beams:idA = 2212");
    pythia.readString("Beams:idB = 2212");
    pythia.readString("Beams:eCM = 14.e3");

    //Generates {nEvents} events
    for(int i = 0; i < nEvents; i++){

        if(!pythia.next()){continue;}
        
        int entries = pythia.event.size();
        std::cout << "Event: " << i << std::endl;
        std::cout << "Event size: " << entries << std::endl;

        //Collects data from each entry in the event
        for(int j = 0; j < entries; j++){
            int id = pythia.event[j].id();

            double mass = pythia.event[j].m();

            double px = pythia.event[j].px();
            double py = pythia.event[j].py();
            double pz = pythia.event[j].pz();
            double pabs = sqrt(pow(px,2) + pow(py,2) + pow(pz,2));\

            double eta = pythia.event[j].eta();
            double vx = pythia.event[j].

            std::cout << "Entry: " << id << " > " << m << " > " << pabs << std::endl;
        }
    }

    pythia.init();


    return 0;
}