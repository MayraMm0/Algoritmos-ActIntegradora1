# Estructura auxiliar para ordenar por pares de rangos
class SubstrRank:
    def __init__(self, left_rank=0, right_rank=0, index=0):
        self.left_rank = left_rank    # Rank primera mitad del prefijo
        self.right_rank = right_rank  # Rank segunda mitad del prefijo
        self.index = index            # Índice original de sufijo en la cadena


# Convierte tuplas ordenadas en números/rangos
def make_ranks(substr_rank, n):
    r = 1
    rank = [-1] * n
    rank[substr_rank[0].index] = r
    
    for i in range(n):
        # Si dos elementos consecutivos tienen diferente left/right rank aumentamos rank
        if (substr_rank[i].left_rank != substr_rank[i-1].left_rank or
			substr_rank[i].right_rank != substr_rank[i-1].right_rank):
            r += 1
        rank[substr_rank[i].index] = r # Mapea ese rank al arreglo de rangos usando el índice original del sufijo
        
    return rank # Si dos consecutivos tienen mismo left/right rank se les asigna lo mismo


# Duplica en cada iteración la longitud del prefijo evaluado
def suffix_array(T):
    n = len(T)
    substr_rank = []

    # 1. L = 1: Inicializar con valores ascii
    for i in range(n):
        substr_rank.append(SubstrRank(ord(T[i]), ord(T[i + 1]) if i < n-1 else 0, i))

    # primer ordenamiento basado en primeros dos caracteres
    substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))

    l = 2
    while l < n:
        # Genera rangos basados en ordenamiento actual
        rank = make_ranks(substr_rank, n)

        # 2. Prefix Doubling
        # i consulta su rank acumulado y lo combina con el de l posiciones adelante
        for i in range(n):
            substr_rank[i].left_rank = rank[i]
            substr_rank[i].right_rank = rank[i+l] if i+l < n else 0
            substr_rank[i].index = i
        l *= 2

        # reordenar con nuevos pares de rangos
        substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))

    # Extraer índices originales ya en su orden final
    SA = [substr_rank[i].index for i in range(n)]

    return SA

SA = suffix_array("mississippi")
print(SA)