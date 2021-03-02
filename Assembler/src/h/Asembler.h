#ifndef ASEMBLER_H
#define ASEMBLER_H

#include <regex>
#include <list>
#include <fstream>
#include <unordered_map>
#include "PomStrukture.h"

using namespace std;

class Asembler {
    public:
        unsigned long brLinije = 1;
        unsigned long LC = 0;
        static unsigned brSimbola;
        static char* delimiters;
        Sekcija* tekSekcija = 0;
        //liste
        list<Simbol> tabelaSimbola;
        list<Sekcija> sekcije;
        list<EQURed> tabelaEQUSimbola;
        //mapa operacionih kodova
        unordered_map<string, int> opKodovi = {
            {"HALT", 0}, {"IRET", 1}, {"RET", 2}, {"INT", 3}, {"CALL", 4},
            {"JMP", 5}, {"JEQ", 6}, {"JNE", 7}, {"JGT", 8}, {"PUSH", 9},
            {"POP", 10}, {"XCHG", 11}, {"MOV", 12}, {"ADD", 13}, {"SUB", 14},
            {"MUL", 15}, {"DIV", 16}, {"CMP", 17}, {"NOT", 18}, {"AND", 19},
            {"OR", 20}, {"XOR", 21}, {"TEST", 22}, {"SHL", 23}, {"SHR", 24}
        };

        Asembler() {
            //inic listi
			tabelaSimbola = list<Simbol>();
			sekcije = list<Sekcija>();
			tabelaEQUSimbola = list<EQURed>();
            //inic fiksnih sekcija
            Sekcija undSekcija;
            Sekcija absSekcija;
            Simbol undSimbol;
            Simbol absSimbol;

            //UND
            undSekcija.ime = "UND";
            undSekcija.ID = 0;
            sekcije.push_back(undSekcija);

            undSimbol.ime = "UND";
            undSimbol.vrednost = 0;
            undSimbol.sekcija = &sekcije.front();
            undSimbol.globalan = false;
            absSimbol.eksterni = false;
            undSimbol.ID = brSimbola++;
            undSimbol.definisan = true;
            tabelaSimbola.push_back(undSimbol);

            //ABS
            absSekcija.ime = "ABS";
            absSekcija.ID = 1;
            sekcije.push_back(absSekcija);
            

            absSimbol.ime = "ABS";
            absSimbol.vrednost = 0;
            absSimbol.sekcija = &sekcije.back();
            absSimbol.globalan = false;
            absSimbol.eksterni = false;
            absSimbol.ID = brSimbola++;
            absSimbol.definisan = true;
            tabelaSimbola.push_back(absSimbol);
        };

        ~Asembler() {};

        void pokreni(ifstream* ulaz);
        void upisi(ofstream* izlaz);
        //EQUSimboli
        EQURed* dohvatiEQUSimbol(string ime);
        //simboli
        void dodajSimbol(Simbol simbol);
        Simbol* dohvatiSimbol(string ime);
        void azurirajSimbol(Simbol simbol);
};

#endif