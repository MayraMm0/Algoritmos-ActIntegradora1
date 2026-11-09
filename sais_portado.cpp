#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <iterator>
#include "memoria.h"

using namespace std;

// Estructura auxiliar para los buckets
struct Bucket {
    int start;
    int end;
};

// Divide el Suffix Array en buckets para cada caracter
vector<Bucket> getBuckets(const vector<int>& T) {

    if (T.empty()) return {};

    int max_val = *max_element(T.begin(), T.end());

    vector<int> count(max_val + 1, 0);
    vector<Bucket> buckets(max_val + 1, {0, 0});

    for (int c : T) {
        count[c]++;
    }

    int start = 0;

    for (int c = 0; c <= max_val; ++c) {

        if (count[c] > 0) {
            buckets[c].start = start;
            buckets[c].end = start + count[c];
            start += count[c];
        }
    }

    return buckets;
}

// Compara dos subcadenas LMS
bool lms_equal(
    const vector<int>& T,
    const vector<bool>& t,
    int i,
    int j,
    const vector<int>& LMS
) {

    if (i == j) return true;

    int end_i = (LMS[i] != -1) ? LMS[i] : i;
    int end_j = (LMS[j] != -1) ? LMS[j] : j;

    int len_i = end_i - i + 1;
    int len_j = end_j - j + 1;

    if (len_i != len_j) return false;

    for (int k = 0; k < len_i; ++k) {

        if (i + k >= (int)T.size() ||
            j + k >= (int)T.size()) {
            return false;
        }

        if (T[i + k] != T[j + k] ||
            t[i + k] != t[j + k]) {
            return false;
        }
    }

    return true;
}

