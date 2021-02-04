import os
import sys
import time
from dimacs import loadWeightedGraph

def main():
    graphs = os.listdir("maxclique")

    #graphs = ["game", "cycle3", "house"]
    #graphs = ["interval-rnd50"]
    graphs = [sys.argv[1]]

    print(len(graphs))
    for name in graphs:
        #print(name, file=sys.stderr)
        V, L = loadWeightedGraph("maxclique/" + name)

        print(V, len(L))

        for u, v, _ in L:
            print(u, v)


if __name__ == "__main__":
    main()

