import sys
import time
import tracemalloc

# Aumentar límite de recursión para archivos txt más grandes
sys.setrecursionlimit(300000)


# Divide suffix array en buckets para cada caracter
def getBuckets(T):
    max_val = max(T) if T else 0
    count = [0] * (max_val + 1)
    buckets = {}

    for c in T:
        count[c] += 1

    start = 0

    for c in range(max_val + 1):
        if count[c] > 0:
            buckets[c] = (start, start + count[c])
            start += count[c]

    return buckets


# Compara dos subcadenas LMS sin crear sublistas
def lms_equal(T, t, i, j, LMS):

    if i == j:
        return True

    end_i = LMS.get(i, i)
    end_j = LMS.get(j, j)

    len_i = end_i - i + 1
    len_j = end_j - j + 1

    if len_i != len_j:
        return False

    for k in range(len_i):

        if i + k >= len(T) or j + k >= len(T):
            return False

        if T[i + k] != T[j + k] or t[i + k] != t[j + k]:
            return False

    return True


def sais(T):

    # Evitar errores cuando el texto está vacío
    if len(T) == 0:
        return []

    # 1. Clasificación de sufijos entre S y L
    # S = 1
    # L = 0
    # Se utiliza bytearray para almacenar la clasificación
    t = bytearray(len(T))

    t[len(T) - 1] = 1

    for i in range(len(T) - 1, 0, -1):

        if T[i - 1] == T[i]:
            t[i - 1] = t[i]
        else:
            t[i - 1] = 1 if T[i - 1] < T[i] else 0

    # 2. Cálculo de cubetas
    buckets = getBuckets(T)

    # 3. Identificar sufijos LMS y primera ordenación inducida
    count = {}
    SA = [-1] * len(T)
    LMS = {}

    end = None

    for i in range(len(T) - 1, 0, -1):

        if t[i] == 1 and t[i - 1] == 0:

            revoffset = count[T[i]] = count.get(T[i], 0) + 1
            SA[buckets[T[i]][1] - revoffset] = i

            if end is not None:
                LMS[i] = end

            end = i

    if end is not None:
        LMS[len(T) - 1] = end
    else:
        LMS[len(T) - 1] = len(T) - 1

    # Inducir sufijos L (recorrer de izquierda a derecha)
    count = {}

    for i in range(len(T)):

        if SA[i] > 0:

            j = SA[i] - 1

            if t[j] == 0:

                symbol = T[j]
                offset = count.get(symbol, 0)

                SA[buckets[symbol][0] + offset] = j
                count[symbol] = offset + 1

    # Inducir sufijos S (recorrer de derecha a izquierda)
    count = {}

    for i in range(len(T) - 1, -1, -1):

        if SA[i] > 0:

            j = SA[i] - 1

            if t[j] == 1:

                symbol = T[j]
                revoffset = count[symbol] = count.get(symbol, 0) + 1

                SA[buckets[symbol][1] - revoffset] = j

    # 4. Renombrar subcadenas LMS y llamada recursiva
    namesp = [-1] * len(T)
    name = 0
    prev = None

    for i in range(len(SA)):

        if SA[i] > 0 and t[SA[i]] == 1 and t[SA[i] - 1] == 0:

            idx = SA[i]

            if prev is not None and not lms_equal(T, t, prev, idx, LMS):
                name += 1

            prev = idx
            namesp[idx] = name

    # Crear texto reducido usando los nombres de las subcadenas LMS
    names = []
    SApIdx = []

    for i in range(len(T)):

        if namesp[i] != -1:
            names.append(namesp[i])
            SApIdx.append(i)

    # Resolver recursivamente si existen nombres duplicados
    if name < len(names) - 1:

        sub_SA = sais(names)

    else:

        sub_SA = [-1] * len(names)

        for i, n in enumerate(names):
            sub_SA[n] = i

    # 5. Colocación final de LMS y ordenación inducida definitiva
    SA = [-1] * len(T)
    count = {}

    for i in range(len(sub_SA) - 1, -1, -1):

        pos = SApIdx[sub_SA[i]]

        revoffset = count[T[pos]] = count.get(T[pos], 0) + 1

        SA[buckets[T[pos]][1] - revoffset] = pos

    # Inducir sufijos L
    count = {}

    for i in range(len(T)):

        if SA[i] > 0:

            j = SA[i] - 1

            if t[j] == 0:

                symbol = T[j]
                offset = count.get(symbol, 0)

                SA[buckets[symbol][0] + offset] = j
                count[symbol] = offset + 1

    # Inducir sufijos S
    count = {}

    for i in range(len(T) - 1, -1, -1):

        if SA[i] > 0:

            j = SA[i] - 1

            if t[j] == 1:

                symbol = T[j]
                revoffset = count[symbol] = count.get(symbol, 0) + 1

                SA[buckets[symbol][1] - revoffset] = j

    return SA


def procesar_libro(ruta_libro):

    try:
        # 1. Leer archivo
        with open(ruta_libro, 'r', encoding='utf-8') as l:
            texto = l.read()

    except FileNotFoundError:
        print("No se encontró el archivo")
        return

    # Añadir símbolo terminal '$'
    if not texto.endswith('$'):
        texto += '$'

    T = [ord(c) for c in texto]

    # 2. Iniciar medición de memoria y tiempo
    tracemalloc.start()
    tiempo_inicio = time.perf_counter()

    # Ejecutar algoritmo SA-IS
    SA = sais(T)

    tiempo_fin = time.perf_counter()

    current, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()

    # 3. Mostrar métricas
    tiempo_total = tiempo_fin - tiempo_inicio
    memoria_pico_mb = peak / (1024 * 1024)

    print(f"Libro: {ruta_libro}")
    print(f"  - Caracteres procesados: {len(T)}")
    print(f"  - Tiempo de ejecución: {tiempo_total:.4f} segundos")
    print(f"  - Memoria pico alcanzada: {memoria_pico_mb:.2f} MB")
    print("-" * 50)