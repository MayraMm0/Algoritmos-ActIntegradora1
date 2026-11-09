#include <iostream>
#include <vector>
#include <string>

using namespace std;

void procesar_libro(const string& ruta_libro);
void mediciones(const string& ruta);

int main() {

    string littleWomen = "libros/Little_Women.txt";
    string metamorphosis = "libros/Metamorphosis.txt";
    string pridePrejudice = "libros/Pride_and_Prejudice.txt";
    string phantomOpera = "libros/The_Phantom_of_the_Opera.txt";

    vector<string> libros = {
        metamorphosis,
        littleWomen,
        pridePrejudice,
        phantomOpera
    };

    cout << "\n==== SA-IS C++ ====\n";

    for (const string& libro : libros) {
        procesar_libro(libro);
    }

    cout << "\n==== MANBER-MYERS C++ ====\n";

    for (const string& libro : libros) {
        mediciones(libro);
    }

    return 0;
}