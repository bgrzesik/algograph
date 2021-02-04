
from dimacs import loadWeightedGraph, readSolution

import math


class Node:
    def __init__(self):
        self.out = set()
        self.order = -math.inf


def loadGraph(name):
    (V,L) = loadWeightedGraph(name)
    G = [Node() for i in range(V+1)]

    for u, v, c in L:
        G[u].out.add(v)
        G[v].out.add(u)

    return G


def LexBFS(G):
    visited = []
    lists = [{i for i in range(1, len(G))}]

    while len(visited) != len(G) - 1:
        vertex = lists[-1].pop()
        #print(f"WHILE vertex = {vertex - 1}")
        #print([{a - 1 for a in li}
        #        for li in lists[::-1]])
        #print()

        visited.append(vertex)

        new_lists = []

        for li in lists:
            #print("ADJECENT", {v - 1 for v in G[vertex].out})
            adj = li.intersection(G[vertex].out)
            #print("ADJ", {v - 1 for v in adj})
            not_adj = li.difference(G[vertex].out)
            #print("NON ADJ", {v - 1 for v in not_adj})

            if not_adj:
                new_lists.append(not_adj)

            if adj:
                new_lists.append(adj)

        lists = new_lists

    return visited


def rn(G, order, n):
    r = set()

    for i in range(n - 1, -1, -1):
        if order[i] in G[order[n]].out:
            r.add(order[i])

    return r


def max_clique(G):

    order = LexBFS(G)
    #print(order)

    rn_parent = set()
    r = 1

    for i in range(0, len(G) - 1):
        rn_i = rn(G, order, i)
        r = max(r, len(rn_i) + 1)

    return r


if False and __name__ == "__main__":
    TESTS = ["maxclique/clique5",
             "maxclique/cycle3",
             "maxclique/house",
             "maxclique/simple"]

    import os
    TESTS = os.listdir("maxclique")

    for name in TESTS:
        name = f"maxclique/{name}"
        correct = int(readSolution(name))
        print(name, correct)
        G = loadGraph(name)

        result = max_clique(G)
        if correct ^ result:
            print(f"FAILED {name}")
            print(f"EXPECTED {correct}")
            print(f"GOT {result}")

if False and __name__ == "__main__":
    import sys
    name = f"maxclique/{sys.argv[1]}"
    G = loadGraph(name)
    print([v - 1 for v in LexBFS(G)])

if True and __name__ == "__main__":
    import sys
    name = f"maxclique/{sys.argv[1]}"
    correct = int(readSolution(name))
    print(correct)
