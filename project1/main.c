#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#if 0
#define valloc malloc
#undef DEBUG
#endif

#ifdef DEBUG
FILE *dot_file;
#define dotdebug(...) fprintf(dot_file, __VA_ARGS__)
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dotdebug(...)
#define dprintf(...) 
#endif


#define min(a, b) ((a) < (b) ? (a) : (b))

typedef int16_t vertex_t;

struct edge {
    vertex_t tail; /* from */
    vertex_t head; /* to */

    int flow;
    int capacity;
    int cost;

    struct edge *next_tail; 
    struct edge *next_head; 
};

struct edge_data {
    int capacity;
    int cost;
};

struct network {
    int vertex_count; 
    int edge_count;

    int total_cost;

    vertex_t source; 
    vertex_t sink; 

    struct edge_data *graph; 
    struct edge *edges;

    /* iteration */
    int *flow;
    int *dist;
    vertex_t *parent;
};

#define fromto(u, v) ((u) * (network->vertex_count) + (v))
#define getdata(u, v) (network->graph[fromto((u), (v))])
#define getflow(u, v) (network->flow[fromto((u), (v))])

/* inline */ int
get_avail_flow(struct network *network, vertex_t tail, vertex_t head)
{
    if (getdata(tail, head).capacity != 0) {
        /* regular */
        return getdata(tail, head).capacity - getflow(tail, head);
    } else if (getdata(head, tail).capacity != 0) {
        /* residual */
        return getflow(head, tail);
    }
    return 0;
} 

vertex_t
bellman_ford(struct network *network)
{
    memset(network->parent, 0xff, sizeof(vertex_t) * network->vertex_count);
    memset(network->dist, 0x7f, sizeof(int) * network->vertex_count);

    network->dist[network->sink] = 0;

    vertex_t relaxed;
    for (int iter = 0; iter < network->vertex_count; iter++) {
        relaxed = -1;

        for (int e = 0; e < network->edge_count; e++) {
            struct edge *edge = &network->edges[e];
            int cost = getdata(edge->tail, edge->head).cost;
            
            bool can = get_avail_flow(network, edge->tail, edge->head) != 0;
            int d = network->dist[edge->tail] + cost;
            if (network->dist[edge->head] > d && can) {
                network->dist[edge->head] = d;
                network->parent[edge->head] = edge->tail;
                relaxed = edge->head;
            }

            can = get_avail_flow(network, edge->head, edge->tail) != 0;
            d = network->dist[edge->head] - cost;
            if (network->dist[edge->tail] > d && can) {
                network->dist[edge->tail] = d;
                network->parent[edge->tail] = edge->head;
                relaxed = edge->tail;
            }
            
        }
    }

    return relaxed;
}

void 
cancel_negative_cycles(struct network *network)
{
    vertex_t cycle_start;
    while ((cycle_start = bellman_ford(network)) != -1) {
        for (int i = 0; i < network->vertex_count; i++) {
            cycle_start = network->parent[cycle_start];
        }

        vertex_t vertex = cycle_start;
        int flow = INT_MAX;

        do {
            vertex_t tail = network->parent[vertex];
            vertex_t head = vertex;

            flow = min(flow, get_avail_flow(network, tail, head));

            vertex = network->parent[vertex];
        } while (vertex != cycle_start);

        vertex = cycle_start;
        do {
            vertex_t tail = network->parent[vertex];
            vertex_t head = vertex;

            if (getdata(tail, head).capacity != 0) {
                /* regular */
                getflow(tail, head) += flow;
                network->total_cost += flow * getdata(tail, head).cost;
            } else if (getdata(head, tail).capacity != 0) {
                /* residual */
                getflow(head, tail) -= flow;
                network->total_cost -= flow * getdata(head, tail).cost;
            }
            
            vertex = network->parent[vertex];
        } while (vertex != cycle_start);
    }
}

void 
pour_flow(struct network *network)
{
    vertex_t head = network->sink;
    int flow = network->dist[head];

    while (head != -1 && head != network->source) {
        vertex_t tail = network->parent[head];

        if (getdata(tail, head).capacity != 0) {
            /* regular */
            getflow(tail, head) += flow;
            network->total_cost += flow * getdata(tail, head).cost;
        } else if (getdata(head, tail).capacity != 0) {
            /* residual */
            getflow(head, tail) -= flow;
            network->total_cost -= flow * getdata(head, tail).cost;
        }
        

        head = tail;
    }
}

void
maxflow_dfs(struct network *network, vertex_t vertex)
{
    for (vertex_t neighbor = 0; neighbor < network->vertex_count; neighbor++) {
        if (network->parent[neighbor] != -1) {
            continue;
        }

        int avail = get_avail_flow(network, vertex, neighbor);

        if (avail == 0) {
            continue;
        }

        network->dist[neighbor] = min(avail, network->dist[vertex]);
        network->parent[neighbor] = vertex;
        
        maxflow_dfs(network, neighbor);

        if (network->parent[network->sink] != -1) {
            return;
        }
    }
}

void 
maxflow(struct network *network)
{

    int edges = network->vertex_count * network->vertex_count;
    memset(network->flow, 0, sizeof(int) * edges);
    network->total_cost = 0;

    do {
        memset(network->parent, 0xff, sizeof(vertex_t) * network->vertex_count);
        memset(network->dist, 0x7f, sizeof(int) * network->vertex_count);

        network->parent[network->source] = network->source;
        maxflow_dfs(network, network->source);
        pour_flow(network);
    } while (network->parent[network->sink] != -1);
}

#undef getflow
#undef getdata
#undef fromto


