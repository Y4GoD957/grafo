#include <stdlib.h>

#include "grafo_ponderado.h"

static int vertice_valido(const GrafoListaPonderado *g, int v)
{
    return v >= 0 && v < g->n;
}

static int contem_destino(const NoPonderado *lista, int destino)
{
    for (const NoPonderado *atual = lista; atual != NULL; atual = atual->prox) {
        if (atual->destino == destino) {
            return 1;
        }
    }

    return 0;
}

GrafoListaPonderado *criar_grafo_ponderado(int n)
{
    if (n < 0 || n > MAX_VERTICES) {
        return NULL;
    }

    GrafoListaPonderado *g = malloc(sizeof(GrafoListaPonderado));
    if (g == NULL) {
        return NULL;
    }

    g->n = n;
    g->adj = NULL;

    if (n > 0) {
        g->adj = calloc((size_t)n, sizeof(NoPonderado *));
        if (g->adj == NULL) {
            free(g);
            return NULL;
        }
    }

    return g;
}

int inserir_arco_ponderado(GrafoListaPonderado *g, int u, int v, int peso)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v) ||
        peso < -PESO_MAXIMO || peso > PESO_MAXIMO ||
        contem_destino(g->adj[u], v)) {
        return -1;
    }

    NoPonderado *no = malloc(sizeof(NoPonderado));
    if (no == NULL) {
        return -1;
    }

    no->destino = v;
    no->peso = peso;
    no->prox = g->adj[u];
    g->adj[u] = no;

    return 0;
}

void liberar_grafo_ponderado(GrafoListaPonderado *g)
{
    if (g == NULL) {
        return;
    }

    for (int i = 0; i < g->n; i++) {
        NoPonderado *atual = g->adj[i];
        while (atual != NULL) {
            NoPonderado *prox = atual->prox;
            free(atual);
            atual = prox;
        }
    }

    free(g->adj);
    free(g);
}