// Algoritmo SA-IS
vector<int> sais_optimizado(const vector<int>& T) {

    int n = T.size();

    if (n == 0) return {};
    if (n == 1) return {0};

    // Clasificacion S/L
    // true = S, false = L
    vector<bool> t(n, false);

    t[n - 1] = true;

    for (int i = n - 1; i > 0; --i) {

        if (T[i - 1] == T[i]) {
            t[i - 1] = t[i];
        } else {
            t[i - 1] = (T[i - 1] < T[i]);
        }
    }

    // Calculo de buckets
    vector<Bucket> buckets = getBuckets(T);

    int max_val = (int)buckets.size() - 1;

    vector<int> count_arr(max_val + 1, 0);
    vector<int> SA(n, -1);

    // Identificacion LMS
    vector<int> LMS(n, -1);

    int end_pos = -1;

    for (int i = n - 1; i > 0; --i) {

        if (t[i] && !t[i - 1]) {

            count_arr[T[i]] += 1;

            int revoffset = count_arr[T[i]];

            SA[buckets[T[i]].end - revoffset] = i;

            if (end_pos != -1) {
                LMS[i] = end_pos;
            }

            end_pos = i;
        }
    }

    LMS[n - 1] = n - 1;

    // Inducir sufijos L
    fill(count_arr.begin(), count_arr.end(), 0);

    for (int i = 0; i < n; ++i) {

        if (SA[i] > 0) {

            int j = SA[i] - 1;

            if (!t[j]) {

                int symbol = T[j];
                int offset = count_arr[symbol];

                SA[buckets[symbol].start + offset] = j;

                count_arr[symbol] = offset + 1;
            }
        }
    }

    // Inducir sufijos S
    fill(count_arr.begin(), count_arr.end(), 0);

    for (int i = n - 1; i >= 0; --i) {

        if (SA[i] > 0) {

            int j = SA[i] - 1;

            if (t[j]) {

                int symbol = T[j];

                count_arr[symbol] += 1;

                int revoffset = count_arr[symbol];

                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    // Renombrar subcadenas LMS
    vector<int> namesp(n, -1);

    int name = 0;
    int prev = -1;

    for (int i = 0; i < n; ++i) {

        if (SA[i] > 0 && t[SA[i]] && !t[SA[i] - 1]) {

            int idx = SA[i];

            if (prev != -1 &&
                !lms_equal(T, t, prev, idx, LMS)) {
                name++;
            }

            prev = idx;
            namesp[idx] = name;
        }
    }

    // Crear texto reducido
    vector<int> names;
    vector<int> SApIdx;

    for (int i = 0; i < n; ++i) {

        if (namesp[i] != -1) {
            names.push_back(namesp[i]);
            SApIdx.push_back(i);
        }
    }

    vector<int> sub_SA;

    if (name < (int)names.size() - 1) {

        sub_SA = sais_optimizado(names);

    } else {

        sub_SA = vector<int>(names.size(), -1);

        for (size_t i = 0; i < names.size(); ++i) {

            int n_val = names[i];

            sub_SA[n_val] = i;
        }
    }

    // Colocacion final de LMS
    fill(SA.begin(), SA.end(), -1);
    fill(count_arr.begin(), count_arr.end(), 0);

    for (int i = (int)sub_SA.size() - 1; i >= 0; --i) {

        int pos = SApIdx[sub_SA[i]];

        count_arr[T[pos]] += 1;

        int revoffset = count_arr[T[pos]];

        SA[buckets[T[pos]].end - revoffset] = pos;
    }

    // Inducir sufijos L
    fill(count_arr.begin(), count_arr.end(), 0);

    for (int i = 0; i < n; ++i) {

        if (SA[i] > 0) {

            int j = SA[i] - 1;

            if (!t[j]) {

                int symbol = T[j];
                int offset = count_arr[symbol];

                SA[buckets[symbol].start + offset] = j;

                count_arr[symbol] = offset + 1;
            }
        }
    }

    // Inducir sufijos S
    fill(count_arr.begin(), count_arr.end(), 0);

    for (int i = n - 1; i >= 0; --i) {

        if (SA[i] > 0) {

            int j = SA[i] - 1;

            if (t[j]) {

                int symbol = T[j];

                count_arr[symbol] += 1;

                int revoffset = count_arr[symbol];

                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    return SA;
}

// Verificar que el SA sea una permutacion y este ordenado
bool verificar_sa(
    const vector<int>& T,
    const vector<int>& SA
) {

    int n = T.size();

    if ((int)SA.size() != n) return false;

    vector<char> visto(n, 0);

    for (int x : SA) {

        if (x < 0 || x >= n || visto[x]) {
            return false;
        }

        visto[x] = 1;
    }

    for (int i = 0; i + 1 < n; ++i) {

        if (!lexicographical_compare(
            T.begin() + SA[i], T.end(),
            T.begin() + SA[i + 1], T.end()
        )) {
            return false;
        }
    }

    return true;
}

// Procesar cada libro y medir tiempo y memoria
void procesar_libro(const string& ruta_libro) {

    ifstream archivo(ruta_libro);

    if (!archivo.is_open()) {

        cout << "No se encontro el archivo: "
             << ruta_libro << endl;

        return;
    }

    string texto(
        (istreambuf_iterator<char>(archivo)),
        istreambuf_iterator<char>()
    );

    archivo.close();

    // Convertir caracteres y agregar centinela
    vector<int> T;

    T.reserve(texto.size() + 1);

    for (char c : texto) {
        T.push_back((unsigned char)c + 1);
    }

    // Centinela unico y menor que todos los caracteres
    T.push_back(0);

    // Medicion de memoria
    const size_t base = mem_actual;
    mem_pico = mem_actual;

    // Medicion de tiempo
    auto tiempo_inicio = chrono::steady_clock::now();

    vector<int> SA = sais_optimizado(T);

    auto tiempo_fin = chrono::steady_clock::now();

    double tiempo_total =
        chrono::duration<double>(
            tiempo_fin - tiempo_inicio
        ).count();

    double memoria_pico = mb(mem_pico - base);

    // Verificacion fuera de las mediciones
    bool correcto = verificar_sa(T, SA);

    cout << "Libro: " << ruta_libro << endl;

    cout << "  Caracteres procesados: "
         << T.size() << endl;

    cout << "  Tiempo de ejecucion: "
         << tiempo_total << " segundos" << endl;

    cout << "  Memoria pico: "
         << memoria_pico << " MB" << endl;

    cout << "  Verificacion del SA: "
         << (correcto ? "OK" : "FALLO") << endl;

    cout << "--------------------------------------------------"
         << endl;
}