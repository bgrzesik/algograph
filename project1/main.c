#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#if 1
#undef DDEEBBUUGG
#endif

#ifdef DDEEBBUUGG 
FILE *dot_file;
#define dotdebug(...) fprintf(dot_file, __VA_ARGS__)
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dotdebug(...)
#define dprintf(...) 
#endif


#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef int64_t vertex_t;
typedef int64_t unit_t;

struct edge {
    vertex_t tail; /* from */
    vertex_t head; /* to */

    unit_t flow;
    unit_t capacity;
    unit_t cost;
};

struct network {
    vertex_t vertex_count; 
    vertex_t edge_count;

    unit_t total_cost;

    vertex_t source; 
    vertex_t sink; 

    struct edge *edges;

    struct edge **source_player;
    struct edge **player_limit;
    struct edge *limit_sink;

    unit_t *avail;
    unit_t *dist;
    struct edge **parent;
};

vertex_t
bellman_ford(struct network *network)
{
    for (vertex_t v = 0; v < network->vertex_count; v++) {
        network->parent[v] = NULL;
        network->dist[v] = INT_MAX;
        network->avail[v] = INT_MAX;
    }

    network->dist[network->source] = 0;

    vertex_t relaxed;
    for (vertex_t iter = 0; iter < network->vertex_count; iter++) {
        relaxed = -1;

        for (int e = 0; e < network->edge_count; e++) {
            struct edge *edge = &network->edges[e];
            unit_t dist = network->dist[edge->tail];
            unit_t d = dist + edge->cost;

            unit_t avail = edge->capacity - edge->flow;
            if (dist != INT_MAX) {
                if (network->dist[edge->head] > d && avail > 0) {
                    network->dist[edge->head] = d;
                    network->parent[edge->head] = edge;
                    network->avail[edge->head] = min(avail, network->avail[edge->tail]);

                    relaxed = edge->head;
                }
            }

            avail = edge->flow;
            dist = network->dist[edge->head];
            d = dist - edge->cost;

            if (dist != INT_MAX) {
                if (network->dist[edge->tail] > d && avail > 0) {
                    network->dist[edge->tail] = d;
                    network->parent[edge->tail] = edge;
                    network->avail[edge->tail] = min(avail, network->avail[edge->head]);

                    relaxed = edge->tail;
                }
            }
        }

        if (relaxed == -1) {
            break;
        }
    }

#ifdef DDEEBBUUGG__
    for (vertex_t v = 0; v < network->vertex_count; v++) {
        dprintf("%lld ", network->dist[v]);
    }
    dprintf("\n");
#endif

    return relaxed;
}

void 
pour_flow(struct network *network)
{
    vertex_t head_vertex = network->sink;
    const unit_t flow = network->avail[network->sink];

    while (head_vertex != network->source) {
        struct edge *edge = network->parent[head_vertex];

        vertex_t neighbor = edge->head == head_vertex ? edge->tail : edge->head;
        int sgn = neighbor == edge->head ? -1 : 1;

        edge->flow += sgn * flow;
        network->total_cost += sgn * flow * edge->cost;


        head_vertex = neighbor;
    }
}

void 
maxflow(struct network *network)
{

    network->total_cost = 0;

    do {
        for (vertex_t v = 0; v < network->vertex_count; v++) {
            network->parent[v] = NULL;
            network->dist[v] = INT_MAX;
        }

        bellman_ford(network);
        if (network->parent[network->sink] != NULL) {
            pour_flow(network);
        }

    } while (network->parent[network->sink] != NULL);
}

struct edge *
add_edge(struct network *network, int64_t *cursor,
         vertex_t tail, vertex_t head,
         int capacity, int cost)
{
    struct edge *edge = &network->edges[*cursor];

    edge->tail = tail;
    edge->head = head;
    edge->flow = 0;
    edge->cost = cost;
    edge->capacity = capacity;

    (*cursor)++;

    return edge;
}

