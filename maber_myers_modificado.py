
import time
import tracemalloc


REPS = 3

# Estructura auxiliar para ordenar por pares de rangos
class SubstrRank:
    def __init__(self, left_rank=0, right_rank=0, index=0):
        self.left_rank = left_rank
        self.right_rank = right_rank
        self.index = index


# Convierte tuplas ordenadas en números/rangos
def make_ranks(substr_rank, rank, n):
    r = 1

    # Asignar el primer rango al primer sufijo ordenado
    rank[substr_rank[0].index] = r

    for i in range(1, n):
        if (substr_rank[i].left_rank != substr_rank[i-1].left_rank or
            substr_rank[i].right_rank != substr_rank[i-1].right_rank):
            r += 1

        rank[substr_rank[i].index] = r

    return rank


# Duplica en cada iteración la longitud del prefijo evaluado
def suffix_array(T, tiempos=None):

    medir = tiempos is not None
    if medir:
        for k in ("inicializacion", "orden_inicial", "make_ranks",
                  "actualizar_rangos", "ordenar_rondas"):
            tiempos.setdefault(k, 0.0)
        tiempos.setdefault("rondas", 0)

    n = len(T)

    # Si el texto está vacío, no hay sufijos que ordenar
    if n == 0:
        return []

    t0 = time.perf_counter()

    substr_rank = []

    # 1. L = 1: Inicializar con valores ascii
    for i in range(n):
        substr_rank.append(SubstrRank(ord(T[i]), ord(T[i + 1]) if i < n-1 else 0, i))

    # Crear rank una sola vez fuera del ciclo
    rank = [0] * n

    t1 = time.perf_counter()

    # Primer ordenamiento basado en primeros dos caracteres
    substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))

    t2 = time.perf_counter()

    if medir:
        tiempos["inicializacion"] += t1 - t0
        tiempos["orden_inicial"] += t2 - t1

    l = 2
    while l < n:
        ta = time.perf_counter()

        # Genera rangos basados en ordenamiento actual
        make_ranks(substr_rank, rank, n)

        tb = time.perf_counter()

        # 2. Prefix Doubling
        for i in range(n):
            substr_rank[i].left_rank = rank[i]
            substr_rank[i].right_rank = rank[i+l] if i+l < n else 0
            substr_rank[i].index = i

        l *= 2
        tc = time.perf_counter()

        # Reordenar con nuevos pares de rangos
        substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))
        td = time.perf_counter()

        if medir:
            tiempos["make_ranks"] += tb - ta
            tiempos["actualizar_rangos"] += tc - tb
            tiempos["ordenar_rondas"] += td - tc
            tiempos["rondas"] += 1

    # Extraer índices originales ya en su orden final
    SA = [substr_rank[i].index for i in range(n)]

    return SA


# Buscar todas las apariciones de un patrón usando el SA
def buscar_patron(T, SA, patron):
    if patron == "":
        return []
    n = len(T)
    m = len(patron)

    # Compara el patrón con el inicio de un sufijo
    def comparar(pos):
        for i in range(m):
            if pos + i >= n:
                return -1
            if T[pos + i] < patron[i]:
                return -1
            if T[pos + i] > patron[i]:
                return 1
        return 0
    # Primera búsqueda binaria: encontrar el primer sufijo
    # que sea igual o mayor que el patrón
    izquierda = 0
    derecha = len(SA)
    while izquierda < derecha:
        medio = (izquierda + derecha) // 2
        if comparar(SA[medio]) < 0:
            izquierda = medio + 1
        else:
            derecha = medio
    inicio = izquierda
    # Segunda búsqueda binaria: encontrar el primer sufijo
    # que sea estrictamente mayor que el patrón
    izquierda = 0
    derecha = len(SA)

    while izquierda < derecha:
        medio = (izquierda + derecha) // 2

        if comparar(SA[medio]) <= 0:
            izquierda = medio + 1
        else:
            derecha = medio
    fin = izquierda

    # Obtener todas las posiciones encontradas
    posiciones = SA[inicio:fin]
    # Ordenar las posiciones en el texto original
    posiciones.sort()
    return posiciones


def mb(nbytes):
    return nbytes / (1024 * 1024)


def medir_tiempo(func, T, reps):
    tiempos_totales = []
    etapas = None

    for r in range(reps):
        etapas_rep = {}
        inicio = time.perf_counter()
        SA = func(T, tiempos=etapas_rep)
        fin = time.perf_counter()

        tiempos_totales.append(fin - inicio)

        if r == 0:
            etapas = etapas_rep

        del SA

    return tiempos_totales, etapas


def medir_memoria(func, T):
    tracemalloc.start()
    base, _ = tracemalloc.get_traced_memory()
    tracemalloc.reset_peak()

    SA = func(T)

    actual, pico = tracemalloc.get_traced_memory()

    # Memoria que sigue ocupando el resultado SA
    resultado = mb(actual - base)

    # Liberar el arreglo de sufijos después de medir su memoria
    del SA

    tracemalloc.stop()

    return mb(pico - base), resultado


def mediciones(ruta_libro):
    try:
        with open(ruta_libro, "r", encoding="utf-8") as f:
            T = f.read()
    except FileNotFoundError:
        print(f"No se encontró el archivo: {ruta_libro}")
        return
    except UnicodeDecodeError:
        print(f"El archivo {ruta_libro} no es texto UTF-8 válido")
        return

    n = len(T)

    # Tiempo
    totales, etapas = medir_tiempo(suffix_array, T, REPS)
    promedio = sum(totales) / len(totales)

    print(f"  Tiempo promedio: {promedio:.3f} s  "
          f"(mín {min(totales):.3f} s, máx {max(totales):.3f} s, {REPS} repeticiones)")

    print(f"  Etapas ({etapas['rondas']} rondas):")
    for k, v in etapas.items():
        if k != "rondas":
            print(f"    {k:<18} {v:.3f} s  ({100 * v / totales[0]:.1f} %)")

    # Memoria
    pico, resultado = medir_memoria(suffix_array, T)

    print(f"  Memoria pico:     {pico:.1f} MB  ({pico * 1024 * 1024 / n:.0f} bytes por carácter)")
    print(f"  Memoria del SA:   {resultado:.1f} MB")

    # Probar búsqueda de un patrón
    patron = "art"

    # Construir el SA para realizar la búsqueda
    SA = suffix_array(T)

    posiciones = buscar_patron(T, SA, patron)

    print(f"\n  Búsqueda del patrón: '{patron}'")
    print(f"  Número de apariciones: {len(posiciones)}")
    print(f"  Posiciones: {posiciones}")
