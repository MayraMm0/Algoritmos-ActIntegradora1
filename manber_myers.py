class SubstrRank:
    def __init__(self, left_rank=0, right_rank=0, index=0):
        self.left_rank = left_rank
        self.right_rank = right_rank
        self.index = index

def make_ranks(substr_rank, n):
    r = 1
    rank = [-1] * n
    rank[substr_rank[0].index] = r
    for i in range(n):
        if (substr_rank[i].left_rank != substr_rank[i-1].left_rank or
			substr_rank[i].right_rank != substr_rank[i-1].right_rank):
            r += 1
        rank[substr_rank[i].index] = r
    return rank

def suffix_array(T):
    n = len(T)
    substr_rank = []

    for i in range(n):
        substr_rank.append(SubstrRank(ord(T[i]), ord(T[i + 1]) if i < n-1 else 0, i))

    substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))

    l = 2
    while l < n:
        rank = make_ranks(substr_rank, n)

        for i in range(n):
            substr_rank[i].left_rank = rank[i]
            substr_rank[i].right_rank = rank[i+l] if i+l < n else 0
            substr_rank[i].index = i
        l *= 2

        substr_rank.sort(key=lambda sr: (sr.left_rank, sr.right_rank))

    SA = [substr_rank[i].index for i in range(n)]

    return SA

SA = suffix_array("mississippi")
print(SA)