void
mincost(struct network *network)
{
    maxflow(network);
    dprintf("pre cost = %d\n", network->total_cost);
    cancel_negative_cycles(network);
    dprintf("post cost = %d\n", network->total_cost);
}


bool
solve_tournament()
{
    int player_count, budget;
    fscanf(stdin, "%d %d", &budget, &player_count);

    int game_count = ((player_count * (player_count - 1)) / 2);
    int game_offset = 0;
    int player_offset = game_offset + game_count;

    struct network network;
    network.vertex_count = game_count + player_count + 3;
    network.edge_count = game_count * 3 + player_count + 1;

    int edges = network.vertex_count * network.vertex_count;

    /* alloc once ? */ 
    network.graph = valloc(sizeof(struct edge_data) * edges);
    network.edges = valloc(sizeof(struct edge) * network.edge_count);
    network.flow = valloc(sizeof(int) * edges);

    network.dist = valloc(sizeof(int) * network.vertex_count);
    network.parent = valloc(sizeof(vertex_t) * network.vertex_count);

    memset(network.graph, 0, sizeof(struct edge_data) * edges);

#define fromto(u, v) ((u) * (network.vertex_count) + (v))
#define getdata(u, v) (network.graph[fromto((u), (v))])

    vertex_t limit_vertex = game_count + player_count;
    vertex_t source_vertex = game_count + player_count + 1;
    vertex_t sink_vertex = game_count + player_count + 2;

    network.sink = sink_vertex;
    network.source = source_vertex;

    struct edge *edge_i = network.edges;
    edge_i--;

    vertex_t player_vertex;

    dotdebug("digraph { //c \n");
    dotdebug("\trankdir=LR;\n");

    dotdebug("\t%d [label=source];\n", source_vertex);
    dotdebug("\t%d [label=limit];\n", limit_vertex);
    dotdebug("\t%d [label=sink];\n", sink_vertex);

    int player_a, player_b, winner, loser, bribe;
    for (int game_idx = 0; game_idx < game_count; game_idx++) {
        fscanf(stdin, "%d %d %d %d", 
               &player_a, &player_b, 
               &winner, &bribe);
        
        loser = winner == player_a ? player_b : player_a;

        vertex_t game_vertex = game_offset + game_idx;
        vertex_t winner_vertex = player_offset + winner;
        vertex_t loser_vertex = player_offset + loser;


        *(++edge_i) = (struct edge) { source_vertex, game_vertex };
        getdata(source_vertex, game_vertex) = (struct edge_data) { 1, 0 };
        
        *(++edge_i) = (struct edge) { game_vertex, winner_vertex };
        getdata(game_vertex, winner_vertex) = (struct edge_data) { 1, 0 };

        *(++edge_i) = (struct edge) { game_vertex, loser_vertex };
        getdata(game_vertex, loser_vertex) = (struct edge_data) { 1, bribe };


        dotdebug("\t%d [label=\"%d vs %d\"];\n",
                game_vertex, player_a, player_b);
    }   

    for (int player_idx = 0; player_idx < player_count; player_idx++) {
        player_vertex = player_offset + player_idx;

        dotdebug("\t%d [label=p%d]\n", player_vertex, player_idx);

        if (player_idx == 0) {
            *(++edge_i) = (struct edge) { player_vertex, sink_vertex };
            getdata(player_vertex, limit_vertex).cost = 0;

        } else {
            *(++edge_i) = (struct edge) { player_vertex, limit_vertex };
            getdata(player_vertex, limit_vertex).cost = 0;
        }        
    }

    player_vertex = player_offset + 0;
   // *(++edge_i) = (struct edge) { player_vertex, sink_vertex };
    *(++edge_i) = (struct edge) { limit_vertex, sink_vertex };


    dprintf("budget = %d\n", budget);
    bool found = false;
    for (int limit = player_count / 2; limit <= player_count - 1; limit++) {
        dprintf("limit = %d\n", limit);

        player_vertex = player_offset + 0;
        getdata(player_vertex, sink_vertex) = (struct edge_data) {limit, 0};
        getdata(limit_vertex, sink_vertex) = (struct edge_data) {game_count - limit, 0};

        for (int player_idx = 1; player_idx < player_count; player_idx++) {
            player_vertex = player_offset + player_idx;
            getdata(player_vertex, limit_vertex).capacity = limit;
        }

        mincost(&network);
        dprintf("spent = %d\n", network.total_cost);

        if (network.total_cost <= budget) {
            found = true;
            break;
        }
    }

#ifdef DEBUG
    for (int v = 0; v < network.vertex_count; v++) {
        for (int u = 0; u < network.vertex_count; u++) {
            struct edge_data *edge = &getdata(v, u);
            if (edge->capacity != 0) {
                dotdebug("\t%d -> %d [label=\"%d, %d, %d\"];\n", 
                         v, u, edge->capacity, edge->cost, 
                         network.flow[fromto(v, u)]);
            }
        }
    }
    dotdebug("}\n");
#endif


    free(network.dist);
    free(network.parent);
    free(network.flow);
    free(network.edges);
    free(network.graph);

#undef getdata
#undef fromto
    return found;
}

int
main(int argc, const char *argv[])
{
#ifdef DEBUG
    dot_file = fopen("out.dot", "w");
#endif

    int n;
    fscanf(stdin, "%d", &n);
    for (int i = 0; i < n; i++) {
        if (solve_tournament()) {
            printf("TAK\n");
        } else {
            printf("NIE\n");
        }
    }

#ifdef DEBUG
    fclose(dot_file);
#endif
    return 0;
}