bool
solve_tournament()
{
    int64_t player_count, budget;
    fscanf(stdin, "%lld %lld", &budget, &player_count);

    if (player_count <= 1) {
        return true;
    }

    int64_t game_count = ((player_count * (player_count - 1)) / 2);
    int64_t max_points = INT_MIN;

    int64_t player_offset_l = 1;

    struct network network;
    memset(&network, 0, sizeof(struct network));

    network.vertex_count = 1 + player_count + 2;
    network.edge_count = player_count + game_count + player_count + 1;

    /* alloc once ? */ 
    network.edges           = valloc(sizeof(struct edge) * network.edge_count);
    network.source_player   = valloc(sizeof(struct edge *) * player_count);
    network.player_limit    = valloc(sizeof(struct edge *) * player_count);
    network.dist            = valloc(sizeof(unit_t) * network.vertex_count);
    network.avail            = valloc(sizeof(unit_t) * network.vertex_count);
    network.parent          = valloc(sizeof(struct edge *) * network.vertex_count);

#ifdef DDEEBBUUGG__
    dprintf("netwo\t = %p\n", &network);
    dprintf("edges\t = %p\n", network.edges);
    dprintf("srcp \t = %p\n", network.source_player);
    dprintf("plli \t = %p\n", network.player_limit);
    dprintf("dist \t = %p\n", network.dist);
    dprintf("pare \t = %p\n", network.parent);
#endif

    vertex_t source_vertex = 0;
    vertex_t limit_vertex = player_offset_l + player_count;
    vertex_t sink_vertex = player_offset_l + player_count + 1;

    network.sink = sink_vertex;
    network.source = source_vertex;

#ifdef DDEEBBUUGG
    dotdebug("digraph { //c \n");
    dotdebug("\trankdir=LR;\n");

    dotdebug("\t%lld [label=source];\n", source_vertex);
    dotdebug("\t%lld [label=limit];\n", limit_vertex);
    dotdebug("\t%lld [label=sink];\n", sink_vertex);
#endif

    int64_t cursor = 0;
    int64_t player_a, player_b, winner, loser, bribe;

    vertex_t player_vertex;
    for (int64_t player_idx = 0; player_idx < player_count; player_idx++) {
        player_vertex = player_offset_l + player_idx;

        network.source_player[player_idx] = add_edge(&network, &cursor,
                source_vertex, player_vertex, 0, 0);

        dotdebug("\t%lld [label=\"%lldl\"];\n",
                player_vertex, player_idx);
    }

    for (int64_t game_idx = 0; game_idx < game_count; game_idx++) {
        fscanf(stdin, "%lld %lld %lld %lld", 
               &player_a, &player_b, 
               &winner, &bribe);
        
        loser = winner == player_a ? player_b : player_a;

        vertex_t winner_vertex = player_offset_l + winner;
        vertex_t loser_vertex = player_offset_l + loser;

        network.source_player[winner]->capacity += 1;
        
        max_points = max(max_points, network.source_player[winner]->capacity);

        add_edge(&network, &cursor,
                 winner_vertex, loser_vertex, 1, bribe);

    }   

    for (int64_t player_idx = 0; player_idx < player_count; player_idx++) {
        player_vertex = player_offset_l + player_idx;

        dotdebug("\t%lld [label=p%lld]\n", player_vertex, player_idx);

        vertex_t head = player_idx == 0 ? sink_vertex : limit_vertex;

        network.player_limit[player_idx] = add_edge(
                &network, &cursor,
                player_vertex, head, 1, 0);

    }

    network.limit_sink = add_edge(&network, &cursor,
             limit_vertex, sink_vertex, 1, 0);

    bool found = false;

    dprintf("budget = %lld\n", budget);
    dprintf("max_points = %lld\n", max_points);
    dprintf("player_count = %lld\n", player_count);

    for (int64_t limit = player_count / 2; limit <= max_points; limit++) {
        dprintf("\n");
        dprintf("limit = %lld\n", limit);

        network.limit_sink->flow = 0;
        network.limit_sink->capacity = game_count - limit;

        for (int64_t e = 0; e < network.edge_count; e++) {
            network.edges[e].flow = 0;
        }

        for (int player_idx = 0; player_idx < player_count; player_idx++) {
            network.player_limit[player_idx]->capacity = limit;
        }

        maxflow(&network);
        dprintf("spent = %lld\n", network.total_cost);

        if (network.total_cost <= budget) {
            found = true;
            break;
        }
    }

#ifdef DDEEBBUUGG
    for (int64_t e = 0; e < network.edge_count; e++) {
        struct edge *edge = &network.edges[e];

        dotdebug("\t%lld -> %lld [label=\"%lld, %lld, %lld\"];\n", 
                 edge->tail, edge->head, 
                 edge->capacity, edge->cost, edge->flow);

    }
    dotdebug("}\n");
#endif
    free(network.avail);
    free(network.dist);
    free(network.parent);

    free(network.source_player);
    free(network.player_limit);
    free(network.edges);

    return found;
}

int
main(int argc, const char *argv[])
{
#ifdef DDEEBBUUGG
    dot_file = fopen("out.dot", "w");
#endif

    int n;
    fscanf(stdin, "%d", &n);
    for (int i = 0; i < n; i++) {
        if (solve_tournament()) {
            dprintf("==========================================\n");
            dprintf("||                  ");
            printf("TAK");
            dprintf("                 ||");
            printf("\n");
            dprintf("==========================================\n");
        } else {
            dprintf("==========================================\n");
            dprintf("||                  ");
            printf("NIE");
            dprintf("                 ||");
            printf("\n");
            dprintf("==========================================\n");
        }
    }

#ifdef DDEEBBUUGG
    fclose(dot_file);
#endif
    return 0;
}


