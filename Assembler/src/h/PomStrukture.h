#ifndef STRUKTURE_H
#define STRUKTURE_H
#include <string>
#include <vector>

using namespace std;

struct Sekcija;

struct RelokZapis{
    unsigned long adresa;
    unsigned IDSimbola;
    string tip;// apsolutan ili relokatibilan
    Sekcija* sekcija;
};

struct Sekcija{
    string ime;
    unsigned long velicina = 0;
    unsigned ID;// onaj ID koji ima simbol koji je vezan za ovu sekciju
    std::list<RelokZapis> relTabela = std::list<RelokZapis>();
    string sadrzaj = "";
};

struct ObrUnapredZapis {
    Sekcija* sekcija;
    unsigned long adresa;
    unsigned velicina; // 1 - bajt, 2 - rec
};

struct Simbol {
    unsigned vrednost;
    string ime;
    unsigned ID;
    bool definisan;
    Sekcija* sekcija;
    bool globalan;
    bool eksterni;
    std::list<ObrUnapredZapis> tabelaObrUnpared = std::list<ObrUnapredZapis>();
};

struct EQUZapis {
    long vrednost;// -1 ili +1 ili literal
    string imeSimbola;
    Sekcija* sekcija;
};

struct EQURed {
    string imeSimbola;
    list<EQUZapis> tabela = list<EQUZapis>();
    unsigned vrednost;//ekv vrednost simbola
};
#endif