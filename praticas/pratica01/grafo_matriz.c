#include <stdio.h>
#include <stdlib.h>

#include "grafo_matriz.h"

static int vertice_valido(const GrafoMatriz *g, int v)
{
    return v >= 0 && v < g->n;
}

GrafoMatriz *criar_grafo_matriz(int n)
{
    if (n < 0) {
        return NULL;
    }

    GrafoMatriz *g = malloc(sizeof(GrafoMatriz));
    if (g == NULL) {
        return NULL;
    }

    g->n = n;
    g->adj = NULL;

    if (n == 0) {
        return g;
    }

    g->adj = malloc((size_t)n * sizeof(int *));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        g->adj[i] = calloc((size_t)n, sizeof(int));
        if (g->adj[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(g->adj[j]);
            }
            free(g->adj);
            free(g);
            return NULL;
        }
    }

    return g;
}

int inserir_aresta_matriz(GrafoMatriz *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    if (g->adj[u][v] == 1) {
        return -1;
    }

    g->adj[u][v] = 1;
    g->adj[v][u] = 1;

    return 0;
}

int remover_aresta_matriz(GrafoMatriz *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    if (g->adj[u][v] == 0) {
        return -1;
    }

    g->adj[u][v] = 0;
    g->adj[v][u] = 0;

    return 0;
}

int grau_matriz(const GrafoMatriz *g, int v)
{
    if (g == NULL || !vertice_valido(g, v)) {
        return -1;
    }

    int grau = 0;
    for (int i = 0; i < g->n; i++) {
        grau += g->adj[v][i];
    }

    return grau;
}

int sao_adjacentes_matriz(const GrafoMatriz *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    return g->adj[u][v];
}

void liberar_grafo_matriz(GrafoMatriz *g)
{
    if (g == NULL) {
        return;
    }

    if (g->adj != NULL) {
        for (int i = 0; i < g->n; i++) {
            free(g->adj[i]);
        }
        free(g->adj);
    }

    free(g);
}
