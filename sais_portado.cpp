#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>

using namespace std;

// Estructura auxiliar para un bucket
struct Bucket {
    int start;
    int end;
};

//Función auxiliar buckets 
// Divide suffix array en buckets para cada caracter
map<int, Bucket> getBuckets(vector<int> T) {
   int max_val = 0;
   if (!T.empty()) {
    max_val = *max_element(T.begin(), T.end());
   }

   vector<int> count(max_val +1 , 0);
   map<int, Bucket> buckets;

   for(int c: T) {
    count[c] += 1;
   }

   int start = 0;
   for(int c = 0; c <= max_val; ++c){
        if(count[c] > 0) {
            Bucket b;
            b.start = start;
            b.end = start + count[c];
            buckets[c] = b;

            start += count[c];
        }
   }
   return buckets;
}

// Función auxiliar que compara dos subcadenas LMS
// compara elemento por elemento sin crear sub-listas (mejor para ram)
bool lms_equal(vector<int> T, vector<char> t, int i, int j, map<int, int> LMS) {
    if (i == j) { return true; }

    // manejo seguro por si algún índice es la pos terminal
    int end_i = LMS.count(i) ? LMS[i] : i;
    int end_j = LMS.count(j) ? LMS[j] : j;

    int len_i = end_i - i + 1;
    int len_j = end_j - j + 1;

    if (len_i != len_j) { return false; }

    for(int k = 0; k < len_i; ++k) {
        if (i + k >= T.size() || j + k >= T.size()) { return false; }
        if (T[i + k] != T[j + k] || t[i + k] != t[j + k]) { return false; }
    }
    return true;
}


