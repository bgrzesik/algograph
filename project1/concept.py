import math
# from ortools.linear_solver import pywraplp
from ortools.graph.pywrapgraph import SimpleMinCostFlow
import pydot


def tournament(player_count, budget, games_data):

    def cycle(x):
        solver = SimpleMinCostFlow()

        indices = [
                   * [f"{player_a} vs {player_b}" for player_a, player_b, _, _ in games_data],
                   * [f"p{player}" for player in range(player_count)],
                   "source", "target", "limit",]


        for player_a, player_b, winner, bribe in games_data:
            loser = player_b if player_a == winner else player_a
            
            solver.AddArcWithCapacityAndUnitCost(
                indices.index("source"),
                indices.index(f"{player_a} vs {player_b}"), 1, 0)

            solver.AddArcWithCapacityAndUnitCost(
                indices.index(f"{player_a} vs {player_b}"),
                indices.index(f"p{winner}"), 1, 0)

            solver.AddArcWithCapacityAndUnitCost(
                indices.index(f"{player_a} vs {player_b}"),
                indices.index(f"p{loser}"), 1, bribe)

        solver.AddArcWithCapacityAndUnitCost(
            indices.index("p0"),
            indices.index("target"), x, 0)

        for player in range(1, player_count):
            solver.AddArcWithCapacityAndUnitCost(
                indices.index(f"p{player}"),
                indices.index("limit"), x, 0)

        games_count = ((player_count - 1) * player_count) // 2
        solver.AddArcWithCapacityAndUnitCost(
            indices.index("limit"),
            indices.index("target"), games_count - x, 0)

        solver.SetNodeSupply(indices.index("source"), games_count)
        solver.SetNodeSupply(indices.index("target"), -games_count)

        status = solver.SolveMaxFlowWithMinCost()


        if status != solver.OPTIMAL:
            return False

        if games_count != solver.MaximumFlow():
            return False

        if solver.OptimalCost() > budget:
            return False

        # print(solver.MaximumFlow())
        # print(solver.OptimalCost())

        print("digraph { // ortools")
        print("\trankdir=LR;")
        for i, v in enumerate(indices):
            print(f'\t{i} [label="{v}"];')

        for i in range(solver.NumArcs()):
            tail = solver.Tail(i)
            head = solver.Head(i)

            capacity = solver.Capacity(i)
            cost = solver.UnitCost(i)
            flow = solver.Flow(i)

            print(f'\t{tail} -> {head} [label="{capacity}, {cost}, {flow}"]')

        print("}")

        print("cost = ", solver.OptimalCost())

        return True

    for x in range(0, player_count):
        if cycle(x):
            print(x)
            return True

    return False


def parse_data(games_data):
    games_data = games_data.strip()
    return [[int(v) for v in line.strip().split(" ")]
            for line in games_data.splitlines()]


if __name__ == "__main__":
    games_data = [[0, 1, 0, 0],
                  [1, 2, 2, 5],
                  [0, 2, 2, 3]]

    player_count = 3
    print(tournament(player_count, 5, games_data))

    games_data = parse_data("""
        0 1 1 5
        0 3 0 0
        0 2 2 14
        1 2 2 3
        1 3 1 8
        2 3 2 1
    """)
    print(tournament(4, 10, games_data))

    games_data = parse_data("""
        0 1 1 5
        0 3 0 0
        0 2 2 14
        1 2 2 3
        1 3 1 8
        2 3 2 1
    """)
    print(tournament(4, 1, games_data))
