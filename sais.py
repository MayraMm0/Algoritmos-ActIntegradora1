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
    # clasifica entre S y L
    # S -> si T[i-1] < T[i]
    # T -> si T[i-1] > T[i]
    t = ["_"] * len(T)
    
    t[len(T) - 1] = "S"
    for i in range(len(T) - 1, 0, -1): # Recorre de der a izq
        if T[i-1] == T[i]:
            t[i - 1] = t[i]
        else:
            t[i - 1] = "S" if T[i-1] < T[i] else "L"
    
    buckets = getBuckets(T)

    # 1. Colocar sufijos al final de sus buckets correspondientes
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
    
    # 2. Inducir sufijos L (recorremos de izq a der)
    count = {}
    for i in range(len(T)):
        if SA[i] >= 0:
            if t[SA[i] - 1] == "L":
                symbol = T[SA[i] - 1] # tipo L, lo mueve al frente de su bucket
                offset = count.get(symbol, 0)
                SA[buckets[symbol][0] + offset] = SA[i] - 1
                count[symbol] = offset + 1

    # 3. Inducir sufijos S (recorremos de der a izq)
    count = {}
    for i in range(len(T) - 1, 0, -1):
        if SA[i] > 0:
            if t[SA[i] - 1] == "S":
                symbol = T[SA[i] - 1]  # tipo S, lo mueve al fondo de su bucket
                revoffset = count[symbol] = count.get(symbol, 0) + 1
                SA[buckets[symbol][1] - revoffset] = SA[i] - 1

    # Renombrar subcadenas LMS y llamada recursiva
    namesp = [-1] * len(T) # Asigna id según orden en que quedaron en SA provicional
    name = 0 
    prev = None
    for i in range(len(SA)):
        if t[SA[i]] == "S" and t[SA[i] - 1] == "L":
            if prev is not None and T[SA[prev]:LMS[SA[prev]]] != T[SA[i]:LMS[SA[i]]]:
                name += 1 # Si dos no son iguales incremente name
            prev = i
            namesp[SA[i]] = name

    names = []
    SApIdx = []
    for i in range(len(T)):
        if namesp[i] != -1:
            names.append(namesp[i])
            SApIdx.append(i)

    # si hay nombres duplicados resuelve recursivamnte
    if name < len(names) - 1:
        names = sais(names)

    names = list(reversed(names))

    # Colocación final de LMS y Induced Sort definitivo
    # Re-inicializar SA
    SA = [-1] * len(T)
    count = {}
    #Colorar LMS en su orden real al fondo de las buckets
    for i in range(len(names)):
        pos = SApIdx[names[i]]
        revoffset = count[T[pos]] = count.get(T[pos], 0) + 1
        SA[buckets[T[pos]][1] - revoffset] = pos

    # inducir Sufijos L
    count = {}
    for i in range(len(T)):
        if SA[i] >= 0:
            if t[SA[i] - 1] == "L":
                symbol = T[SA[i] - 1]
                offset = count.get(symbol, 0)
                SA[buckets[symbol][0] + offset] = SA[i] - 1
                count[symbol] = offset + 1

    # Inducir sufijos S
    count = {}
    for i in range(len(T) - 1, 0, -1):
        if SA[i] > 0:
            if t[SA[i] - 1] == "S":
                symbol = T[SA[i] - 1]
                revoffset = count[symbol] = count.get(symbol, 0) + 1
                SA[buckets[symbol][1] - revoffset] = SA[i] - 1

    return SA

string = "GTCCCGATGTCATGTCAGGA$"
T = [ord(c) for c in string]
SA = sais(T)
print(SA)