// Algoritmo principal SAIS sin optimizaciones
vector<int> sais_portado(vector<int> T) {
    int n = T.size();

    // FASE 1. clasificación de sufijos entre S y L
        // S -> si T[i-1] < T[i]
        // L -> si T[i-1] > T[i]
    vector<char> t(n, '_');
    t[n - 1] = 'S'; // '$' siempre es S
    for(int i = n - 1; i > 0; --i) {
        if(T[i-1] == T[i]) {
            t[i - 1] = t[i];
        } else {
            t[i - 1] = (T[i-1] < T[i]) ? 'S' : 'L';
        }
    }

    // FASE 2. cálculo de buckets
    map<int, Bucket> buckets = getBuckets(T);

    // FASE 3. identificar LMS y primera inducción
    map<int, int> count_map;
    vector<int> SA(n, -1);
    map<int, int> LMS; // Left Most S-type, cuando es S y el de la izq es L
    int end_pos = -1;
    for(int i = n - 1; i > 0; --i){
        if (t[i] == 'S' && t[i-1] == 'L') { // LMS cuando es S y el de la izq es L
            count_map[T[i]] += 1;
            int revoffset = count_map[T[i]];
            SA[buckets[T[i]].end - revoffset] = i;
            
            if (end_pos != -1) {
                LMS[i] = end_pos;
            }
            end_pos = i;
        }
    }

    if (end_pos != -1) {
        LMS[n -1] = end_pos;
    } else {
        LMS[n - 1] = n - 1;
    }

    // inducir sufijos L (recorrer de izq -> der)
    count_map.clear(); // Reiniciar punteros a inicio de cada bucket
    for(int i = 0; i < n; ++i) {
        if (SA[i] > 0) { // Valida que exista un sufijo anterior
            int j = SA[i] - 1;
            if (t[j] == 'L') { // Si ese anterior es tipo L
                int symbol = T[j];
                int offset = count_map[symbol];
                SA[buckets[symbol].start + offset] = j; // Colocar j en puntero actual de su bucket
                count_map[symbol] = offset + 1; // avanzar el puntero de la bucket +1
            }
        }
    }

    // inducir sufijos S (recorrer de der -> izq)
    count_map.clear(); // Reiniciar punteros al final de cada bucket
    for(int i = n-1; i >= 0; --i) {
        if( SA[i] > 0){
            int j = SA[i] - 1;
            if(t[j] == 'S') { // Si ese anterior es tipo S
                int symbol = T[j];
                count_map[symbol] += 1;
                int revoffset = count_map[symbol];
                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    // FASE 4. Renombrar subcadenas LMS y llamada recursivsa
    vector<int> namesp(n, -1);
    int name = 0;
    int prev = -1;
    for(int i = 0; i < n; ++i){
        if(SA[i] > 0 && t[SA[i]] == 'S' && t[SA[i]-1] == 'L') {
            int idx = SA[i];
            if( prev != -1 && !lms_equal(T, t, prev, idx, LMS)) {
                name += 1; // Si dos no son iguales incremente name
            }
            prev = idx;
            namesp[idx] = name; // Si dos consecutivas son identicas se asigna mismo name
        }
    }

    // Crear texto reducido usando names como caracteres
    vector<int> names;
    vector<int> SApIdx;
    for (int i = 0; i < n; ++i) {
        if (namesp[i] != -1) {
            names.push_back(namesp[i]); 
            SApIdx.push_back(i);
        }
    }

    vector<int> sub_SA;
    // si hay nombres duplicados resuelve recursivamente
    if(name < names.size() - 1) {
        // obtener el Suffix Array de la cadena reducida
        sub_SA = sais_portado(names);
    } else {
        // Base de la recursión: si todos los nombres son únicos, SA directo
        sub_SA = vector<int>(names.size(), -1);
        for (size_t i = 0; i < names.size(); ++i) {
            int n_val = names[i];
            sub_SA[n_val] = i;
        }
    }

    // Colocación final de LMS e induced sort definitivo
    fill(SA.begin(), SA.end(), -1);
    count_map.clear();

    // colocar LMS en su orden real exacto al fondo de las buckets
    for (int i = (int)sub_SA.size() - 1; i >= 0; --i) {
        int pos = SApIdx[sub_SA[i]];
        count_map[T[pos]] += 1;
        int revoffset = count_map[T[pos]];
        SA[buckets[T[pos]].end - revoffset] = pos;
    }

    // induce con conteo final exacto
    // inducir sufijos L
    count_map.clear();
    for (int i = 0; i < n; ++i) {
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            if (t[j] == 'L') {
                int symbol = T[j];
                int offset = count_map[symbol];
                SA[buckets[symbol].start + offset] = j;
                count_map[symbol] = offset + 1;
            }
        }
    }

    // inducir sufijos S
    count_map.clear();
    for (int i = n - 1; i >= 0; --i) {
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            if (t[j] == 'S') {
                int symbol = T[j];
                count_map[symbol] += 1;
                int revoffset = count_map[symbol];
                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    return SA;
}

void procesar_libro(string ruta_libro) {
    ifstream archivo(ruta_libro);
    if (!archivo.is_open()) {
        cout << "No se encontró el archivo" << std::endl;
        return;
    }
    
    string texto((istreambuf_iterator<char>(archivo)), istreambuf_iterator<char>());
    archivo.close();
    
    if (texto.empty() || texto.back() != '$') {
        texto += '$';
    }
    
    vector<int> T;
    for (char c : texto) {
        T.push_back((unsigned char)c);
    }
    
    // Iniciar perfilado de tiempo
    auto tiempo_inicio = chrono::high_resolution_clock::now();
    
    vector<int> SA = sais_portado(T);
    
    auto tiempo_fin = chrono::high_resolution_clock::now();
    chrono::duration<double> tiempo_total = tiempo_fin - tiempo_inicio;
    
    cout << "Libro: " << ruta_libro << endl;
    cout << "  - Caracteres procesados: " << T.size() << endl;
    cout << "  - Tiempo de ejecución: " << tiempo_total.count() << " segundos" << endl;
    cout << "--------------------------------------------------" << endl;
}