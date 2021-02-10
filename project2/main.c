#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>

#if 1
#undef DDEEBBUUGG
#endif

#if 0
#define LEX_DEBUG
#endif

#define valloc malloc

#ifdef DDEEBBUUGG 
FILE *dot_file;
#define dotdebug(...) fprintf(dot_file, __VA_ARGS__)
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define dotdebug(...)
#define dprintf(...) 
#endif

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define ALIGN_TO 16

typedef uint32_t vertex_t;

typedef uint32_t bitmask_t;

#define BITMASK_BITS (sizeof(bitmask_t) * 8)

#define _BIT(bit) \
    ((bitmask_t) (((bitmask_t) 1) << ((bit) % BITMASK_BITS)))

#define _BITMASK_ELEM(bitmask, bit) \
    bitmask[(bit) / BITMASK_BITS]

#define BITMASK_SET(bitmask, bit) \
    _BITMASK_ELEM(bitmask, bit) = _BITMASK_ELEM(bitmask, bit) | _BIT(bit)

#define BITMASK_HAS(bitmask, bit) \
    ((_BITMASK_ELEM(bitmask, bit) & _BIT(bit)) == _BIT(bit))

struct vertex {
    bitmask_t             *adj_mask;
    vertex_t               peo_pred_count;

} __attribute__ ((aligned (ALIGN_TO)));

struct set_node {
    vertex_t               vertex_idx;
    bool                   set_ends; 
    struct set_node       *next;

} __attribute__ ((aligned (ALIGN_TO)));

struct context {
    vertex_t               vertex_count;
    vertex_t               edge_count;
    size_t                 mask_len;

    struct vertex         *vertices;
    
    struct set_node       *set_list;

} __attribute__ ((aligned (ALIGN_TO)));

void
print_list(struct set_node *node)
{
#if 0
    dprintf("===================================\n");
    for (struct set_node *i = node; i != NULL; i = i->next) {
        dprintf("\n");
        dprintf("(%p)->vertex_idx = %lld\n", i, i->vertex_idx);
        dprintf("(%p)->set_next   = %d\n",   i, i->set_ends);
        dprintf("(%p)->next       = %p\n",   i, i->next);
        dprintf("\n");
    }
    dprintf("===================================\n");
#else
    dprintf("{ ");
    if (node == NULL) {
        dprintf("}");
    }
    for (struct set_node *i = node; i != NULL; i = i->next) {
        dprintf("%lld, ", i->vertex_idx);
        if (i->set_ends) {
            dprintf("}");

            if (i->next != NULL) {
                dprintf(", {");
            }
        }
    }
    dprintf("\n");
#endif
}

bool
is_adj(struct context *ctx, vertex_t a, vertex_t b) 
{
    return BITMASK_HAS(ctx->vertices[a].adj_mask, b);
}


void
lex_bfs_next(struct context *ctx, struct set_node **node)
{
    struct set_node *next = (*node)->next;

    struct set_node *head = NULL;
    struct set_node *tail = NULL;

#if defined(DDEEBBUUGG) && defined(LEX_DEBUG)
    dprintf("WHOLE vertex = %lld\n", (*node)->vertex_idx);
    print_list(*node);
#endif

    while (next != NULL) {
        struct set_node *head_adj     = NULL;
        struct set_node *tail_adj     = NULL;

        struct set_node *head_non_adj = NULL;
        struct set_node *tail_non_adj = NULL;

        /* split set */
        for (struct set_node *i = next; i != NULL; i = next) {
            next = i->next;
            i->next = NULL;

            struct set_node **append_to_head;
            struct set_node **append_to_tail;

            if (is_adj(ctx, (*node)->vertex_idx, i->vertex_idx)) {
                append_to_head = &head_adj;
                append_to_tail = &tail_adj;

                ctx->vertices[i->vertex_idx].peo_pred_count++;
            } else {
                append_to_head = &head_non_adj;
                append_to_tail = &tail_non_adj;
            }

            if (*append_to_head == NULL) {
                *append_to_head = i;
                *append_to_tail = i;
            } else {
                (*append_to_tail)->next = i;
                *append_to_tail = i;
            }

            if (i->set_ends) {
                break;
            }
        }

        if (tail_adj != NULL) {
            tail_adj->set_ends = true;
        }
        if (tail_non_adj != NULL) {
            tail_non_adj->set_ends = true;
        }

        
#if defined(DDEEBBUUGG) && defined(LEX_DEBUG)
        dprintf("ADJ");
        print_list(head_adj);
        dprintf("NON ADJ");
        print_list(head_non_adj);
#endif

        /* end of set */

        struct set_node **append_to = &head;
        if (head != NULL) {
            append_to = &tail->next;
        }

        if (head_adj != NULL) {
            *append_to = head_adj;
            tail_adj->next = head_non_adj;
        } else {
            *append_to = head_non_adj;
        }

        if (head_non_adj != NULL) {
            tail = tail_non_adj;
        } else if (head_adj != NULL) {
            tail = tail_adj;
        }

    }

    *node = head;

#if 0 && defined(DDEEBBUUGG) && defined(LEX_DEBUG)
    dprintf("AFTER \n");
    print_list(*node);
#endif
}

