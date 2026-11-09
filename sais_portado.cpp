#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <chrono>

using namespace std;

// Estructura auxiliar compacta para un bucket (inicio y fin)
struct Bucket {
    int start;
    int end;
};

// OPTIMIZACIÓN 1: Paso por referencia constante (const vector<int>& T) para evitar copias.
// OPTIMIZACIÓN 2: Uso de vector directo en lugar de std::map para accesos en O(1) en lugar de O(log N).
// Divide el Suffix Array en buckets para cada carácter.
vector<Bucket> getBuckets(const vector<int>& T) {
    if (T.empty()) return {};

    int max_val = *max_element(T.begin(), T.end());
    vector<int> count(max_val + 1, 0);
    vector<Bucket> buckets(max_val + 1, {0, 0});

    for (int c : T) {
        count[c] += 1;
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

// OPTIMIZACIÓN 1: Paso de parámetros por referencia constante.
// OPTIMIZACIÓN 3: 't' ahora es const vector<bool>& (bit vector), donde true = S, false = L.
// OPTIMIZACIÓN 2: 'LMS' es un vector<int> indexado directamente en O(1), evitando búsquedas en mapa.
bool lms_equal(const vector<int>& T, const vector<bool>& t, int i, int j, const vector<int>& LMS) {
    if (i == j) { return true; }

    int end_i = (LMS[i] != -1) ? LMS[i] : i;
    int end_j = (LMS[j] != -1) ? LMS[j] : j;

    int len_i = end_i - i + 1;
    int len_j = end_j - j + 1;

    if (len_i != len_j) { return false; }

    for (int k = 0; k < len_i; ++k) {
        if (i + k >= (int)T.size() || j + k >= (int)T.size()) { return false; }
        if (T[i + k] != T[j + k] || t[i + k] != t[j + k]) { return false; }
    }
    return true;
}

// Algoritmo principal SA-IS con optimizaciones de espacio y tiempo
// OPTIMIZACIÓN 1: Paso por referencia (const vector<int>& T) evita duplicar vectores en memoria por cada llamada recursiva.
// REQUISITO: el último símbolo de T debe ser un centinela ÚNICO y ESTRICTAMENTE MENOR que todos los demás (aquí, 0).
vector<int> sais_optimizado(const vector<int>& T) {
    int n = T.size();

    // CORRECCIÓN: casos base para entradas vacías o de un solo elemento.
    if (n == 0) return {};
    if (n == 1) return {0};

    // OPTIMIZACIÓN 3: Reemplazo de vector<char> ('S'/'L') por vector<bool> (Bit Vector/Bit Array).
    // true representa tipo 'S', false representa tipo 'L'.
    // Esto reduce el consumo de memoria de 1 byte por carácter a solo 1 bit por carácter (8x más compacto).
    vector<bool> t(n, false);
    t[n - 1] = true; // El centinela siempre es S (true)

    // FASE 1. Clasificación de sufijos entre S (true) y L (false)
    for (int i = n - 1; i > 0; --i) {
        if (T[i - 1] == T[i]) {
            t[i - 1] = t[i];
        } else {
            t[i - 1] = (T[i - 1] < T[i]); // true si es S, false si es L
        }
    }

    // FASE 2. Cálculo de buckets
    vector<Bucket> buckets = getBuckets(T);

    // CORRECCIÓN: max_val se deduce del tamaño de buckets (antes estaba calculado dos veces).
    int max_val = (int)buckets.size() - 1;

    // OPTIMIZACIÓN 2: Reemplazo de std::map por vector<int> indexado por símbolo para O(1).
    vector<int> count_arr(max_val + 1, 0);
    vector<int> SA(n, -1);

    // OPTIMIZACIÓN 2: LMS representado como un vector directo de tamaño n inicializado en -1
    vector<int> LMS(n, -1);
    int end_pos = -1;

    // FASE 3. Identificar LMS y primera inducción
    for (int i = n - 1; i > 0; --i) {
        // LMS cuando es S (true) y el de la izquierda es L (false)
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

    // CORRECCIÓN: la subcadena LMS del centinela termina en sí misma.
    // Antes se sobrescribía con el LMS más a la izquierda, lo que hacía que lms_equal comparara longitudes erróneas.
    LMS[n - 1] = n - 1;

    // Inducir sufijos L (recorrer de izq -> der)
    fill(count_arr.begin(), count_arr.end(), 0); // Reiniciar punteros en O(K)
    for (int i = 0; i < n; ++i) {
        if (SA[i] > 0) { // Valida que exista un sufijo anterior
            int j = SA[i] - 1;
            if (!t[j]) { // Si ese anterior es tipo L (false)
                int symbol = T[j];
                int offset = count_arr[symbol];
                SA[buckets[symbol].start + offset] = j; // Colocar j en puntero actual de su bucket
                count_arr[symbol] = offset + 1; // Avanzar el puntero de la bucket +1
            }
        }
    }

    // Inducir sufijos S (recorrer de der -> izq)
    fill(count_arr.begin(), count_arr.end(), 0); // Reiniciar punteros
    for (int i = n - 1; i >= 0; --i) {
        if (SA[i] > 0) {
            int j = SA[i] - 1;
            if (t[j]) { // Si ese anterior es tipo S (true)
                int symbol = T[j];
                count_arr[symbol] += 1;
                int revoffset = count_arr[symbol];
                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    // FASE 4. Renombrar subcadenas LMS y llamada recursiva
    vector<int> namesp(n, -1);
    int name = 0;
    int prev = -1;
    for (int i = 0; i < n; ++i) {
        if (SA[i] > 0 && t[SA[i]] && !t[SA[i] - 1]) {
            int idx = SA[i];
            if (prev != -1 && !lms_equal(T, t, prev, idx, LMS)) {
                name += 1; // Si dos no son iguales incrementa name
            }
            prev = idx;
            namesp[idx] = name; // Si dos consecutivas son idénticas se asigna mismo name
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
    // Si hay nombres duplicados resuelve recursivamente
    if (name < (int)names.size() - 1) {
        // Obtener el Suffix Array de la cadena reducida (pases recursivos por referencia)
        sub_SA = sais_optimizado(names);
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
    fill(count_arr.begin(), count_arr.end(), 0);

    // Colocar LMS en su orden real exacto al fondo de los buckets
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
            if (!t[j]) { // Si es L (false)
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
            if (t[j]) { // Si es S (true)
                int symbol = T[j];
                count_arr[symbol] += 1;
                int revoffset = count_arr[symbol];
                SA[buckets[symbol].end - revoffset] = j;
            }
        }
    }

    return SA;
}

// CORRECCIÓN (nuevo): verifica que el SA sea una permutación válida y que los sufijos estén en orden lexicográfico.
// Se llama fuera de la medición de tiempo. Si no la necesitas en el reporte final, puedes quitar la llamada.
bool verificar_sa(const vector<int>& T, const vector<int>& SA) {
    int n = T.size();
    if ((int)SA.size() != n) return false;

    vector<char> visto(n, 0);
    for (int x : SA) {
        if (x < 0 || x >= n || visto[x]) return false; // Debe ser una permutación de 0..n-1
        visto[x] = 1;
    }
    for (int i = 0; i + 1 < n; ++i) {
        if (!lexicographical_compare(T.begin() + SA[i],     T.end(),
                                     T.begin() + SA[i + 1], T.end()))
            return false; // Orden estricto entre sufijos consecutivos
    }
    return true;
}

void procesar_libro(const string& ruta_libro) {
    ifstream archivo(ruta_libro);
    if (!archivo.is_open()) {
        cout << "No se encontró el archivo: " << ruta_libro << endl;
        return;
    }

    string texto((istreambuf_iterator<char>(archivo)), istreambuf_iterator<char>());
    archivo.close();

    // CORRECCIÓN: ya no se agrega '$' al texto (ASCII 36 NO es el menor: espacio, '\n', '!' y '"' son menores).
    // Se desplaza cada carácter +1 (rango 1..256) y se agrega un centinela 0, único y estrictamente mínimo.
    vector<int> T;
    T.reserve(texto.size() + 1);
    for (char c : texto) {
        T.push_back((unsigned char)c + 1);
    }
    T.push_back(0);

    // Iniciar perfilado de tiempo
    auto tiempo_inicio = chrono::high_resolution_clock::now();

    vector<int> SA = sais_optimizado(T);

    auto tiempo_fin = chrono::high_resolution_clock::now();
    chrono::duration<double> tiempo_total = tiempo_fin - tiempo_inicio;

    // Verificación de correctitud (fuera del tiempo medido)
    bool correcto = verificar_sa(T, SA);

    cout << "Libro: " << ruta_libro << endl;
    cout << "  - Caracteres procesados: " << T.size() << endl;
    cout << "  - Tiempo de ejecución: " << tiempo_total.count() << " segundos" << endl;
    cout << "  - Verificación del SA: " << (correcto ? "OK" : "FALLO") << endl;
    cout << "--------------------------------------------------" << endl;
}