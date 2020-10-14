from dimacs import *
from queue import PriorityQueue 
from dataclasses import dataclass, field
import math
import sys
import os


def _bfs_visit(adj, max_weight):
    queue = [(0, math.inf)]
    visited = [False] * len(adj)

    while len(queue) > 0 and not visited[1]:
        u, m = queue.pop(0)
        if visited[u]:
            continue
        
        visited[u] = True

        for v, w in adj[u]:
            if w < max_weight or visited[v]:
                continue
            
            queue.append((v, min(w, m)))

            if v == 1:
                return True, min(m, w)

    return False, -math.inf

def turist_guide(V, L):
    left = 0
    right = len(L) - 1

    L.sort(key=lambda x: x[2])
    adj = [[] for _ in range(V)]

    for (u, v, w) in L:
        adj[u - 1].append((v - 1, w))
        adj[v - 1].append((u - 1, w))

    result = -math.inf

    while True:
        if left >= right:
            return result
        
        mid = (left + right ) // 2
        max_weight = L[mid][2]

        found, m = _bfs_visit(adj, max_weight)
        
        result = max(result, m)

        if found:
            left = mid + 1
        else:
            right = mid

    return None
    

def _test(graph):
    solution = readSolution(f"graphs/{graph}")
    (V, L) = loadWeightedGraph(f"graphs/{graph}")
    result = turist_guide(V, L)

    print(f"graph={graph}", f"solution={solution}", f"result={result}")

    assert int(solution) == result 

if __name__ == "__main__":
    graph = "g1"
    if len(sys.argv) > 1:
        _test(graph)
    else:
        for graph in os.listdir("graphs/"):
            _test(graph)
    # _test("g1")


