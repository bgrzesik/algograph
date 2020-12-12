#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#if 1
#undef DDEEBBUUGG
#endif
#define valloc malloc

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

typedef int32_t vertex_t;
typedef int32_t player_idx_t;
typedef int32_t unit_t;

#define UNIT_MIN 0x80000000
#define UNIT_MAX 0x7fffffff
#define ALIGN_TO 16

struct edge {
    vertex_t tail; /* from */
    vertex_t head; /* to */

    unit_t flow;
    unit_t capacity;
    unit_t cost;

    struct edge *next_head;
    struct edge *next_tail;
} __attribute__ ((aligned (ALIGN_TO)));

struct vertex {
    unit_t avail;
    unit_t dist;
    struct edge *parent;

    struct edge *adj_head;
    struct edge *adj_tail;
};

struct vertex_queue {
    vertex_t cursor;
    vertex_t end;

    /* TODO uint64_t *in_queue;  */ 
    bool *in_queue;
    vertex_t *queue;
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

    struct vertex *vertices;

    struct vertex_queue queue;

} __attribute__ ((aligned (ALIGN_TO)));

static inline void
vertex_queue_put(struct network *network, vertex_t vertex)
{
    struct vertex_queue *queue = &network->queue;
    
    if (queue->in_queue[vertex]) {
        return;
    }

    if (queue->cursor == -1) {
        queue->cursor = 0;
        queue->end = 0;
        queue->queue[0] = vertex;
    } else {
        queue->end = (queue->end + 1) % network->vertex_count;
        queue->queue[queue->end] = vertex;
    }

    queue->in_queue[vertex] = true;
}

static inline bool
vertex_queue_empty(struct network *network)
{
    struct vertex_queue *queue = &network->queue;
    return queue->cursor == -1;
}

static inline vertex_t
vertex_queue_pop(struct network *network) 
{
    struct vertex_queue *queue = &network->queue;

    vertex_t vertex = queue->queue[queue->cursor];
    if (queue->cursor == queue->end) {
        queue->cursor = -1;
    } else {
        queue->cursor = (queue->cursor + 1) % network->vertex_count;
    }

    queue->in_queue[vertex] = false;
    return vertex;
}

static inline void 
relax(struct network *network, struct edge *edge)
{
    unit_t dist = network->vertices[edge->tail].dist;
    unit_t d;
    unit_t avail;

    struct vertex *head = &network->vertices[edge->head];
    struct vertex *tail = &network->vertices[edge->tail];

    if (dist != UNIT_MAX) {
        d = dist + edge->cost;
        avail = edge->capacity - edge->flow;

        if (head->dist > d && avail > 0) {
            head->dist = d;
            head->parent = edge;
            head->avail = min(avail, tail->avail);

            vertex_queue_put(network, edge->head);
        }
    }

    dist = network->vertices[edge->head].dist;
    if (dist != UNIT_MAX) {
        d = dist - edge->cost;
        avail = edge->flow;

        if (tail->dist > d && avail > 0) {
            tail->dist = d;
            tail->parent = edge;
            tail->avail = min(avail, head->avail);

            vertex_queue_put(network, edge->tail);
        }
    }
}

static void
bellman_ford(struct network *network)
{
    for (vertex_t v = 0; v < network->vertex_count; v++) {
        /* network->parent[v] = NULL; */
        network->vertices[v].dist = UNIT_MAX;
        network->queue.in_queue[v] = false;
        /* network->avail[v] = UNIT_MAX; */
    }

    network->vertices[network->source].dist = 0;
    network->vertices[network->sink].parent = NULL;
    network->vertices[network->source].avail = UNIT_MAX;

    network->queue.cursor = -1;
    network->queue.end = 0;
    vertex_queue_put(network, network->source);


    while (!vertex_queue_empty(network)) {
        vertex_t index = vertex_queue_pop(network);
        struct vertex *vertex = &network->vertices[index];


        for (struct edge *edge = vertex->adj_tail; edge != NULL; 
                edge = edge->next_tail) {
            relax(network, edge);
        }

        for (struct edge *edge = vertex->adj_head; edge != NULL; 
                edge = edge->next_head) {
            relax(network, edge);
        }
    }

#ifdef DDEEBBUUGG__
    for (vertex_t v = 0; v < network->vertex_count; v++) {
        dprintf("%lld ", network->dist[v]);
    }
    dprintf("\n");
#endif
}

