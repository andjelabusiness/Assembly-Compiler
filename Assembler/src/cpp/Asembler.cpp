#include <iostream>
#include <cmath>
#include <cstring>
#include <algorithm>
#include "../h/Asembler.h"

unsigned Asembler::brSimbola = 0;
char* Asembler::delimiters = (char*)" \t\n,\f\r\v";

//Simboli
Simbol* Asembler::dohvatiSimbol(string s) {
    list<Simbol>::iterator i = tabelaSimbola.begin();
    while (i != tabelaSimbola.end()){
        if(!(*i).ime.compare(s.c_str())){
            return &(*i);
        }
        i++;
    }
    return nullptr;
}

void Asembler::azurirajSimbol(Simbol s) {
    //nadji index simbola
    bool nasao = false;
    list<Simbol>::iterator i = tabelaSimbola.begin();
    while (i!= tabelaSimbola.end()){
        if(!(*i).ime.compare(s.ime.c_str())){
            nasao = true;
            break;
        }
        i++;
    }
    //nije nadjen simbol nekako
    if(!nasao) {
        cout << "Greska: nije nadjen simbol u funkciji azurirajSimbol" << "\n";
        exit(-1);
    }
    //
    (*i).vrednost = s.vrednost;
    //(*i).sekcija = tekSekcija;
    (*i).sekcija = s.sekcija;
    //vec bio jednom def
    if((*i).definisan) {
        cout << "Greska: ne mozete dva puta definisati isti simbol" << "\n";
        exit(-1);
    }
    else (*i).definisan = true;
}

void Asembler::dodajSimbol(Simbol s) {
    //sort
    if ( s.sekcija && s.ime == s.sekcija->ime){  //ako je sekcija, dodati ga na mesto sekcija
        list<Simbol>::iterator i = tabelaSimbola.begin();
        bool nasao= false;
        while (i!= tabelaSimbola.end()){
          if(!(*i).sekcija || (*i).sekcija && (*i).ime.compare((*i).sekcija->ime)){
                tabelaSimbola.insert(i,s); 
                nasao = true;
                break;
          }
          i++;
        }
        if(!nasao) tabelaSimbola.push_back(s);
    } 
    else tabelaSimbola.push_back(s); //ako je simbol samo stavi na kraj
}

//EQUSimboli
EQURed* Asembler::dohvatiEQUSimbol(string ime){
    list<EQURed>::iterator i = tabelaEQUSimbola.begin();
    while (i!= tabelaEQUSimbola.end()){
        if(!(*i).imeSimbola.compare(ime.c_str())){
            return &(*i);
        }
        i++;
    } 
       
    return nullptr;  
}

