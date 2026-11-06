# Divide suffix array en buckets para cada caracter
def getBuckets(T):
    count = {}
    buckets = {} # estructura: buckets[char] = (inicio_bucket, fin_bucket)
    for c in T:
        count[c] = count.get(c, 0) + 1
    start = 0
    for c in sorted(count.keys()):
        buckets[c] = (start, start + count[c])
        start += count[c]
    return buckets


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
            if prev is not None and T[prev:LMS[prev]] != T[idx:LMS[idx]]:
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

string = "banana$"
T = [ord(c) for c in string]
SA = sais(T)
print(SA)