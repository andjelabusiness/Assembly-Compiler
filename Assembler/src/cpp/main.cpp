#include <fstream>
#include <iostream>
#include "../h/Asembler.h"

int main(int argc, const char* argv[]) {
    //losi argumenti
    if(argc != 4) {
        cout << "Pozovite program ovako: ./program -o izlazniFajl ulazniFajl" << "\n";
        return -1;
    }

    //fajlovi
    ofstream izlazniFajl(argv[2]);
    ifstream ulazniFajl(argv[3]);

    //greske kod otvaranja fajlova
    if(!ulazniFajl.is_open()) {
        if(izlazniFajl.is_open()) {
            izlazniFajl.close();
            cout << "Greska pri otvaranju ulaznog fajla." << "\n";
            return -2;
        }
        cout << "Greska pri otvaranju fajlova." << "\n";
        return -4;
    }
    else if(!izlazniFajl.is_open()) {
        if(ulazniFajl.is_open()) {
            izlazniFajl.close();
            cout << "Greska pri otvaranju izlaznog fajla." << "\n";
            return -3;
        }
        cout << "Greska pri otvaranju fajlova." << "\n";
        return -4;
    }

    //pokreni asm
    Asembler asem;
    asem.pokreni(&ulazniFajl);

    asem.upisi(&izlazniFajl);
    
    //kraj
    ulazniFajl.close();
    izlazniFajl.close();
    return 0;
}