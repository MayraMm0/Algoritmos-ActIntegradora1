#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>
using namespace std;

void procesar_libro(string ruta_libro);

void mediciones(const string& ruta);


int main() {
    string littleWomen = "libros/Little_Women.txt";
    string metamorphosis = "libros/Metamorphosis.txt";
    string pridePrejudice = "libros/Pride_and_Prejudice.txt";
    string phantomOpera = "libros/The_Phantom_of_the_Opera.txt";

    vector<string> libros = {metamorphosis, littleWomen, pridePrejudice, phantomOpera};

    cout << " ==== SAIS PORTADO C++ ==== " << endl;
    for (const string& libro : libros) {
        procesar_libro(libro);
    }

    cout << "Manber and Myers" << endl;
    for (const string& libro : libros) {
        mediciones(libro);
    }

    return 0;
}