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
    if (n < 0) {
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

static int argumentos_validos(const GrafoListaPonderado *g, int u, int v, int peso)
{
    return g != NULL && vertice_valido(g, u) && vertice_valido(g, v) &&
           u != v && peso >= 0 && peso <= PESO_MAXIMO;
}

static int inserir_no(GrafoListaPonderado *g, int u, int v, int peso)
{
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

int inserir_arco_ponderado(GrafoListaPonderado *g, int u, int v, int peso)
{
    if (!argumentos_validos(g, u, v, peso) || contem_destino(g->adj[u], v)) {
        return -1;
    }

    return inserir_no(g, u, v, peso);
}

int inserir_aresta_ponderada(GrafoListaPonderado *g, int u, int v, int peso)
{
    if (!argumentos_validos(g, u, v, peso) ||
        contem_destino(g->adj[u], v) || contem_destino(g->adj[v], u)) {
        return -1;
    }

    if (inserir_no(g, u, v, peso) != 0) {
        return -1;
    }

    if (inserir_no(g, v, u, peso) != 0) {
        /* desfaz o primeiro arco para nao deixar a aresta pela metade */
        NoPonderado *no = g->adj[u];
        g->adj[u] = no->prox;
        free(no);
        return -1;
    }

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