void 
pour_flow(struct network *network)
{
    vertex_t head_vertex = network->sink;
    const unit_t flow = network->vertices[network->sink].avail;

    while (head_vertex != network->source) {
        struct edge *edge = network->vertices[head_vertex].parent;

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
        bellman_ford(network);
        if (network->vertices[network->sink].parent != NULL) {
            pour_flow(network);
        }

    } while (network->vertices[network->sink].parent != NULL);
}

static struct edge *
add_edge(struct network *network, vertex_t *cursor,
         vertex_t tail, vertex_t head,
         unit_t capacity, unit_t cost)
{
    struct edge *edge = &network->edges[*cursor];

    edge->tail = tail;
    edge->head = head;
    edge->flow = 0;
    edge->cost = cost;
    edge->capacity = capacity;

    edge->next_tail = network->vertices[tail].adj_tail;
    network->vertices[tail].adj_tail = edge;

    edge->next_head = network->vertices[head].adj_head;
    network->vertices[head].adj_head = edge;

    (*cursor)++;

    return edge;
}

bool
solve_tournament()
{
    player_idx_t player_count;
    unit_t budget;
    fscanf(stdin, "%d %d", &budget, &player_count);

    if (player_count <= 1) {
        return true;
    }

    vertex_t game_count = ((player_count * (player_count - 1)) / 2);
    unit_t max_points = UNIT_MIN;

    vertex_t player_offset_l = 1;

    struct network network;
    /* memset(&network, 0, sizeof(struct network)); */

    network.vertex_count = 1 + player_count + 2;
    network.edge_count = player_count + game_count + player_count + 1;

    /* alloc once ? */ 
    network.edges           = valloc(sizeof(struct edge) * network.edge_count);
    network.vertices        = valloc(sizeof(struct vertex) * network.vertex_count);
    network.source_player   = valloc(sizeof(struct edge *) * player_count);
    network.player_limit    = valloc(sizeof(struct edge *) * player_count);

    network.queue.in_queue  = valloc(sizeof(bool) * network.vertex_count);
    network.queue.queue     = valloc(sizeof(vertex_t) * network.vertex_count);

    for (vertex_t vertex = 0; vertex < network.vertex_count; vertex++) {
        network.vertices[vertex].adj_head = NULL;
        network.vertices[vertex].adj_tail = NULL;
    }

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

    vertex_t cursor = 0;
    player_idx_t player_a, player_b, winner, loser;
    unit_t bribe;

    vertex_t player_vertex;
    for (player_idx_t player_idx = 0; player_idx < player_count; player_idx++) {
        player_vertex = player_offset_l + player_idx;

        network.source_player[player_idx] = add_edge(&network, &cursor,
                source_vertex, player_vertex, 0, 0);

        dotdebug("\t%lld [label=\"%lldl\"];\n",
                player_vertex, player_idx);
    }

    for (vertex_t game_idx = 0; game_idx < game_count; game_idx++) {
        fscanf(stdin, "%d %d %d %d", 
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

    for (player_idx_t player_idx = 0; player_idx < player_count; player_idx++) {
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

    for (unit_t limit = player_count / 2; limit <= max_points; limit++) {
        dprintf("\n");
        dprintf("limit = %lld\n", limit);

        network.limit_sink->flow = 0;
        network.limit_sink->capacity = game_count - limit;

        for (vertex_t e = 0; e < network.edge_count; e++) {
            network.edges[e].flow = 0;
        }

        for (player_idx_t player_idx = 0; player_idx < player_count; player_idx++) {
            unit_t flow = min(network.source_player[player_idx]->capacity,
                           limit);
            
            flow = min(flow, network.limit_sink->capacity - network.limit_sink->flow);
            
            if (player_idx != 0) {
                network.limit_sink->flow += flow;
            }

            network.source_player[player_idx]->flow = flow;
            network.player_limit[player_idx]->flow = flow;
            
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
    for (vertex_t e = 0; e < network.edge_count; e++) {
        struct edge *edge = &network.edges[e];

        dotdebug("\t%lld -> %lld [label=\"%lld, %lld, %lld\"];\n", 
                 edge->tail, edge->head, 
                 edge->capacity, edge->cost, edge->flow);

    }
    dotdebug("}\n");
#endif

    free(network.source_player);
    free(network.player_limit);
    free(network.edges);
    free(network.vertices);

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


