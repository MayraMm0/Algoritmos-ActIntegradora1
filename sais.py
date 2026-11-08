import sys
import time
import tracemalloc

# Aumentar límite de recursión para archivos txt más grandes
sys.setrecursionlimit(300000)

# Divide suffix array en buckets para cada caracter
def getBuckets(T):
    # En lugar de diccionarios, usar un arreglo directo según el valor máximo del alfabeto
    max_val = max(T) if T else 0
    count = [0] * (max_val +1)
    buckets = {} # estructura: buckets[char] = (inicio_bucket, fin_bucket)
    
    for c in T:
        count[c] += 1
        
    start = 0
    
    for c in range(max_val+1):
        if count[c] > 0:
            buckets[c] = (start, start + count[c])
            start += count[c]
            
    return buckets

# Función auxiliar que compara dos subcademas LMS
# compara elemento por elemento sin crear sub-listas (mejor para ram)
def lms_equal(T, t, i, j, LMS):
    # Si son el mismo índice, son iguales
    if i == j:
        return True
    
    # Manejo seguro por si algún índice es la posición terminal
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
    # 1. clasificación de sufijos entre S y L
    # S -> si T[i-1] < T[i]
    # T -> si T[i-1] > T[i]
    t = ["_"] * len(T)
    
    t[len(T) - 1] = "S"
    for i in range(len(T) - 1, 0, -1): # Recorre de der a izq
        if T[i-1] == T[i]:
            t[i - 1] = t[i]
        else:
            t[i - 1] = "S" if T[i-1] < T[i] else "L"
    
    # 2. Calculo de cubetas
    buckets = getBuckets(T)

    # 3. Identificar sufijos LMS
    # y primera ordenación iducida (para LMS) y colocar sufijos al final de sus buckets correspondientes
    count = {}
    SA = [-1] * len(T)
    LMS = {} # Left Most S-type, cuando es S y el de la izq es L
    end = None
    for i in range(len(T) - 1, 0, -1):
        if t[i] == "S" and t[i - 1] == "L": # Left Most S-type, cuando es S y el de la izq es L
            revoffset = count[T[i]] = count.get(T[i], 0) + 1
            SA[buckets[T[i]][1] - revoffset] = i
            if end is not None:
                LMS[i] = end
            end = i

    if end is not None:
        LMS[len(T) - 1 ] = end
    else: 
        LMS[len(T) - 1] = len(T) - 1
    
    # Inducir sufijos L (recorremos de izq a der)
    count = {} # Reiniciar punteros a inicio de cada bucket
    for i in range(len(T)):
        if SA[i] > 0: # Valida que exista un sufijo anterior
            j = SA[i] - 1 #sufijo inmediatamente anterior
            if t[j] == "L": # si ese anterior es (L)
                symbol = T[j] # tipo L
                offset = count.get(symbol, 0)
                SA[buckets[symbol][0] + offset] = j # Colocar j en puntero actual de su bucket
                count[symbol] = offset + 1 # avanzar el puntero de la bucket +1

    # Inducir sufijos S (recorremos de der a izq)
    count = {} # Reiniciar punteros al final de cada bucket
    for i in range(len(T) - 1, -1, -1):
        if SA[i] > 0: # Validar que exista un sufijo anterior
            j = SA[i] - 1
            if t[j] == "S": # si ese anterior es (S)
                symbol = T[j]  # tipo S
                revoffset = count[symbol] = count.get(symbol, 0) + 1
                SA[buckets[symbol][1] - revoffset] = j # colocar j en puntero actual de su bucket
                # Retroceder el puntero de esa cubeta se maneja restando el revoffset del final del bucket

    # 4. Renombrar subcadenas LMS y llamada recursiva
    namesp = [-1] * len(T) # Asigna id según orden en que quedaron en SA provicional
    name = 0 
    prev = None
    for i in range(len(SA)):
        if SA[i] > 0 and t[SA[i]] == "S" and t[SA[i] - 1] == "L":
            idx = SA[i]
            if prev is not None and not lms_equal(T, t, prev, idx, LMS):
                name += 1 # Si dos no son iguales incremente name
            prev = idx
            namesp[idx] = name # Si dos consecutivas son identicas se asigna mismo name

    # Crear un nuevo texto reducido usando names como caracteres y SApIdx como indices de LMS
    names = []
    SApIdx = []
    for i in range(len(T)):
        if namesp[i] != -1:
            names.append(namesp[i])
            SApIdx.append(i)

    # si hay nombres duplicados resuelve recursivamnte
    if name < len(names) - 1:
        # Resolver recursivamente y obtener el Suffix Array de la cadena reducida
        sub_SA = sais(names)
    else:
        # Base de la recursión: si todos los nombres son únicos, construye SA directo
        sub_SA = [-1] * len(names)
        for i, n in enumerate(names):
            sub_SA[n] = i
    
    # Colocación final de LMS y induced sort definitivo
    SA = [-1] * len(T) # re-inicializar SA
    count = {}

    # colocar LMS en su orden real exacto al fondo de las buckets
    for i in range(len(sub_SA) - 1, -1, -1):
        pos = SApIdx[sub_SA[i]]
        revoffset = count[T[pos]] = count.get(T[pos], 0) + 1 
        SA[buckets[T[pos]][1] - revoffset] = pos
    

    # Otra vez induce con conteo final exacto
    # inducir Sufijos L
    count = {}
    for i in range(len(T)):
        if SA[i] > 0:
            j = SA[i] - 1
            if t[j] == "L":
                symbol = T[j]
                offset = count.get(symbol, 0)
                SA[buckets[symbol][0] + offset] = j
                count[symbol] = offset + 1

    # Inducir sufijos S
    count = {}
    for i in range(len(T) - 1, -1, -1):
        if SA[i] > 0:
            j = SA[i] - 1
            if t[j] == "S":
                symbol = T[j]
                revoffset = count[symbol] = count.get(symbol, 0) + 1
                SA[buckets[symbol][1] - revoffset] = j

    return SA


def procesar_libro(ruta_libro):
    try:
        #1. Leer archivo
        with open(ruta_libro, 'r', encoding='utf-8') as l:
            texto = l.read()
    except FileNotFoundError:
        print("No se encontró el archivo")
        return
        
    # añadir símbolo terminal '$'
    if not texto.endswith('$'):
        texto += '$'
    
    T = [ord(c) for c in texto]
    
    # 2. Iniciar perfilado de memoria y tiempo
    tracemalloc.start()
    tiempo_inicio = time.perf_counter()
    
    #ejecución de algoritmo sais
    SA = sais(T)
    
    tiempo_fin = time.perf_counter()
    current, peak = tracemalloc.get_traced_memory()
    tracemalloc.stop()
    
    # 3. Sacar métricas
    tiempo_total = tiempo_fin - tiempo_inicio
    memoria_pico_mb = peak / (1024 * 1024) # megabyte
    
    print(f"Libro: {ruta_libro}")
    print(f"  - Caracteres procesados: {len(T)}")
    print(f"  - Tiempo de ejecución: {tiempo_total:.4f} segundos")
    print(f"  - Memoria pico alcanzada: {memoria_pico_mb:.2f} MB")
    print("-" * 50)
    #print(f"SA: {SA}")