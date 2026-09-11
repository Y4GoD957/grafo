#include <stdlib.h>

#include "busca_largura.h"

Fila *fila_criar(int capacidade)
{
    if (capacidade <= 0) {
        return NULL;
    }

    Fila *f = malloc(sizeof(Fila));
    if (f == NULL) {
        return NULL;
    }

    f->dados = malloc((size_t)capacidade * sizeof(int));
    if (f->dados == NULL) {
        free(f);
        return NULL;
    }

    f->capacidade = capacidade;
    f->inicio = 0;
    f->fim = 0;
    f->tamanho = 0;

    return f;
}

int fila_vazia(const Fila *f)
{
    return f == NULL || f->tamanho == 0;
}

int fila_cheia(const Fila *f)
{
    return f != NULL && f->tamanho == f->capacidade;
}

int fila_inserir(Fila *f, int valor)
{
    if (fila_cheia(f)) {
        return -1;
    }

    f->dados[f->fim] = valor;
    f->fim = (f->fim + 1) % f->capacidade;
    f->tamanho++;

    return 0;
}

int fila_remover(Fila *f, int *valor)
{
    if (fila_vazia(f) || valor == NULL) {
        return -1;
    }

    *valor = f->dados[f->inicio];
    f->inicio = (f->inicio + 1) % f->capacidade;
    f->tamanho--;

    return 0;
}

void fila_destruir(Fila *f)
{
    if (f == NULL) {
        return;
    }

    free(f->dados);
    free(f);
}

void bfs(GrafoLista *g, int origem, int *dist, int *pred)
{
    if (g == NULL || dist == NULL || pred == NULL || origem < 0 || origem >= g->n) {
        return;
    }

    for (int i = 0; i < g->n; i++) {
        dist[i] = BFS_SEM_DISTANCIA;
        pred[i] = BFS_SEM_PREDECESSOR;
    }

    if (g->n == 0) {
        return;
    }

    int *descoberto = calloc((size_t)g->n, sizeof(int));
    Fila *fila = fila_criar(g->n);
    if (descoberto == NULL || fila == NULL) {
        free(descoberto);
        fila_destruir(fila);
        return;
    }

    dist[origem] = 0;
    descoberto[origem] = 1;
    fila_inserir(fila, origem);

    int u;
    while (fila_remover(fila, &u) == 0) {
        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            int v = atual->destino;
            if (!descoberto[v]) {
                descoberto[v] = 1;
                dist[v] = dist[u] + 1;
                pred[v] = u;
                fila_inserir(fila, v);
            }
        }
    }

    fila_destruir(fila);
    free(descoberto);
}

int eh_bipartido(GrafoLista *g)
{
    if (g == NULL) {
        return -1;
    }

    if (g->n == 0) {
        return 1;
    }

    int *cor = malloc((size_t)g->n * sizeof(int));
    Fila *fila = fila_criar(g->n);
    if (cor == NULL || fila == NULL) {
        free(cor);
        fila_destruir(fila);
        return -1;
    }

    for (int i = 0; i < g->n; i++) {
        cor[i] = -1;
    }

    int bipartido = 1;

    for (int origem = 0; origem < g->n && bipartido; origem++) {
        if (cor[origem] != -1) {
            continue;
        }

        cor[origem] = 0;
        fila_inserir(fila, origem);

        int u;
        while (bipartido && fila_remover(fila, &u) == 0) {
            for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
                int v = atual->destino;

                if (cor[v] == -1) {
                    cor[v] = 1 - cor[u];
                    fila_inserir(fila, v);
                } else if (cor[v] == cor[u]) {
                    bipartido = 0;
                    break;
                }
            }
        }
    }

    fila_destruir(fila);
    free(cor);

    return bipartido;
}