uint64_t
max_clique(struct context *ctx)
{
    struct set_node *node = &ctx->set_list[1];
    uint64_t max_len = ctx->vertices[node->vertex_idx].peo_pred_count + 1;

    lex_bfs_next(ctx, &node);

    while (node != NULL) {
        max_len = max(max_len, 
                      ctx->vertices[node->vertex_idx].peo_pred_count + 1);

        lex_bfs_next(ctx, &node);
    }

    return max_len;
}

vertex_t
read_num(void)
{
    char c;
    vertex_t a = 0;

    do {
        c = getchar();
    } while(!(c >= '0' && c <= '9'));

    do {
        a *= 10;
        a += c - '0';
        c = getchar();
    } while(c >= '0' && c <= '9'); 

    /*while (c = getchar(), c != '\n' && c != ' ') {
        if (c >= '0' && c <= '9') {
            a *= 10;
            a += c - '0';
        }
    }*/

    return a;
}

uint64_t
solve_game(void)
{
    struct context ctx;

    ctx.vertex_count = read_num();
    ctx.edge_count = read_num();
    ctx.mask_len = (ctx.vertex_count / BITMASK_BITS) + 1;
    ctx.vertex_count++;

/*
    dprintf("vertex_count = %lld\n", ctx.vertex_count);
    dprintf("edge_count = %lld\n", ctx.edge_count);
    dprintf("mask_len = %zu\n", ctx.mask_len);
*/
 
    /* checker counts the instructions sooo */
    /* TODO Improve locality (PC won't be affected?)? */

    size_t vertices_size = ctx.vertex_count                    * sizeof(struct vertex);
    size_t set_list_size = ctx.vertex_count                    * sizeof(struct set_node);
    size_t bitmask_size  = (ctx.vertex_count * ctx.mask_len)   * sizeof(bitmask_t);

    uint8_t *big_sector = valloc(vertices_size + set_list_size + bitmask_size);

    size_t offset = 0;

    ctx.vertices = (struct vertex *) &big_sector[offset];
    offset += vertices_size;

    ctx.set_list = (struct set_node *) &big_sector[offset];
    offset += set_list_size;

    bitmask_t *bitmasks = (bitmask_t *) &big_sector[offset];
    offset += bitmask_size;

    for (vertex_t i = 0; i < ctx.vertex_count; i++) {
        struct vertex *vertex = &ctx.vertices[i];

        vertex->peo_pred_count = 0;
        vertex->adj_mask = &bitmasks[ctx.mask_len * i];

        for (uint64_t m = 0; m < ctx.mask_len; m++) {
            vertex->adj_mask[m] = 0;
        }


        struct set_node *node = &ctx.set_list[i];    
        node->vertex_idx = i;

        if (i + 1 < ctx.vertex_count) {
            node->next = &ctx.set_list[i + 1];
            node->set_ends = false;
        } else {
            node->next = NULL;
            node->set_ends = true;
        }
    }

    for (vertex_t i = 0; i < ctx.edge_count; i++) {
        vertex_t a;
        vertex_t b;
        /* fscanf(stdin, "%d %d", &a, &b); */
        a = read_num();
        b = read_num();
        
        BITMASK_SET(ctx.vertices[a].adj_mask, b);
        BITMASK_SET(ctx.vertices[b].adj_mask, a);
    }

    uint64_t solution = max_clique(&ctx) - 1;
    solution = max(solution, 2);

    free(big_sector);

    return solution;
}



int
main(int argc, const char *argv[])
{
#ifdef DDEEBBUUGG
    dot_file = fopen("out.dot", "w");
#endif

    int game_count;
    /*fscanf(stdin, "%d", &game_count);*/

    game_count = read_num();

    /* dprintf("game_count = %d\n", game_count); */

    for (int game = 0; game < game_count; game++) {
        vertex_t solution = solve_game();
        printf("%d\n", solution);
    }


#ifdef DDEEBBUUGG
    fclose(dot_file);
#endif
    return 0;
}


