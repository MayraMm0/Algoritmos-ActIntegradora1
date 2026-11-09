#include <iostream>
#include <vector>
#include <fstream>
#include <iterator>
#include <string>
#include <chrono>
#include <algorithm>
#include <tuple>
#include "memoria.h"

using namespace std;

struct SubstrRank {
    int left_rank;
    int right_rank;
    int index;
};

vector<int> make_ranks(const vector<SubstrRank>& substr_rank, int n) {

    int r = 1;
    vector<int> rank(n, -1);

    if (n == 0) return rank;

    rank[substr_rank[0].index] = r;

    for (int i = 1; i < n; i++) {

        if (substr_rank[i].left_rank != substr_rank[i - 1].left_rank ||
            substr_rank[i].right_rank != substr_rank[i - 1].right_rank) {

            r++;
        }

        rank[substr_rank[i].index] = r;
    }

    return rank;
}

vector<int> suffix_array(const string& T) {

    int n = static_cast<int>(T.size());

    if (n == 0) return {};

    vector<SubstrRank> substr_rank;

    for (int i = 0; i < n; i++) {

        const int left = static_cast<unsigned char>(T[i]);

        const int right = (i < n - 1)
            ? static_cast<unsigned char>(T[i + 1])
            : 0;

        substr_rank.push_back({left, right, i});
    }

    sort(substr_rank.begin(), substr_rank.end(),
        [](const SubstrRank& a, const SubstrRank& b) {
            return tie(a.left_rank, a.right_rank) <
                   tie(b.left_rank, b.right_rank);
        });

    int l = 2;

    while (l < n) {

        vector<int> rank = make_ranks(substr_rank, n);

        // CORRECCIÓN: antes el ciclo tenía j < 0
        for (int j = 0; j < n; j++) {

            substr_rank[j].left_rank = rank[j];

            substr_rank[j].right_rank = (j + l < n)
                ? rank[j + l]
                : 0;

            substr_rank[j].index = j;
        }

        l *= 2;

        sort(substr_rank.begin(), substr_rank.end(),
            [](const SubstrRank& a, const SubstrRank& b) {
                return tie(a.left_rank, a.right_rank) <
                       tie(b.left_rank, b.right_rank);
            });
    }

    vector<int> SA;

    for (int k = 0; k < n; k++) {
        SA.push_back(substr_rank[k].index);
    }

    return SA;
}

void mediciones(const string& ruta) {

    ifstream archivo(ruta);

    if (!archivo) {
        cout << "No se encontro el archivo: " << ruta << "\n";
        return;
    }

    string T(
        (istreambuf_iterator<char>(archivo)),
        istreambuf_iterator<char>()
    );

    archivo.close();

    // Reiniciar medicion de memoria
    const size_t base = mem_actual;
    mem_pico = mem_actual;

    auto inicio = chrono::steady_clock::now();

    vector<int> SA = suffix_array(T);

    auto fin = chrono::steady_clock::now();

    double tiempo =
        chrono::duration<double>(fin - inicio).count();

    double pico = mb(mem_pico - base);

    cout << "Libro: " << ruta << "\n";
    cout << "  Caracteres procesados: " << T.size() << "\n";
    cout << "  Tiempo de ejecucion: " << tiempo << " segundos\n";
    cout << "  Memoria pico: " << pico << " MB\n";
    cout << "--------------------------------------------------\n";
}