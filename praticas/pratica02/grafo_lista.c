#include <stdio.h>
#include <stdlib.h>

#include "grafo_lista.h"

static int vertice_valido(const GrafoLista *g, int v)
{
    return v >= 0 && v < g->n;
}

static No *criar_no(int destino)
{
    No *no = malloc(sizeof(No));
    if (no == NULL) {
        return NULL;
    }

    no->destino = destino;
    no->prox = NULL;

    return no;
}

static int contem_destino(const No *lista, int destino)
{
    for (const No *atual = lista; atual != NULL; atual = atual->prox) {
        if (atual->destino == destino) {
            return 1;
        }
    }

    return 0;
}

/* Remove o primeiro no com destino == alvo da lista de g->adj[origem].
 * Retorna 0 se removeu, -1 se nao encontrou o no. */
static int remover_no(GrafoLista *g, int origem, int alvo)
{
    No *atual = g->adj[origem];
    No *anterior = NULL;

    while (atual != NULL) {
        if (atual->destino == alvo) {
            if (anterior == NULL) {
                g->adj[origem] = atual->prox;
            } else {
                anterior->prox = atual->prox;
            }
            free(atual);
            return 0;
        }
        anterior = atual;
        atual = atual->prox;
    }

    return -1;
}

GrafoLista *criar_grafo_lista(int n)
{
    if (n < 0) {
        return NULL;
    }

    GrafoLista *g = malloc(sizeof(GrafoLista));
    if (g == NULL) {
        return NULL;
    }

    g->n = n;
    g->adj = NULL;

    if (n == 0) {
        return g;
    }

    g->adj = malloc((size_t)n * sizeof(No *));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        g->adj[i] = NULL;
    }

    return g;
}

int inserir_aresta_lista(GrafoLista *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    if (contem_destino(g->adj[u], v)) {
        return -1;
    }

    No *no_u = criar_no(v);
    if (no_u == NULL) {
        return -1;
    }
    no_u->prox = g->adj[u];
    g->adj[u] = no_u;

    if (u != v) {
        No *no_v = criar_no(u);
        if (no_v == NULL) {
            remover_no(g, u, v);
            return -1;
        }
        no_v->prox = g->adj[v];
        g->adj[v] = no_v;
    }

    return 0;
}

int remover_aresta_lista(GrafoLista *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    if (remover_no(g, u, v) != 0) {
        return -1;
    }

    if (u != v) {
        remover_no(g, v, u);
    }

    return 0;
}

int grau_lista(const GrafoLista *g, int v)
{
    if (g == NULL || !vertice_valido(g, v)) {
        return -1;
    }

    int grau = 0;
    for (const No *atual = g->adj[v]; atual != NULL; atual = atual->prox) {
        grau++;
    }

    return grau;
}

int sao_adjacentes_lista(const GrafoLista *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    return contem_destino(g->adj[u], v);
}

void liberar_grafo_lista(GrafoLista *g)
{
    if (g == NULL) {
        return;
    }

    if (g->adj != NULL) {
        for (int i = 0; i < g->n; i++) {
            No *atual = g->adj[i];
            while (atual != NULL) {
                No *proximo = atual->prox;
                free(atual);
                atual = proximo;
            }
        }
        free(g->adj);
    }

    free(g);
}
