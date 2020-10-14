from dimacs import *
from queue import PriorityQueue 
from dataclasses import dataclass, field
import sys
import os


@dataclass
class FindUnion:
    rank: int = 0
    parent: "FindUnion" = field(init=False, repr=False)

    def __post_init__(self):
        self.parent = self

    def find(self):
        if self.parent is not self:
            self.parent = self.parent.find()
        return self.parent

    def union(self, other: "FindUnion"):
        self = self.find()
        other = other.find()

        if self.rank > other.rank:
            other.parent = self
        else:
            if self.rank == other.rank:
                other.rank += 1
            self.parent = other.parent

    def same_set(self, other):
        return self.find() is other.find()

    def __repr__(self):
        return hex(id(self.find())).upper()
    

def turist_guide(V, L):
    unions = [FindUnion() for _ in range(V)]
    L.sort(key=lambda x: x[2], reverse=True)

    m = L[0][2]
    
    for (u, v, w) in L:        
        if not unions[u - 1].same_set(unions[v - 1]):
            unions[u - 1].union(unions[v - 1])
            m = min(m, w)

        if unions[0].same_set(unions[1]):
            return m;

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