void Asembler::pokreni(ifstream* ulaz){

    string linija;
    string pomLinija;
    char* token;
    while(getline(*ulaz, linija)) {
        //nova linija
        pomLinija = linija;
        //token prvi
        token = strtok((char*)linija.c_str(), delimiters);
        
        if(!token) {
            brLinije++;
            continue;
        } 
        //direktive
        if(regex_match(pomLinija.c_str(), regex("^(\\s*)(\\w+)[:](((\\s+)(.*)(\\s*))|(\\s*))$")) ) { //labela
            if(tekSekcija == 0) { 
                cout << "Labela ne sme biti definisana van sekcije! Linija: " << to_string(brLinije) << "\n";
                exit(-1); 
            }
            Simbol s;
            s.vrednost = LC;
            //ime
            string pom = string(token);
            pom.pop_back();
            s.ime = pom;
            s.sekcija = tekSekcija; // sekcija simbola
            //ifovi
            if(dohvatiEQUSimbol(s.ime)) {
                cout << "Simbol vec definisan sa EQU! Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }
            if(dohvatiSimbol(s.ime) && dohvatiSimbol(s.ime)->definisan == false  && dohvatiSimbol(s.ime)->eksterni == false ){
                azurirajSimbol(s);
            }
            else if(dohvatiSimbol(s.ime) && dohvatiSimbol(s.ime)->eksterni == true) {
                cout << "Eksterni simbol ne moze biti definisan! Linija: " << to_string(brLinije) << "\n";
                exit(-1); 
            }
            else if(dohvatiSimbol(s.ime) && dohvatiSimbol(s.ime)->definisan == true) {
                cout << "Ne moze isti simbol biti vise puta definisan! Linija: " << to_string(brLinije) << "\n";
                exit(-1); 
            }
            else {
                s.ID = brSimbola++;//ID
                s.globalan = false;// globalan simb ce vec biti u tabeli, tako da ako ovog nema onda je lokalan
                s.definisan = true;//def
                dodajSimbol(s);
            }
            token = strtok(0, delimiters);

            if(!token) {
                brLinije++;
                continue;//ako je sama labela u liniji
            }
            pomLinija = pomLinija.substr(pomLinija.find_first_of(':') + 1);
        }

        if(!string(token).compare(".end")) break;//end
        else if(regex_match(pomLinija.c_str(), regex("(\\s*)\\.section(\\s+)(\\w+)(\\s*)"))){ //sekcija
            token = strtok(0, " \t");
            if(dohvatiSimbol(token) || dohvatiEQUSimbol(token)) {
                cout << "Simbol vec postoji! Linija: " << to_string(brLinije) << "\n";
                exit(-1); 
            }
            //sim
            Simbol sim;
            sim.ime = token;
            sim.ID = brSimbola++;
            sim.vrednost = 0;
            sim.definisan = true;
            sim.globalan = false;
            sim.eksterni = false;
            //sek
            Sekcija sek;
            sek.ime = token;
            sek.ID = sim.ID;
            //dodavanje sek
            sekcije.push_back(sek);
            //postavi sek simbolu
            sim.sekcija = &sekcije.back();
            //tekSekcija
            if(tekSekcija) tekSekcija->velicina = LC;
            tekSekcija = &sekcije.back();
            LC = 0;
            //dodaj simbol
            dodajSimbol(sim);
            brLinije++;
            continue;
        }

        else if(regex_match(pomLinija.c_str(), regex("(\\s*)\\.extern(\\s+)(\\w+)(((\\s*)[,](\\s*)(\\w+)(\\s*))*)(\\s*)"))){ //extern done
            for(token = strtok(0, delimiters); token != NULL; token = strtok(0, delimiters)) {
                if(dohvatiSimbol(token)) {
                    cout << "Simbol " <<  token << " vec u tabeli simbola! Linija: " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                if(dohvatiEQUSimbol(token)) {
                    cout << "Simbol " <<  token << " vec definisan sa EQU! Linija: " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                //ubacivanje simbola
                Simbol s;
                s.ime = token;
                s.globalan = true;
                s.eksterni = true;
                s.ID = brSimbola++;
                s.definisan = false;
                s.sekcija = &sekcije.front();
                s.vrednost = 0;
                dodajSimbol(s);
            }
            brLinije++;

            continue;
        }

        else if(regex_match(pomLinija.c_str(), regex("(\\s*)\\.global(\\s+)(\\w+)(((\\s*)[,](\\s*)(\\w+)(\\s*))*)(\\s*)"))) { // global
            for(token = strtok(0, delimiters); token != NULL; token = strtok(0, delimiters)) {
                if(dohvatiSimbol(token)) {
                    cout << "Simbol " <<  token << " vec u tabeli simbola! Linija: " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                if(dohvatiEQUSimbol(token)) {
                    cout << "Simbol " <<  token << " vec definisan sa EQU! Linija: " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                //ubacivanje simbola
                Simbol s;
                s.ime = token;
                s.globalan = true;
                s.eksterni = false;
                s.ID = brSimbola++;
                s.definisan = false;
                s.sekcija = 0;
                s.vrednost = 0;
                dodajSimbol(s);
            }
            brLinije++;

            continue;
        }

        else if(regex_match(pomLinija.c_str(), regex("^(\\s*)\\.equ(\\s+)(\\w+)(\\s*)[,](\\s*)[\\+\\-]{0,1}(\\s*)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))((\\s*)[\\+\\-](\\s*)((\\w+)|((\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+)))(\\s*))*$"))) {//equ 
            token = strtok(0, delimiters);
            //da li je vec def simbol
            if((dohvatiSimbol(token) && dohvatiSimbol(token)->definisan) || (dohvatiSimbol(token) && dohvatiSimbol(token)->eksterni) || dohvatiEQUSimbol(token)) {
                cout << "EQU simbol " <<  token << " vec definisan! Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }
            //
            EQURed red;
            red.imeSimbola = token;
            // + ili -
            token = strtok(0, delimiters);
            //dole ide ovo novo
            string pomocniString = pomLinija.substr(pomLinija.find_first_of(',') + 1);
            istringstream ss;//mislim da mora stream jer on sa >> jede bele znakove a mozemo da ih imamo kolko oces
            ss.str(pomocniString);
            char pomChar;
            string pomToken;
            //
            ss >> pomChar;
            //
            bool pozitivan = true;// ovo je bilo
            //skini + ili minus i podesi znak
            if(pomChar == '-') {
                ss.str(ss.str().substr(ss.str().find_first_of('-') + 1));
                pozitivan = false;
            }
            else if(pomChar == '+') {
                ss.str(ss.str().substr(ss.str().find_first_of('+') + 1));
            }
            //
            pomocniString = ss.str();
            if(pomocniString.find_first_of('-') < pomocniString.find_first_of('+') ) {
                pomToken = pomocniString.substr(0, pomocniString.find_first_of('-'));
                pomocniString = pomocniString.substr(pomocniString.find_first_of('-'));
                ss.str(pomocniString);
            }
            else if ( pomocniString.find_first_of('+') < pomocniString.find_first_of('-')) {
                pomToken = pomocniString.substr(0, pomocniString.find_first_of('+'));
                pomocniString = pomocniString.substr(pomocniString.find_first_of('+'));
                ss.str(pomocniString);
            }
            else {//nema ni - ni +
                pomToken = pomocniString;
                pomocniString = "";
                ss.str("");
            }
            //nakon ovog gore je pomToken prvi token iz izraza
            pomToken = pomToken.substr(pomToken.find_first_not_of(' '));// skini whitespaceova sa pocetka
            if(pomToken.find_first_of(' ') != string::npos )pomToken = pomToken.substr(0, pomToken.find_first_of(' '));//skini whitespaceova sa kraja
            //ovde se stavlja token na bas taj pomToken
            //strcpy(token,pomToken.c_str());
            token = (char*)pomToken.c_str();
            //dalje ide
            //
            while(1) {//obradi simbole iz izraza
                //novi simbol iz equ izraza
                EQUZapis zap;
                if(regex_match(token, regex("((0[bB][01]+)|(\\d+)|(0[xX][0-9a-fA-F]+))"))) {
                    long literal;
                    if(string(token).length() > 1 && (*(token + 1) == 'b' || *(token + 1) == 'B')) {
                        literal = strtol(token + 2, NULL, 2);
                    }   
                    else if(string(token).length() > 1 && (*(token + 1) == 'x' || *(token + 1) == 'X')) {
                        literal = strtol(token + 2, NULL, 16);
                    }
                    else {
                        literal = atol(token);
                    }
                    //podesi zapis
                    zap.vrednost = ((pozitivan) ? literal : -literal);
                    zap.imeSimbola = "";
                    //dohvati abs sekciju
                    list<Sekcija>::iterator i = sekcije.begin();
                    i++;
                    zap.sekcija = &(*i);
                }
                else if(dohvatiSimbol(token)) {
                    //ime
                    zap.imeSimbola = token;
                    //ostalo
                    if(dohvatiSimbol(token)->eksterni) {//eksterni
                        zap.sekcija = &sekcije.front();//UND je prva
                        zap.vrednost = 1;
                    }
                    else if(dohvatiSimbol(token)->definisan) {//def
                        zap.sekcija = dohvatiSimbol(token)->sekcija;
                        zap.vrednost = ((pozitivan) ? 1 : -1);
                    }
                    else {//nije def
                        zap.vrednost = ((pozitivan) ? 1 : -1);
                        zap.sekcija = 0;
                    }
                }
                else {//forwardRef za neki lokalan simbol iz equ izraza
                    zap.vrednost = ((pozitivan) ? 1 : -1);
                    zap.sekcija = 0;
                    zap.imeSimbola = token;
                    //dodaj taj simbol u tabelu
                    Simbol sim;
                    sim.definisan = false;
                    sim.eksterni = false;
                    sim.globalan = false;
                    sim.ID = brSimbola++;
                    sim.ime = token;
                    sim.sekcija = 0;
                    sim.vrednost = 0;
                    dodajSimbol(sim);
                }
                //dodaj i uzmi sledeci
                red.tabela.push_back(zap);
                token = strtok(0, delimiters);
                pozitivan = true;
                //
                if(pomocniString.empty()) break;//kraj
                ss >> pomChar;
                //ovo je sad onaj sto je bio sledeci
                if(pomChar == '-') {
                    ss.str(ss.str().substr(ss.str().find_first_of('-') + 1));
                    pozitivan = false;
                }
                else if(pomChar == '+') {
                    ss.str(ss.str().substr(ss.str().find_first_of('+') + 1));
                }
                //
                pomocniString = ss.str();
                if(pomocniString.find_first_of('-') < pomocniString.find_first_of('+') ) {
                    pomToken = pomocniString.substr(0, pomocniString.find_first_of('-'));
                    pomocniString = pomocniString.substr(pomocniString.find_first_of('-'));
                    ss.str(pomocniString);
                }
                else if ( pomocniString.find_first_of('+') < pomocniString.find_first_of('-')) {
                    pomToken = pomocniString.substr(0, pomocniString.find_first_of('+'));
                    pomocniString = pomocniString.substr(pomocniString.find_first_of('+'));
                    ss.str(pomocniString);
                }
                else {//nema ni - ni +
                    pomToken = pomocniString;
                    pomocniString = "";
                    ss.str("");
                }
                //nakon ovog gore je pomToken prvi token iz izraza
                pomToken = pomToken.substr(pomToken.find_first_not_of(' '));// skini whitespaceova sa pocetka
                if(pomToken.find_first_of(' ') != string::npos )pomToken = pomToken.substr(0, pomToken.find_first_of(' '));//skini whitespaceova sa kraja
                //ovde se stavlja nas token na bas taj pomToken
                token = (char*)pomToken.c_str();
            }
            //inicijalizuj vektor pomocni
            vector<pair<string, long>> EQUProvera = vector<pair<string, long>>();
            //za literale
            long sumaLiterala = 0;
            for(list<EQUZapis>::iterator it = red.tabela.begin(); it != red.tabela.end(); it++){
                //ako je literal
                if((*it).sekcija && !(*it).sekcija->ime.compare("ABS")) {
                    if(!(*it).imeSimbola.compare("")) sumaLiterala += (*it).vrednost;
                    else sumaLiterala += ( ((*it).vrednost == 1 ) ? dohvatiSimbol((*it).imeSimbola)->vrednost : -(dohvatiSimbol((*it).imeSimbola)->vrednost));
                    continue;
                }
                //ako je nedef simbol
                if((*it).sekcija == 0) {
                    tabelaEQUSimbola.push_back(red);
                    break;
                }
                //samo dodaj u EQUProveru
                bool nasao = false;
                //ako jeste, samo saberi sa ovom vrednoscu
                for(int i = 0; i < EQUProvera.size(); i++) {
                    if(EQUProvera[i].first.compare((*it).sekcija->ime.c_str())) {
                        nasao = true;
                        EQUProvera[i] = make_pair(EQUProvera[i].first, EQUProvera[i].second + (*it).vrednost);
                        break;
                    }
                }
                //ako nije u EQUProvera, push back
                if(!nasao) EQUProvera.push_back(make_pair((*it).sekcija->ime, (*it).vrednost));
            }
            //ako nismo ubacili u tab simbola, onda moze da se izracuna
            if(tabelaEQUSimbola.size() == 0 || tabelaEQUSimbola.back().imeSimbola.compare(red.imeSimbola.c_str())) {
                Sekcija* sek = 0;
                for(int i = 0; i < EQUProvera.size(); i++) {
                    //ako naidjemo na keca validno je samo ako jos nismo pokupili nijednog keca
                    if(EQUProvera[i].second == 1 && !sek) {
                        list<Sekcija>::iterator it = sekcije.begin();
                        while (it!= sekcije.end()){
                            if(!(*it).ime.compare(EQUProvera[i].first.c_str())){
                                sek = &(*it);
                            }
                            it++;
                        } 
                    }
                    else if(EQUProvera[i].second == 0) i++;
                    else {
                        cout << "Nevalidan EQU izraz za simbol: " << red.imeSimbola.c_str() << ". Linija " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                }
                Simbol sim;
                sim.definisan = true;
                sim.eksterni = false;
                sim.globalan = false;
                sim.ime = red.imeSimbola;
                if(!sek) {
                    list<Sekcija>::iterator i = sekcije.begin();
                    i++;
                    sim.sekcija = &(*i);
                }
                else sim.sekcija = sek;
                sim.vrednost = sumaLiterala;
                //vrednost
                for(list<EQUZapis>::iterator it = red.tabela.begin(); it!= red.tabela.end(); it++){
                    //samo ako je razlicito od literala
                    if((*it).sekcija->ime.compare("ABS")) sim.vrednost += ( ((*it).vrednost == 1 ) ? dohvatiSimbol((*it).imeSimbola)->vrednost : -(dohvatiSimbol((*it).imeSimbola)->vrednost));
                }
                //ako je ovo definicija za neki simbol koji vec postoji kao nedefinisan u tabeli onda azuriraj
                if(dohvatiSimbol(sim.ime)) {
                    azurirajSimbol(sim);
                }
                else {
                    sim.ID = brSimbola++;
                    dodajSimbol(sim);
                }
            }
            brLinije++;
            //
            continue;
        }

        else if(regex_match(pomLinija.c_str(), regex("^(\\s*)\\.byte(\\s+)((\\w+)|([0-9]+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(((\\s*)[,](\\s*)((\\w+)|([0-9]]+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(\\s*))*)(\\s*)$"))){ // byte
            if(!tekSekcija) {
                cout << "Nije dozvoljeno pisanje direktiva/instrukcija van sekcija. Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }
            //prvi token, obradi tokene
            token = strtok(0, delimiters);
            while(token) {
                if (regex_match(token, regex("^[0-9]+$"))) { //decimalni
                    long tmp = atoi(token);
                    //da li moze da stane u bajt
                    if(tmp > 255) {
                        cout << "Byte operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //dodamo u content
                    stringstream ss;
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    if(pom.length() == 1 ) pom = "0" + pom;
                    tekSekcija->sadrzaj += " " + pom;
                }
                else if(string(token).length() > 2 && (*(token + 1) == 'x' || *(token + 1) == 'X')) { //hexa
                    //preskocimo 0x
                    token+=2;;
                    //iz hex u decim, i onda iz decim opet u hex da bi skinuo sve 0 sa pocetka
                    stringstream ss;
                    long tmp = strtol(token, NULL, 16);
                    //da li moze da stane u bajt
                    if(tmp > 255) {
                        cout << "Byte operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    //dodaj sadrzaj
                    if(pom.length() == 1 ) pom = "0" + pom;
                    tekSekcija->sadrzaj += " " + pom;
                }
                else if(string(token).length() > 2 && (*(token + 1) == 'b' || *(token + 1) == 'B')) { //binarni
                    //preskocimo 0x
                    token+=2;;
                    //iz bin u decim, i onda iz decim u hex, isto se skinu sve 0 sa pocetka
                    stringstream ss;
                    long tmp = strtol(token, NULL, 2);
                    //da li moze da stane u bajt
                    if(tmp > 255) {
                        cout << "Byte operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    //dodaj sadrzaj
                    if(pom.length() == 1 ) pom = "0" + pom;
                    tekSekcija->sadrzaj += " " + pom;
                }
                else { //simbol
                    //napravi zapis
                    RelokZapis rZapis;
                    rZapis.tip = "apsolutan";
                    rZapis.sekcija = tekSekcija;
                    rZapis.adresa = LC;
                    if(dohvatiSimbol(string(token))){//sim je u tabeli
                        //dohv simbol
                        Simbol* sim = dohvatiSimbol(string(token));
                        rZapis.IDSimbola = sim->ID;
                        //
                        if(!sim->globalan){
                            //ako je lokalan relZapis ima ID te sekcije
                            //mora if jer moze da se desi da jos nije def simbol, a nalazi se u tabeli
                            if(sim->sekcija) rZapis.IDSimbola = sim->sekcija->ID;
                            else rZapis.IDSimbola = sim->ID;
                            if(sim->definisan) {//def simbol
                                //da li moze da stane u bajt, vr simbola ne moze biti neg
                                if(sim->vrednost > 255) {
                                    cout << "Byte operand je preveliki. Linija: " << to_string(brLinije) << "\n";
                                    exit(-1);
                                }
                                //upisi vrednost
                                stringstream ss;
                                ss << hex << sim->vrednost;
                                string pom = ss.str();
                                if(pom.length() == 1 ) pom = "0" + pom;
                                tekSekcija->sadrzaj += " " + pom;
                            }
                            else {//nije def ali je u tabeli
                                //napravi fRef zapis
                                ObrUnapredZapis oZapis;
                                oZapis.adresa = LC;
                                oZapis.sekcija = tekSekcija;
                                oZapis.velicina = 1;
                                sim->tabelaObrUnpared.push_back(oZapis);
                                //upisi vrednost
                                tekSekcija->sadrzaj += " 00";
                            }
                        }
                        else tekSekcija->sadrzaj += " 00";//globalan
                    } 
                    else {//sim nije u tabeli
                        //napravi simbol
                        Simbol sim;
                        sim.definisan = false;
                        sim.eksterni = false;
                        sim.globalan = false;
                        sim.ID = brSimbola++;
                        sim.ime = string(token);
                        sim.sekcija = 0;
                        sim.vrednost = 0;
                        //napravi relZapis
                        //fejk ID
                        rZapis.IDSimbola = sim.ID;
                        //napravi fRef zapis                  
                        ObrUnapredZapis oZapis;
                        oZapis.adresa = LC;
                        oZapis.sekcija = tekSekcija;
                        oZapis.velicina = 1;
                        //ubaci zapis obr unapred
                        sim.tabelaObrUnpared.push_back(oZapis);
                        //ubaci simbol
                        dodajSimbol(sim);
                        //sadrzaj
                        tekSekcija->sadrzaj += " 00";
                    }
                    //stavi rZapis u rTabelu
                    tekSekcija->relTabela.push_back(rZapis);
                }
                LC++;
                token = strtok(0, delimiters);
            }
            brLinije++;
            //
            continue;
        }

        else if(regex_match(pomLinija.c_str(), regex("(\\s*)\\.word(\\s+)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(((\\s*)[,](\\s*)((\\w+)|(\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(\\s*))*)(\\s*)"))) {
            if(!tekSekcija) {
                cout << "Nije dozvoljeno pisanje direktiva/instrukcija van sekcija. Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }
            //prvi token, obradi tokene
            token = strtok(0, delimiters);
            while(token) {
                if (regex_match(token, regex("^[0-9]+$"))) { //decimalni
                    long tmp = atoi(token);
                    //da li moze da stane u bajt
                    if(tmp > 65535) {
                        cout << "Byte operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //dodamo u content
                    stringstream ss;
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    //
                    switch (pom.length()){
                        case 1:
                            pom = " 0" + pom + " 00";
                            break;
                        case 2:
                            pom = " " + pom + " 00";
                            break;
                        case 3:
                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                            break;
                        case 4:
                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                            break;
                        default:
                            break;
                    }
                    tekSekcija->sadrzaj += pom;
                }
                else if(string(token).length() > 2 && (*(token + 1) == 'x' || *(token + 1) == 'X')) { //hexa
                    //preskocimo 0x
                    token+=2;;
                    //iz hex u decim, i onda iz decim opet u hex da bi skinuo sve 0 sa pocetka
                    stringstream ss;
                    long tmp = strtol(token, NULL, 16);
                    //da li moze da stane u bajt
                    if(tmp > 65535) {
                        cout << "Byte operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    //dodaj sadrzaj
                    //
                    switch (pom.length()){
                        case 1:
                            pom = " 0" + pom + " 00";
                            break;
                        case 2:
                            pom = " " + pom + " 00";
                            break;
                        case 3:
                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                            break;
                        case 4:
                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                            break;
                        default:
                            break;
                    }
                    tekSekcija->sadrzaj += pom;
                }
                else if(string(token).length() > 2 && (*(token + 1) == 'b' || *(token + 1) == 'B')) { //binarni
                    //preskocimo 0x
                    token+=2;;
                    //iz bin u decim, i onda iz decim u hex, isto se skinu sve 0 sa pocetka
                    stringstream ss;
                    long tmp = strtol(token, NULL, 2);
                    //da li moze da stane u bajt
                    if(tmp > 65535) {
                        cout << "Word operand je preveliki.Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    ss << uppercase << hex << tmp;
                    string pom = ss.str();
                    //dodaj sadrzaj
                    //
                    switch (pom.length()){
                        case 1:
                            pom = " 0" + pom + " 00";
                            break;
                        case 2:
                            pom = " " + pom + " 00";
                            break;
                        case 3:
                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                            break;
                        case 4:
                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                            break;
                        default:
                            break;
                    }
                    tekSekcija->sadrzaj += pom;
                }
                else { //simbol
                    //napravi zapis
                    RelokZapis rZapis;
                    rZapis.tip = "apsolutan";
                    rZapis.sekcija = tekSekcija;
                    rZapis.adresa = LC;
                    if(dohvatiSimbol(string(token))){//sim je u tabeli
                        //dohv simbol
                        Simbol* sim = dohvatiSimbol(string(token));
                        rZapis.IDSimbola = sim->ID;
                        //
                        if(!sim->globalan){
                            //ako je lokalan relZapis ima ID te sekcije
                            //mora if jer moze da se desi da jos nije def simbol, a nalazi se u tabeli
                            if(sim->sekcija) rZapis.IDSimbola = sim->sekcija->ID;
                            else rZapis.IDSimbola = sim->ID;
                            if(sim->definisan) {//def simbol
                                //da li moze da stane u bajt, vr simbola ne moze biti neg
                                if(sim->vrednost > 65535) {
                                    cout << "Byte operand je preveliki. Linija: " << to_string(brLinije) << "\n";
                                    exit(-1);
                                }
                                //upisi vrednost
                                stringstream ss;
                                ss << hex << sim->vrednost;
                                string pom = ss.str();
                                //
                                switch (pom.length()){
                                    case 1:
                                        pom = " 0" + pom + " 00";
                                        break;
                                    case 2:
                                        pom = " " + pom + " 00";
                                        break;
                                    case 3:
                                        pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                        break;
                                    case 4:
                                        pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                        break;
                                    default:
                                        break;
                                }
                                tekSekcija->sadrzaj += pom;
                            }
                            else {//nije def ali je u tabeli
                                //napravi fRef zapis
                                ObrUnapredZapis oZapis;
                                oZapis.adresa = LC;
                                oZapis.sekcija = tekSekcija;
                                oZapis.velicina = 2;
                                sim->tabelaObrUnpared.push_back(oZapis);
                                //upisi vrednost
                                tekSekcija->sadrzaj += " 00 00";
                            }
                        }
                        else tekSekcija->sadrzaj += " 00 00";//globalan
                    } 
                    else {//sim nije u tabeli
                        //napravi simbol
                        Simbol sim;
                        sim.definisan = false;
                        sim.eksterni = false;
                        sim.globalan = false;
                        sim.ID = brSimbola++;
                        sim.ime = string(token);
                        sim.sekcija = 0;
                        sim.vrednost = 0;
                        //napravi relZapis
                        //fejk ID
                        rZapis.IDSimbola = sim.ID;
                        //napravi fRef zapis                  
                        ObrUnapredZapis oZapis;
                        oZapis.adresa = LC;
                        oZapis.sekcija = tekSekcija;
                        oZapis.velicina = 2;
                        //ubaci zapis obr unapred
                        sim.tabelaObrUnpared.push_back(oZapis);
                        //ubaci simbol
                        dodajSimbol(sim);
                        //sadrzaj
                        tekSekcija->sadrzaj += " 00 00";
                    }
                    //stavi rZapis u rTabelu
                    tekSekcija->relTabela.push_back(rZapis);
                }
                LC += 2;
                token = strtok(0, delimiters);
            }
            brLinije++;
            //
            continue;
        }
        
        else if(regex_match(pomLinija.c_str(), regex("(\\s*)\\.skip(\\s+)((\\d+)|(0[bB][0-1]+)|(0[xX][a-fA-F0-9]+))(\\s*)"))) {
            if(!tekSekcija) {
                cout << "Nije dozvoljeno pisanje direktiva/instrukcija van sekcija. Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }
            //literal token
            token = strtok(0, delimiters);
            long tmp = 0;
            //
            if (regex_match(token, regex("^[0-9]+$"))) { //decimalni
                tmp = atoi(token);
            }
            else if(string(token).length() > 2 && (*(token + 1) == 'x' || *(token + 1) == 'X')) { //hexa
                //preskocimo 0x
                token+=2;;
                tmp = strtol(token, NULL, 16);
            }
            else if(string(token).length() > 2 && (*(token + 1) == 'b' || *(token + 1) == 'B')) { //binarni
                //preskocimo 0b
                token+=2;
                tmp = strtol(token, NULL, 2);
            }
            //
            for(long i = 0; i < tmp; i++) tekSekcija->sadrzaj += " 00";
            //
            LC += tmp;
            brLinije++;
            //
            continue;
        }

        //
        stringstream ss;
        string str(token);
        //velika slova za instrukciju
        for(int i = 0; i < str.length(); i++) str[i] = toupper(str[i]);
        //velicina op
        bool bajt = false;
        if(((str[str.length() - 1] == 'B' && str.compare("SUB")) || ((str[str.length() - 1] == 'B' && str[str.length() - 2] == 'B' && !str.compare("SUB"))))) {
            bajt = true;
            str.pop_back();
        }
        else if(str[str.length() - 1] == 'W') {
            str.pop_back();
        }
        //
        if(opKodovi.find(str) != opKodovi.end()) {//neka instrukcija
            //greska
            if(!tekSekcija) {
                cout << "Nije dozvoljeno pisanje direktiva/instrukcija van sekcija. Linija: " << to_string(brLinije) << "\n";
                exit(-1);
            }

            int brojOp = 2;
            //dest operand
            int brojOdredisnogOp = 2;
            if((*opKodovi.find(str)).second == 24) brojOdredisnogOp = 1;

            //preskoci instrukciju
            pomLinija = pomLinija.substr(pomLinija.find_first_not_of(' '));
            if(pomLinija.find_first_of(' ') != string::npos) pomLinija = pomLinija.substr(pomLinija.find_first_of(' ') + 1);
            else pomLinija = "";
            //
            unsigned long pom = (*opKodovi.find(str)).second * 8;//shiftujemo u levo za 3 bita, pa puta 2^3
            if(!bajt) pom += 4;// S bit
            //pretvori u hex
            ss.str("");//praznjenje stringstrima
            ss << uppercase << hex << pom; 
            //dodavanje InstrDescr bajta
            if(ss.str().length() == 1) tekSekcija->sadrzaj += " 0" + ss.str();
            else tekSekcija->sadrzaj += " " + ss.str();
            //locCounter se uveca jer smo upisali instrDescr bajt
            LC++;
            //ako je halt(0) ili iret(1) ili ret(2), onda je to to
            if((*opKodovi.find(str)).second < 3) {
                if(!regex_match(pomLinija.c_str(), regex("(\\s*)"))) {//ako ima jos nesto pored halt/iret/ret onda greska
                    cout << "Nepoznata instrukcija \"" + str + "\"  na liniji " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                brLinije++;
                continue;
            }
            //da li je jedan op
            if((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 11) brojOp = 1;
            
            //greske za format instrukcije
            if(brojOp == 1 && !regex_match(pomLinija.c_str(), regex("(\\s*)[\\*\\%\\(\\)\\$\\w]+(\\s*)"))) {
                cout << "Nepoznata direktiva/instrukcija na liniji " << to_string(brLinije) << "\n";
                exit(-1);
            }
            else if(brojOp == 2 && !regex_match(pomLinija.c_str(), regex("(\\s*)[\\*\\%\\(\\)\\$\\w]+(\\s*)[,](\\s*)[\\*\\%\\(\\)\\$\\w]+(\\s*)"))) {
                cout << "Nepoznata direktiva/instrukcija \"" + pomLinija + "\"  na liniji " << to_string(brLinije) << "\n";
                exit(-1);
            }

            //prodji dva operanda
            for( int brojOpKojiSeObradjuje = 1; brojOpKojiSeObradjuje <= 2; brojOpKojiSeObradjuje++) {
                //op1/2
                token = strtok(0, delimiters);
                //blok za op
                if( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) &&  regex_match(token, regex("((\\d+)|(0[bB][01]+)|(0[xX][0-9a-fA-F]+))")))  || (((*opKodovi.find(str)).second > 8)  &&  regex_match(token, regex("\\$((\\d+)|(0[bB][01]+)|(0[xX][0-9a-fA-F]+))")))) {//literal neposredna vrednost
                    if( (brojOdredisnogOp == 1 && brojOpKojiSeObradjuje == 1) || (brojOdredisnogOp == 2 && brojOpKojiSeObradjuje == 2) || ((*opKodovi.find(str)).second == 11) || (*opKodovi.find(str)).second == 10 ) {
                        cout << "Ne moze odredisni operand da bude neposredna vrednost. Linija: " << to_string(brLinije) << "\n";
                        exit(-1); 
                    }
                    //skini $ ako ima
                    if(*token == '$') token++;
                    //op1Descr
                    tekSekcija->sadrzaj += " 00";//AM2AM1AM0 = 0 R3-0 = 0 L/H = 0
                    //obradi literal
                    if (regex_match(token, regex("^[0-9]+$"))) { //decimalni
                        long tmp = atoi(token);
                        if(tmp > 255 && bajt || tmp > 65535 && !bajt) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        //dodamo u sadrzaj
                        ss.str("");
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        if(bajt) {
                            if(pom.length() == 1 ) pom = "0" + pom;
                            tekSekcija->sadrzaj += " " + pom;
                        }
                        else {
                            switch (pom.length()){
                                case 1:
                                    pom = " 0" + pom + " 00";
                                    break;
                                case 2:
                                    pom = " " + pom + " 00";
                                    break;
                                case 3:
                                    pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                    break;
                                case 4:
                                    pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                    break;
                                default:
                                    break;
                            }
                            tekSekcija->sadrzaj += pom;
                        }
                    }
                    else if(string(token).length() > 2 && (*(token + 1) == 'x' || *(token + 1) == 'X')) { //hexa
                        //preskocimo 0x
                        token+=2;;
                        //iz hex u decim, i onda iz decim opet u hex da bi skinuo sve 0 sa pocetka
                        ss.str("");
                        long tmp = strtol(token, NULL, 16);
                        //da li moze da stane u bajt
                        if(tmp > 255 && bajt || tmp > 65535 && !bajt) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //dodaj sadrzaj
                        if(bajt) {
                            if(pom.length() == 1 ) pom = "0" + pom;
                            tekSekcija->sadrzaj += " " + pom;
                        }
                        else {
                            switch (pom.length()){
                                case 1:
                                    pom = " 0" + pom + " 00";
                                    break;
                                case 2:
                                    pom = " " + pom + " 00";
                                    break;
                                case 3:
                                    pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                    break;
                                case 4:
                                    pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                    break;
                                default:
                                    break;
                            }
                            tekSekcija->sadrzaj += pom;
                        }
                    }
                    else if(string(token).length() > 2 && (*(token + 1) == 'b' || *(token + 1) == 'B')) { //binarni
                        //preskocimo 0x
                        token+=2;;
                        //iz bin u decim, i onda iz decim u hex, isto se skinu sve 0 sa pocetka
                        ss.str("");
                        long tmp = strtol(token, NULL, 2);
                        //da li moze da stane u bajt
                        if(tmp > 255 && bajt || tmp > 65535 && !bajt) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //dodaj sadrzaj
                        if(bajt) {
                            if(pom.length() == 1 ) pom = "0" + pom;
                            tekSekcija->sadrzaj += " " + pom;
                        }
                        else {
                            switch (pom.length()){
                                case 1:
                                    pom = " 0" + pom + " 00";
                                    break;
                                case 2:
                                    pom = " " + pom + " 00";
                                    break;
                                case 3:
                                    pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                    break;
                                case 4:
                                    pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                    break;
                                default:
                                    break;
                            }
                            tekSekcija->sadrzaj += pom;
                        }
                    }
                    LC += 2;
                    if(!bajt) LC++;
                }
                else if( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) &&  regex_match(token, regex("(\\w+)")))  || (((*opKodovi.find(str)).second > 8)  &&  regex_match(token, regex("\\$(\\w+)")))) {//simbol neposredna vrednost
                    if(( brojOdredisnogOp == 1 && brojOpKojiSeObradjuje == 1) || (brojOdredisnogOp == 2 && brojOpKojiSeObradjuje == 2) || ((*opKodovi.find(str)).second == 11) || (*opKodovi.find(str)).second == 10) {
                        cout << "Ne moze odredisni operand da bude neposredna vrednost. Linija: " << to_string(brLinije) << "\n";
                        exit(-1); 
                    }
                    if(*token == '$') token++;//skini $ ako ima
                    //op1Descr
                    tekSekcija->sadrzaj += " 00";//AM2AM1AM0 = 0 R3-0 = 0 L/H = 0
                    LC++;
                    //napravi zapis
                    RelokZapis rZapis;
                    rZapis.tip = "apsolutan";
                    rZapis.sekcija = tekSekcija;
                    rZapis.adresa = LC;
                    if(dohvatiSimbol(token)){//sim je u tabeli
                        //dohv simbol
                        Simbol* sim = dohvatiSimbol(token);
                        rZapis.IDSimbola = sim->ID;
                        //
                        if(!sim->globalan){
                            //ako je lokalan relZapis ima ID te sekcije
                            //mora if jer moze da se desi da jos nije def simbol, a nalazi se u tabeli
                            if(sim->sekcija) rZapis.IDSimbola = sim->sekcija->ID;
                            else rZapis.IDSimbola = sim->ID;
                            if(sim->definisan) {//def simbol
                                //da li moze da stane u bajt, vr simbola ne moze biti neg
                                if(sim->vrednost > 255 && bajt || sim->vrednost > 65535 && !bajt) {
                                    cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                                    exit(-1);
                                }
                                //upisi vrednost
                                ss.str("");
                                ss << hex << sim->vrednost;
                                string pom = ss.str();
                                if(bajt) {
                                    if(pom.length() == 1 ) pom = " 0" + pom;
                                    tekSekcija->sadrzaj += " " + pom;
                                }
                                else {
                                    switch (pom.length()){
                                        case 1:
                                            pom = " 0" + pom + " 00";
                                            break;
                                        case 2:
                                            pom = " " + pom + " 00";
                                            break;
                                        case 3:
                                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                            break;
                                        case 4:
                                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                            break;
                                        default:
                                            break;
                                    }
                                    tekSekcija->sadrzaj += pom;
                                }
                            }
                            else {//nije def ali je u tabeli
                                //napravi fRef zapis
                                ObrUnapredZapis oZapis;
                                oZapis.adresa = LC;
                                oZapis.sekcija = tekSekcija;
                                oZapis.velicina = 1;
                                sim->tabelaObrUnpared.push_back(oZapis);
                                //upisi vrednost
                                tekSekcija->sadrzaj += " 00";
                                if(!bajt) tekSekcija->sadrzaj += " 00";
                            }
                        }
                        else {//globalan
                            tekSekcija->sadrzaj += " 00";
                            if(!bajt) tekSekcija->sadrzaj += " 00";
                        }
                    } 
                    else {//sim nije u tabeli
                        //napravi simbol
                        Simbol sim;
                        sim.definisan = false;
                        sim.eksterni = false;
                        sim.globalan = false;
                        sim.ID = brSimbola++;
                        sim.ime = string(token);//*bila pomLinija valjda je to greska
                        sim.sekcija = 0;
                        sim.vrednost = 0;
                        //napravi relZapis
                        //fejk ID
                        rZapis.IDSimbola = sim.ID;
                        //napravi fRef zapis                  
                        ObrUnapredZapis oZapis;
                        oZapis.adresa = LC;
                        oZapis.sekcija = tekSekcija;
                        if(bajt) oZapis.velicina = 1;
                        else oZapis.velicina = 2;
                        //ubaci zapis obr unapred
                        sim.tabelaObrUnpared.push_back(oZapis);
                        //ubaci simbol
                        dodajSimbol(sim);
                        //sadrzaj
                        tekSekcija->sadrzaj += " 00";
                        if(!bajt) tekSekcija->sadrzaj += " 00";
                    }
                    //stavi rZapis u rTabelu
                    tekSekcija->relTabela.push_back(rZapis);
                    //                
                    LC++;
                    if(!bajt) LC++;
                }
                else if( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) &&  regex_match(token, regex("\\*\\%r[0-7][lhLH]{0,1}"))) || (((*opKodovi.find(str)).second > 8 &&  regex_match(token, regex("\\%r[0-7][lhLH]{0,1}"))))) {//regDir
                    //skini * i % ili samo %
                    if(*token == '*') token += 2;
                    else token++;;
                    //
                    long tmp = 32;//AM2AM1AM0 = 1
                    tmp += 2 * atol(token + 1);//izvuci redni br registra
                    string tokenStr(token);
                    if(bajt) {
                        if(tokenStr[tokenStr.length() - 1] != 'l' && tokenStr[tokenStr.length() - 1] != 'L' && tokenStr[tokenStr.length() - 1] != 'h' && tokenStr[tokenStr.length() - 1] != 'H') {
                            cout << "Kad je velicina instrukcije bajt, potrebno je navesti kojih 8 bita registra se koristi sa: h\\l. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        if(tokenStr[tokenStr.length() - 1] == 'h' || tokenStr[tokenStr.length() - 1] == 'H') tmp += 1;//h bit setovan
                    }
                    else if (tokenStr[tokenStr.length() - 1] == 'l' || tokenStr[tokenStr.length() - 1] == 'L' || tokenStr[tokenStr.length() - 1] == 'h' || tokenStr[tokenStr.length() - 1] == 'H') {
                        cout << "Kad je velicina instrukcije rec, nije validno navoditi kojih 8 bita registra se koriste sa: h\\l. Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //pretvori op1Descr u hex string
                    ss.str("");
                    ss << uppercase << hex << tmp;
                    //dodaj sadrzaj u sekciju
                    tekSekcija->sadrzaj += " " + ss.str();
                    //
                    LC++;
                }
                else if ( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) &&  regex_match(token, regex("\\*\\(\\%r[0-7]\\)"))) || (((*opKodovi.find(str)).second > 8 &&  regex_match(token, regex("\\(\\%r[0-7]\\)")))) ) {//regIndir
                    //skini * i (% ili samo (%
                    if(*token == '*') token += 3;
                    else token += 2;
                    //greska kod reg ind
                    if(*(token + 2) == 'l' || *(token + 2) == 'h') {
                        cout << "Za registarsko indirektno ne moze se navoditi kojih 8 bita registra se koriste sa \"l\\h\". Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //skini zagradu poslednju
                    string tokenStr(token);
                    tokenStr.pop_back();
                    long tmp = 64;//AM2AM1AM0 = 2
                    tmp += 2 * atol(&tokenStr[1]);//izvuci redni br registra
                    //pretvori op1Descr u hex string
                    ss.str("");
                    ss << uppercase << hex << tmp;
                    //dodaj sadrzaj u sekciju
                    tekSekcija->sadrzaj += " " + ss.str();
                    //
                    LC++;
                }
                else if( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) && regex_match(token, regex("\\*((\\d+)|(0[bB][0]*[01]{1,16})|(0[xX][0]*[a-fA-F0-9]{1,4}))\\(\\%r[0-7]\\)"))) || (((*opKodovi.find(str)).second > 8 &&  regex_match(token, regex("((\\d+)|(0[bB][0]*[01]{1,16})|(0[xX][0]*[a-fA-F0-9]{1,4}))\\(\\%r[0-7]\\)")))) ) {//regInDir sa pom
                    //skini *
                    if(*token == '*') token++;
                    string tokenStr(token);
                    token += tokenStr.find_first_of('(') + 2;//postavi token na 'r<num>)'
                    tokenStr = tokenStr.substr(0, tokenStr.find_first_of('('));//izdvaja literal iz izraza
                    //greska kod reg ind
                    if(*(token + 2) == 'l' || *(token + 2) == 'h') {
                        cout << "Za registarsko indirektno ne moze se navoditi kojih 8 bita registra se koriste sa \"l\\h\". Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //op1Descr
                    long op1Descr = 96;//AM2AM1AM0 = 3
                    op1Descr += 2 * atol(token + 1);//izvuci redni br registra
                    //pretvori op1Descr u hex string
                    ss.str("");
                    ss << uppercase << hex << op1Descr;
                    //dodaj sadrzaj u sekciju
                    tekSekcija->sadrzaj += " " + ss.str();
                    //obradi literal
                    if (regex_match(tokenStr.c_str(), regex("^[0-9]+$"))) { //decimalni
                        long tmp = atoi(tokenStr.c_str());
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        //dodamo u content
                        ss.str("");
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        switch (pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    else if(tokenStr.length() > 2 && (tokenStr[1] == 'x' || tokenStr[1] == 'X')) { //hexa
                        //preskocimo 0x
                        tokenStr = tokenStr.substr(2);
                        //iz hex u decim, i onda iz decim opet u hex da bi skinuo sve 0 sa pocetka
                        long tmp = strtol(tokenStr.c_str(), NULL, 16);
                        //da li moze da stane u bajt
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss.str("");
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //add content
                        switch (pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    else if(tokenStr.length() > 2 && (tokenStr[1] == 'b' || tokenStr[1] == 'B')) { //binarni
                        //preskocimo 0b
                        tokenStr = tokenStr.substr(2);
                        //iz bin u decim, i onda iz decim u hex, isto se skinu sve 0 sa pocetka
                        ss.str("");
                        long tmp = strtol(tokenStr.c_str(), NULL, 2);
                        //da li moze da stane u bajt
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //dodaj sadrzaj
                        switch (pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    //
                    LC += 3;
                }
                else if( (((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) && regex_match(token, regex("\\*(\\w+)\\(((\\%r[0-7])|(\\%pc))\\)"))) || (((*opKodovi.find(str)).second > 8 &&  regex_match(token, regex("(\\w+)\\(((\\%r[0-7])|(\\%pc))\\)")))) ) {//regInDir sa pom,simbol
                    //skini *
                    if(*token == '*') token++;
                    string tokenStr(token);
                    token += tokenStr.find_first_of('(') + 2;//postavi token na 'r<num>)'
                    tokenStr = tokenStr.substr(0, tokenStr.find_first_of('('));//izdvaja simbol iz izraza
                    //greska kod reg ind
                    if(*(token + 2) == 'l' || *(token + 2) == 'h') {
                        cout << "Za registarsko indirektno ne moze se navoditi kojih 8 bita registra se koriste sa \"l\\h\". Linija: " << to_string(brLinije) << "\n";
                        exit(-1);
                    }
                    //op1Descr
                    //*******spojila sam pcrel i drugi registri rel
                    bool pcRel = false;
                    if(*(token + 1) == '7' || *token == 'p') {
                        pcRel = true;
                    }
                    long op1Descr = 96;//AM2AM1AM0 = 3
                    if(pcRel) op1Descr += 2 * 7;//izvuci redni br registra
                    else op1Descr += 2 * atol(token + 1);
                    //pretvori op1Descr u hex string
                    ss.str("");
                    ss << uppercase << hex << op1Descr;
                    //dodaj sadrzaj u sekciju
                    tekSekcija->sadrzaj += " " + ss.str();
                    LC++;
                    //velicina pomeraja pom u izrazu za pcRel: vrSimbola - * - pom
                    long pom;
                    if(brojOp == 1 || brojOpKojiSeObradjuje == 2) {
                        pom = -2;
                    }
                    else {
                        string drugiOp = pomLinija.substr(pomLinija.find_first_of(',') + 1);
                        if(bajt) {
                            //ako je regDir ili regIndir onda -3B
                            if(regex_match(drugiOp.c_str(), regex("(\\s*)\\%r[0-7][lhLH]{0,1}(\\s*)")) || regex_match(drugiOp.c_str(), regex("(\\s*)\\(\\%r[0-7]\\)(\\s*)"))) {
                                pom = -3;
                            }
                            else if(regex_match(drugiOp.c_str(), regex("(\\s*)[\\$]{0,1}((\\d+)|(0[bB][01]+)|(0[xX][0-9a-fA-F]+))(\\s*)")) || regex_match(drugiOp.c_str(), regex("(\\s*)[\\$]{0,1}(\\w+))(\\s*)")) ) { // neposredno ili memorijsko
                                pom = -4;
                            }
                            else pom = -5;
                        }
                        else {
                            //ako je regDir ili regIndir onda -3B
                            if(regex_match(drugiOp.c_str(), regex("(\\s*)\\%r[0-7][lhLH]{0,1}(\\s*)")) || regex_match(drugiOp.c_str(), regex("(\\s*)\\(\\%r[0-7]\\)(\\s*)"))) {
                                pom = -3;
                            }
                            else pom = -5;
                        }
                    }
                    //obradi simbol
                    if(pcRel) {
                        //
                        RelokZapis rZapis;
                        rZapis.adresa = LC;
                        rZapis.sekcija = tekSekcija;
                        rZapis.tip = "relokatibilan";
                        if(dohvatiSimbol(tokenStr)) {
                            Simbol* sim = dohvatiSimbol(tokenStr);
                            rZapis.IDSimbola = sim->ID;
                            //
                            if(sim->definisan) {
                                if(sim->sekcija->ID == tekSekcija->ID) {
                                    //da li je def u istoj sekciji kao i tekSekcija, ako jeste i ako je pcrel onda nema rel zapisa
                                    long vrednost = sim->vrednost;
                                    vrednost += pom - LC;
                                    //upisi vrednost u sadrzaj
                                    ss.str("");
                                    ss << uppercase << hex << vrednost;//ffffe
                                    string stringVrednost = ss.str();
                                    //skini sve f-ove sa pocetka ako je neg vrednost
                                    if(stringVrednost.length() > 4) stringVrednost = stringVrednost.substr(stringVrednost.length() - 4);
                                    //ubaci vrednost u sadrzaj
                                    switch (stringVrednost.length()){
                                        case 1:
                                            stringVrednost = " 0" + stringVrednost + " 00";
                                            break;
                                        case 2:
                                            stringVrednost = " " + stringVrednost + " 00";
                                            break;
                                        case 3:
                                            stringVrednost = " " + stringVrednost.substr(1,1) + stringVrednost.substr(2,1) + " 0" + stringVrednost.substr(0,1);
                                            break;
                                        case 4:
                                            stringVrednost = ' ' + stringVrednost.substr(2,1) + stringVrednost.substr(3,1) + ' ' + stringVrednost.substr(0,1) + stringVrednost.substr(1,1);
                                            break;
                                        default:
                                            break;
                                    }
                                    tekSekcija->sadrzaj += stringVrednost;
                                }
                                else {
                                    //vrednost uvek
                                    long vrednost = 0;
                                    if(!sim->globalan) {
                                        rZapis.IDSimbola = sim->sekcija->ID;
                                        //zameni vr simbola jer je lokalan, u onaj izraz
                                        vrednost = sim->vrednost;
                                    }
                                    vrednost += pom;
                                    long vrednost2 = sim->vrednost + pom;
                                    //upisi vrednost u sadrzaj
                                    ss.str("");
                                    ss << uppercase << hex << vrednost;//
                                    string stringVrednost = ss.str();
                                    //
                                    if(stringVrednost.length() > 4) stringVrednost = stringVrednost.substr(stringVrednost.length() - 4);
                                    //ubaci vrednost u sadrzaj
                                    switch (stringVrednost.length()) {
                                        case 1:
                                            stringVrednost = " 0" + stringVrednost + " 00";
                                            break;
                                        case 2:
                                            stringVrednost = " " + stringVrednost + " 00";
                                            break;
                                        case 3:
                                            stringVrednost = " " + stringVrednost.substr(1,1) + stringVrednost.substr(2,1) + " 0" + stringVrednost.substr(0,1);
                                            break;
                                        case 4:
                                            stringVrednost = ' ' + stringVrednost.substr(2,1) + stringVrednost.substr(3,1) + ' ' + stringVrednost.substr(0,1) + stringVrednost.substr(1,1);
                                            break;
                                        default:
                                            break;
                                    }
                                    tekSekcija->sadrzaj += stringVrednost;
                                    //uvek dodaj rZapis
                                    tekSekcija->relTabela.push_back(rZapis);
                                }
                            }
                            else {
                                //ide rel zapis i fRef zapis
                                ObrUnapredZapis oZapis; //treba uvek oZapis jer moze da se desi da simbol bude def u istoj sekciji u kojoj je ovaj rZapis, pa iako je globalan bice potrebno da se njegova vrednost upise u mesto koriscenja
                                oZapis.adresa = LC;
                                oZapis.sekcija = tekSekcija;
                                oZapis.velicina = 2;
                                //
                                long vrednost = pom;
                                //ubaci sadrzaj
                                ss.str("");
                                ss << uppercase << hex << vrednost;
                                string stringVrednost = ss.str();
                                //
                                if(stringVrednost.length() > 4) stringVrednost = stringVrednost.substr(stringVrednost.length() - 4);
                                //ubaci vrednost u sadrzaj
                                switch (stringVrednost.length()) {
                                    case 1:
                                        stringVrednost = " 0" + stringVrednost + " 00";
                                        break;
                                    case 2:
                                        stringVrednost = " " + stringVrednost + " 00";
                                        break;
                                    case 3:
                                        stringVrednost = " " + stringVrednost.substr(1,1) + stringVrednost.substr(2,1) + " 0" + stringVrednost.substr(0,1);
                                        break;
                                    case 4:
                                        stringVrednost = ' ' + stringVrednost.substr(2,1) + stringVrednost.substr(3,1) + ' ' + stringVrednost.substr(0,1) + stringVrednost.substr(1,1);
                                        break;
                                    default:
                                        break;
                                }
                                tekSekcija->sadrzaj += stringVrednost;
                                //uvek dodaj rZapis
                                tekSekcija->relTabela.push_back(rZapis);
                                sim->tabelaObrUnpared.push_back(oZapis);
                            }
                        }
                        else {
                            //napravi sim i ubaci ga u tabelu simbola
                            Simbol noviSimbol;
                            noviSimbol.definisan = false;
                            noviSimbol.eksterni = false;
                            noviSimbol.globalan = false;
                            noviSimbol.ID = brSimbola++;
                            noviSimbol.ime = tokenStr;
                            noviSimbol.sekcija = 0;
                            noviSimbol.vrednost = 0;
                            //rel zapis i fRef zapis
                            ObrUnapredZapis oZapis; //treba uvek oZapis jer moze da se desi da simbol bude def u istoj sekciji u kojoj je ovaj rZapis, pa iako je globalan bice potrebno da se njegova vrednost upise u mesto koriscenja
                            oZapis.adresa = LC;
                            oZapis.sekcija = tekSekcija;
                            oZapis.velicina = 2;
                            //
                            long vrednost = pom;
                            //ubaci sadrzaj
                            ss.str("");
                            ss << uppercase << hex << vrednost;
                            string stringVrednost = ss.str();
                            //
                            if(stringVrednost.length() > 4) stringVrednost = stringVrednost.substr(stringVrednost.length() - 4);
                            //ubaci vrednost u sadrzaj
                            switch (stringVrednost.length()) {
                                case 1:
                                    stringVrednost = " 0" + stringVrednost + " 00";
                                    break;
                                case 2:
                                    stringVrednost = " " + stringVrednost + " 00";
                                    break;
                                case 3:
                                    stringVrednost = " " + stringVrednost.substr(1,1) + stringVrednost.substr(2,1) + " 0" + stringVrednost.substr(0,1);
                                    break;
                                case 4:
                                    stringVrednost = ' ' + stringVrednost.substr(2,1) + stringVrednost.substr(3,1) + ' ' + stringVrednost.substr(0,1) + stringVrednost.substr(1,1);
                                    break;
                                default:
                                    break;
                            }
                            tekSekcija->sadrzaj += stringVrednost;
                            //dodaj simbol i zapise
                            tekSekcija->relTabela.push_back(rZapis);
                            noviSimbol.tabelaObrUnpared.push_back(oZapis);
                            tabelaSimbola.push_back(noviSimbol);
                        }
                    }
                    else {
                        //napravi zapis
                        RelokZapis rZapis;
                        rZapis.tip = "apsolutan";
                        rZapis.sekcija = tekSekcija;
                        rZapis.adresa = LC;
                        if(dohvatiSimbol(tokenStr)){//sim je u tabeli
                            //dohv simbol
                            Simbol* sim = dohvatiSimbol(tokenStr);
                            rZapis.IDSimbola = sim->ID;
                            //
                            if(!sim->globalan){
                                //ako je lokalan relZapis ima ID te sekcije
                                //mora if jer moze da se desi da jos nije def simbol, a nalazi se u tabeli
                                if(sim->sekcija) rZapis.IDSimbola = sim->sekcija->ID;
                                else rZapis.IDSimbola = sim->ID;
                                if(sim->definisan) {//def simbol
                                    //da li moze da stane u bajt, vr simbola ne moze biti neg
                                    if(sim->vrednost > 65535) {
                                        cout << "Byte operand je preveliki. Linija: " << to_string(brLinije) << "\n";
                                        exit(-1);
                                    }
                                    //upisi vrednost
                                    stringstream ss;
                                    ss << hex << sim->vrednost;
                                    string pom = ss.str();
                                    //
                                    switch (pom.length()){
                                        case 1:
                                            pom = " 0" + pom + " 00";
                                            break;
                                        case 2:
                                            pom = " " + pom + " 00";
                                            break;
                                        case 3:
                                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                            break;
                                        case 4:
                                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                            break;
                                        default:
                                            break;
                                    }
                                    tekSekcija->sadrzaj += pom;
                                }
                                else {//nije def ali je u tabeli
                                    //napravi fRef zapis
                                    ObrUnapredZapis oZapis;
                                    oZapis.adresa = LC;
                                    oZapis.sekcija = tekSekcija;
                                    oZapis.velicina = 2;
                                    sim->tabelaObrUnpared.push_back(oZapis);
                                    //upisi vrednost
                                    tekSekcija->sadrzaj += " 00 00";
                                }
                            }
                            else tekSekcija->sadrzaj += " 00 00";//globalan
                        } 
                        else {//sim nije u tabeli
                            //napravi simbol
                            Simbol sim;
                            sim.definisan = false;
                            sim.eksterni = false;
                            sim.globalan = false;
                            sim.ID = brSimbola++;
                            sim.ime = tokenStr;
                            sim.sekcija = 0;
                            sim.vrednost = 0;
                            //napravi relZapis
                            //fejk ID
                            rZapis.IDSimbola = sim.ID;
                            //napravi fRef zapis                  
                            ObrUnapredZapis oZapis;
                            oZapis.adresa = LC;
                            oZapis.sekcija = tekSekcija;
                            oZapis.velicina = 2;
                            //ubaci zapis obr unapred
                            sim.tabelaObrUnpared.push_back(oZapis);
                            //ubaci simbol
                            dodajSimbol(sim);
                            //sadrzaj
                            tekSekcija->sadrzaj += " 00 00";
                        }
                        //stavi rZapis u rTabelu
                        tekSekcija->relTabela.push_back(rZapis);
                    }
                    //
                    LC += 2;
                }
                else if((((*opKodovi.find(str)).second > 2 && (*opKodovi.find(str)).second < 9) && regex_match(token, regex("\\*(\\w+)"))) || (((*opKodovi.find(str)).second > 8 &&  regex_match(token, regex("(\\w+)"))))) {
                    //op1Descr
                    if(*token == '*') token++;// za skokove
                    tekSekcija->sadrzaj += " 80";
                    LC++;
                    //obradi literal
                    if (regex_match(token, regex("^[0-9]+$"))) { //decimalni
                        long tmp = atoi(token);
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        //dodamo u sadrzaj
                        ss.str("");
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        switch (pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    else if(string(token).length() > 2 && (*(token + 1) == 'x' || *(token + 1) == 'X')) { //hexa
                        //preskocimo 0x
                        token+=2;;
                        //iz hex u decim, i onda iz decim opet u hex da bi skinuo sve 0 sa pocetka
                        ss.str("");
                        long tmp = strtol(token, NULL, 16);
                        //da li moze da stane u bajt
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //add content
                        switch (pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    else if(string(token).length() > 2 && (*(token + 1) == 'b' || *(token + 1) == 'B')) { //binarni
                        //preskocimo 0x
                        token+=2;;
                        //iz bin u decim, i onda iz decim u hex, isto se skinu sve 0 sa pocetka
                        ss.str("");
                        long tmp = strtol(token, NULL, 2);
                        //da li moze da stane u bajt
                        if(tmp > 65535) {
                            cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                            exit(-1);
                        }
                        ss << uppercase << hex << tmp;
                        string pom = ss.str();
                        //dodaj sadrzaj
                        switch(pom.length()){
                            case 1:
                                pom = " 0" + pom + " 00";
                                break;
                            case 2:
                                pom = " " + pom + " 00";
                                break;
                            case 3:
                                pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                break;
                            case 4:
                                pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                break;
                            default:
                                break;
                        }
                        tekSekcija->sadrzaj += pom;
                    }
                    else {//simbol
                        RelokZapis rZapis;
                        rZapis.tip = "apsolutan";
                        rZapis.sekcija = tekSekcija;
                        rZapis.adresa = LC;
                        if(dohvatiSimbol(token)){//sim je u tabeli
                            //dohv simbol
                            Simbol* sim = dohvatiSimbol(token);
                            rZapis.IDSimbola = sim->ID;
                            //
                            if(!sim->globalan){
                                //ako je lokalan relZapis ima ID te sekcije
                                //mora if jer moze da se desi da jos nije def simbol, a nalazi se u tabeli
                                if(sim->sekcija) rZapis.IDSimbola = sim->sekcija->ID;
                                else rZapis.IDSimbola = sim->ID;
                                if(sim->definisan) {//def simbol
                                    //da li moze da stane u bajt, vr simbola ne moze biti neg
                                    if(sim->vrednost > 255 && bajt || sim->vrednost > 65535 && !bajt) {
                                        cout << "Stvarna velicina operanda je prevelika za velicinu operanda instrukcije. Linija: " << to_string(brLinije) << "\n";
                                        exit(-1);
                                    }
                                    //upisi vrednost
                                    ss.str("");
                                    ss << hex << sim->vrednost;
                                    string pom = ss.str();
                                    switch (pom.length()){
                                        case 1:
                                            pom = " 0" + pom + " 00";
                                            break;
                                        case 2:
                                            pom = " " + pom + " 00";
                                            break;
                                        case 3:
                                            pom = " " + pom.substr(1,1) + pom.substr(2,1) + " 0" + pom.substr(0,1);
                                            break;
                                        case 4:
                                            pom = ' ' + pom.substr(2,1) + pom.substr(3,1) + ' ' + pom.substr(0,1) + pom.substr(1,1);
                                            break;
                                        default:
                                            break;
                                    }
                                    tekSekcija->sadrzaj += pom;
                                }
                                else {//nije def ali je u tabeli
                                    //napravi fRef zapis
                                    ObrUnapredZapis oZapis;
                                    oZapis.adresa = LC;
                                    oZapis.sekcija = tekSekcija;
                                    oZapis.velicina = 2;
                                    sim->tabelaObrUnpared.push_back(oZapis);
                                    //upisi vrednost
                                    tekSekcija->sadrzaj += " 00 00";
                                }
                            }
                            else {//globalan
                                tekSekcija->sadrzaj += " 00 00";
                            }
                        } 
                        else {//sim nije u tabeli
                            //napravi simbol
                            Simbol sim;
                            sim.definisan = false;
                            sim.eksterni = false;
                            sim.globalan = false;
                            sim.ID = brSimbola++;
                            sim.ime = string(token);
                            sim.sekcija = 0;
                            sim.vrednost = 0;
                            //napravi relZapis
                            //fejk ID
                            rZapis.IDSimbola = sim.ID;
                            //napravi fRef zapis                  
                            ObrUnapredZapis oZapis;
                            oZapis.adresa = LC;
                            oZapis.sekcija = tekSekcija;
                            oZapis.velicina = 2;
                            //ubaci zapis obr unapred
                            sim.tabelaObrUnpared.push_back(oZapis);
                            //ubaci simbol
                            dodajSimbol(sim);
                            //sadrzaj
                            tekSekcija->sadrzaj += " 00 00";
                        }
                        //
                        tekSekcija->relTabela.push_back(rZapis);
                    } 
                    //
                    LC += 2;
                }
                else {
                    cout << "Nepoznata direktiva/instrukcija na liniji " << to_string(brLinije) << "\n";
                    exit(-1);
                }
                //
                if(brojOp == 1) break;
            }
            brLinije++;
            continue;
        }
        else {//greska, nepoznata direktiva/instrukcija
            cout << "Nepoznata direktiva/instrukcija \"" + str + "\" na liniji " << to_string(brLinije) << "\n";
            exit(-1);
        }
    }   
    //prolaz kroz tabelu equ simbola 
    for(list<EQURed>::iterator it = tabelaEQUSimbola.begin(); it != tabelaEQUSimbola.end(); it++) {
        
        bool preskociSimbol = false;
        //popuni nedefinisanim simbolima sekcije
        for(list<EQUZapis>::iterator it2 = (*it).tabela.begin(); it2 != (*it).tabela.end(); it2++){
            if(!(*it2).sekcija) {//ako nema sek postavi je
                if(!dohvatiSimbol((*it2).imeSimbola) || !dohvatiSimbol((*it2).imeSimbola)->definisan ) {
                  
                    preskociSimbol = true;
                }
                (*it2).sekcija = dohvatiSimbol((*it2).imeSimbola)->sekcija;
            }
        }
        if(preskociSimbol) continue;
        //inicijalizuj vektor pomocni
        vector<pair<string, long>> EQUProvera = vector<pair<string, long>>();
        //za literale
        long sumaLiterala = 0;
        //popuni pomocni vektor
        for(list<EQUZapis>::iterator it2 = (*it).tabela.begin(); it2 != (*it).tabela.end(); it2++){
            //ako je literal
            if((*it2).sekcija && !(*it2).sekcija->ime.compare("ABS")) {
                if(!(*it2).imeSimbola.compare("")) sumaLiterala += (*it2).vrednost;
                else sumaLiterala += ( ((*it2).vrednost == 1 ) ? dohvatiSimbol((*it2).imeSimbola)->vrednost : -(dohvatiSimbol((*it2).imeSimbola)->vrednost));
                continue;
            }
            //samo dodaj u EQUProveru
            bool nasao = false;
            //ako jeste, samo saberi sa ovom vrednoscu
            for(int i = 0; i < EQUProvera.size(); i++) {
                if(!EQUProvera[i].first.compare((*it2).sekcija->ime.c_str())) {
                    nasao = true;
                    EQUProvera[i] = make_pair(EQUProvera[i].first, EQUProvera[i].second + (*it2).vrednost);
                    break;
                }
            }
            //ako nije u EQUProvera, push back
            if(!nasao) {
                EQUProvera.push_back(make_pair((*it2).sekcija->ime, (*it2).vrednost));
            }
        }
        //vidi je l validan i izracunaj vrednost
        Sekcija* sek = 0;
        //validan
        for(int i = 0; i < EQUProvera.size(); i++) {
            //ako naidjemo na keca validno je samo ako jos nismo pokupili nijednog keca
            if(EQUProvera[i].second == 1 && !sek) {
                list<Sekcija>::iterator it2 = sekcije.begin();
                while (it2 != sekcije.end()){
                    if(!(*it2).ime.compare(EQUProvera[i].first.c_str())){
                        sek = &(*it2);
                        break;
                    }
                    it2++;
                } 
            }
            else if(EQUProvera[i].second == 0);
            else {
                cout << "Nevalidan EQU izraz za simbol: " << (*it).imeSimbola.c_str() <<  "\n";
                exit(-1);
            }
        }
        //ubaci simbol u tabelu
        Simbol sim;
        sim.definisan = true;
        sim.eksterni = false;
        sim.globalan = false;
        sim.ime = (*it).imeSimbola;
        if(!sek) {
            list<Sekcija>::iterator i = sekcije.begin();
            i++;
            sim.sekcija = &(*i);
        }
        else sim.sekcija = sek;
        sim.vrednost = sumaLiterala;
        //vrednost
        for(list<EQUZapis>::iterator it3 = (*it).tabela.begin(); it3!= (*it).tabela.end(); it3++){
            //samo ako je razlicito od literala
            if((*it3).sekcija->ime.compare("ABS")) sim.vrednost += ( ((*it3).vrednost == 1 ) ? dohvatiSimbol((*it3).imeSimbola)->vrednost : -(dohvatiSimbol((*it3).imeSimbola)->vrednost));
        }
        //ako je ovo definicija za neki simbol koji vec postoji kao nedefinisan u tabeli onda azuriraj
        if(dohvatiSimbol(sim.ime)) {
            azurirajSimbol(sim);
        }
        else {
            sim.ID = brSimbola++;
            dodajSimbol(sim);
        }
        //izbaci ga iz liste ako si ga definisao
        tabelaEQUSimbola.erase(it);
        it = tabelaEQUSimbola.begin();
        it--;
    }
    //ako ima jos koji equ simbol onda ne valja
    if(tabelaEQUSimbola.size() > 0) {
        cout << "Neizracunljiv EQU simbol: \"" << (*(tabelaEQUSimbola.begin())).imeSimbola.c_str() << "\" \n";
        exit(-1);
    }
    //backpatching
    for(list<Simbol>::iterator it = tabelaSimbola.begin(); it != tabelaSimbola.end(); it++) {
        //da li je def simbol
        if(!(*it).definisan && !(*it).eksterni) {
            cout << "Simbol \"" << (*it).ime.c_str() << "\" nije definisan." <<"\n";
            exit(-1);
        }
        //ako jeste krpi obracanja unapred i rel zapise
        for(list<ObrUnapredZapis>::iterator oZapIt = (*it).tabelaObrUnpared.begin(); oZapIt != (*it).tabelaObrUnpared.end(); oZapIt++) {
            //;
            RelokZapis* mojRelZapis = 0;
            for(list<RelokZapis>::iterator rZapIt = (*oZapIt).sekcija->relTabela.begin(); rZapIt != (*oZapIt).sekcija->relTabela.end(); rZapIt++) {
                if((*rZapIt).adresa == (*oZapIt).adresa) mojRelZapis = &(*rZapIt);
            }
            //
            if(mojRelZapis) {
                if(!mojRelZapis->tip.compare("apsolutan")) {//apsolutan
                    mojRelZapis->IDSimbola = (*it).sekcija->ID;
                    //
                    if(!(*it).globalan) {
                        //pocinje nam sadrzaj sa space pa uvek kasnimo 1 karakter, pa zato +1
                        char* pom = &((*oZapIt).sekcija->sadrzaj[3*(*oZapIt).adresa + 1]);
                        //
                        stringstream ss;
                        ss << uppercase << hex << (*it).vrednost;
                        string hexVrednost = ss.str();
                        //ne moze da stane
                        if((hexVrednost.size() > 2 && (*oZapIt).velicina == 1) || (hexVrednost.size() > 4 && (*oZapIt).velicina == 2)) {
                            cout << "Vrednost simbola \"" << (*it).vrednost << "\" je prevelika za njegovo koriscenje u sekciji \"" << (*oZapIt).sekcija->ime.c_str() << "\"" <<  ", na adresi:" << (*oZapIt).adresa << "\n";
                            exit(-1);
                        }
                        //
                        switch (hexVrednost.size()){
                            case 1:
                                hexVrednost = "0" + hexVrednost + " 00";
                                break;
                            case 2:
                                hexVrednost = hexVrednost + " 00";
                                break;
                            case 3:
                                hexVrednost = hexVrednost.substr(1,1) + hexVrednost.substr(2,1) + " 0" + hexVrednost.substr(0,1); 
                                break;
                            case 4:
                                hexVrednost = hexVrednost.substr(2,1) + hexVrednost.substr(3,1) + " " + hexVrednost.substr(0,1) + hexVrednost.substr(1,1); 
                                break;
                            default:
                                break;
                        }//
                        //nizi bajt
                        *pom = hexVrednost[0];
                        pom++;
                        *pom = hexVrednost[1];
                        pom++;
                        if((*oZapIt).velicina == 2) {
                            //da preskocimo space izmedju dva bajta: 01 00
                            pom++;
                            //visi bajt
                            *pom = hexVrednost[3];
                            pom++;
                            *pom = hexVrednost[4];
                        }
                    }
                }
                else {//pc relokativan
                    //
                    long vrednost = 0;
                    //procitaj stari sadrzaj sa lokacije oZapisa
                    string stariSadrzaj = "";
                    stariSadrzaj += mojRelZapis->sekcija->sadrzaj.substr(mojRelZapis->adresa * 3 + 1, 2);
                    if((*oZapIt).velicina == 2) stariSadrzaj = mojRelZapis->sekcija->sadrzaj.substr(mojRelZapis->adresa * 3 + 4, 2) + stariSadrzaj;
                    //prebaci taj sadrzaj iz stringa u long
                    //evo je vrednost
                    vrednost = strtol(stariSadrzaj.c_str(), NULL, 16);
                    if((*oZapIt).velicina == 2) {
                        if(vrednost >= (1L << 15)) vrednost -= (1L << 16);
                    }
                    else {
                        if(vrednost >= (1L << 7)) vrednost -= (1L << 8);
                    }
                    if((*it).sekcija->ID == mojRelZapis->sekcija->ID) {//onda obrisi rel zapis i upisi odg vrednost na mesto koriscenja
                        //oznaci kao nevalidan
                        mojRelZapis->tip = "obrisan";
                        vrednost += (*it).vrednost - mojRelZapis->adresa;
                    }
                    else {
                        if(!(*it).globalan) {
                            mojRelZapis->IDSimbola = (*it).sekcija->ID;
                            vrednost += (*it).vrednost;
                        }
                    }
                    //pocinje nam sadrzaj sa space pa uvek kasnimo 1 karakter, pa zato +1
                    char* pom = &((*oZapIt).sekcija->sadrzaj[3*(*oZapIt).adresa + 1]);
                    //
                    stringstream ss;
                    ss << uppercase << hex << vrednost;
                    string hexVrednost = ss.str();
                    //ne moze da stane
                    if(hexVrednost.length() > 4 && hexVrednost[0] == 'F') hexVrednost = hexVrednost.substr(hexVrednost.length() - 4);
                    else if((hexVrednost.size() > 2 && (*oZapIt).velicina == 1) || (hexVrednost.size() > 4 && (*oZapIt).velicina == 2)) {
                        cout << "Vrednost simbola \"" << (*it).ime << "\" je prevelika za njegovo koriscenje u sekciji \"" << (*oZapIt).sekcija->ime.c_str() << "\"" <<  ", na adresi:" << (*oZapIt).adresa << "\n";
                        exit(-1);
                    }
                    switch (hexVrednost.size()){
                        case 1:
                            hexVrednost = "0" + hexVrednost + " 00";
                            break;
                        case 2:
                            hexVrednost = hexVrednost + " 00";
                            break;
                        case 3:
                            hexVrednost =  hexVrednost.substr(1,1) + hexVrednost.substr(2,1) + " 0" + hexVrednost.substr(0,1); 
                            break;
                        case 4:
                            hexVrednost =  hexVrednost.substr(2,1) + hexVrednost.substr(3,1) + " " + hexVrednost.substr(0,1) + hexVrednost.substr(1,1); 
                            break;
                        default:
                            break;
                    }//
                    //nizi bajt
                    *pom = hexVrednost[0];
                    pom++;
                    *pom = hexVrednost[1];
                    pom++;
                    if((*oZapIt).velicina == 2) {
                        //da preskocimo space izmedju dva bajta: 01 00
                        pom++;
                        //visi bajt
                        *pom = hexVrednost[3];
                        pom++;
                        *pom = hexVrednost[4];
                    }
                }
            }
            else {
                cout << "Backpatching greska, ne postoji rel zapis koji se poklapa sa zapisom obracanja unapred" << "\n";
                exit(-1);
            }
        }
    }
    //
    printf("END\n");
}

void Asembler::upisi(ofstream* izlaz){
    //tabela simbola
    *izlaz << "#tabela simbola" << "\n";
    *izlaz << '\t' << "#ime - sek - vr - vid - ID" << "\n";
    list<Simbol>::iterator i = tabelaSimbola.begin();
    stringstream ss;
    while (i!= tabelaSimbola.end()){
        ss.str("");
        ss << uppercase << hex << (*i).vrednost;
        *izlaz << '\t' << (*i).ime.c_str() << " - " << (*i).sekcija->ime.c_str() << " - 0x" << ss.str() << " - ";
        if((*i).globalan) *izlaz << "G - 0x";
        else *izlaz << "L - 0x";
        ss.str("");
        ss << uppercase << hex << (*i).ID;
        *izlaz << ss.str() << "\n";
        //
        i++;
    }
    *izlaz << "\n"; 
    //rel tabele za svaku sek
    list<Sekcija>::iterator sekIt = sekcije.begin();
    while (sekIt!= sekcije.end()){
        if((*sekIt).ID > 1) {
            //
            *izlaz << "#Relokaciona tabela za sekciju " << (*sekIt).ime.c_str() << "\n";
            //
            *izlaz << '\t' << "#adresa - tip - IDSimbola" << "\n";
            //
            list<RelokZapis>::iterator relZapIt = (*sekIt).relTabela.begin();
            while(relZapIt != (*sekIt).relTabela.end()) {
                if(!(*relZapIt).tip.compare("obrisan")) {
                    relZapIt++;
                    continue;
                }
                ss.str("");
                ss << uppercase << hex << (*relZapIt).adresa;
                *izlaz << '\t' <<  "0x" << ss.str() << " - " ;
                ss.str("");
                ss << uppercase << hex << (*relZapIt).IDSimbola;
                *izlaz << (*relZapIt).tip << " - 0x" << ss.str() << "\n";
                relZapIt++;
            }
            //
            *izlaz << "\n";
        }
        sekIt++;
    }
    //sadrzaj svake sek
    sekIt = sekcije.begin();
    while(sekIt != sekcije.end()){
        if((*sekIt).ID > 1) {
            //
            *izlaz << "#" << (*sekIt).ime.c_str() << "\n";
            *izlaz << (*sekIt).sadrzaj.c_str() << "\n\n";
        }
        //
        sekIt++;
    }
}

