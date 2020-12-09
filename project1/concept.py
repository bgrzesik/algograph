import math
# from ortools.linear_solver import pywraplp
from ortools.graph.pywrapgraph import SimpleMinCostFlow
import pydot
import imgcat


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


        #dot = pydot.Dot(rankdir="LR")
        dot = None

        if dot:
            for i, v in enumerate(indices):
                dot.add_node(pydot.Node(i, label=v))

        for i in range(solver.NumArcs()):
            tail = solver.Tail(i)
            head = solver.Head(i)

            capacity = solver.Capacity(i)
            cost = solver.UnitCost(i)
            flow = solver.Flow(i)

            if dot:
                dot.add_edge(pydot.Edge(tail, head, 
                                        label=f"{capacity}, {cost}, {flow}"))

        if dot:
            if not game_count > 50:
                imgcat.imgcat(dot.create_png())
            elif not game_count > 200:
                dot.write_png(f"{player_count}_big_boy.png")

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
    with open("input.txt", "r") as f:
        game_count = int(f.readline())

        for game in range(game_count):
            budget = int(f.readline())
            player_count = int(f.readline())
            game_count = (player_count * (player_count - 1)) // 2
            data = "".join([f.readline() for _ in range(game_count)])

            if game_count == 0:
                print(True)
                continue

            print(tournament(player_count, budget, parse_data(data)))

            
