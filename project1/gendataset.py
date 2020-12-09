import sys
import random

n = int(sys.argv[1])

print("B")
print(n)


for i in range(n):
    for j in range(i + 1, n):
        wins = random.choice((i, j))
        bribe = random.randint(1, 15)
        print(i, j, wins, bribe)





