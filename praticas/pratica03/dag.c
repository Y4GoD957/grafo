#include <stdlib.h>

#include "dag.h"

int *ordenacao_topologica_kahn(GrafoLista *g, int *tamanho)
{
    if (tamanho == NULL) {
        return NULL;
    }

    if (g == NULL || g->n < 0) {
        *tamanho = 0;
        return NULL;
    }

    int n = g->n;
    size_t capacidade = n > 0 ? (size_t)n : 1;

    int *grau_entrada = calloc(capacidade, sizeof(int));
    int *fila = malloc(capacidade * sizeof(int));
    int *ordem = malloc(capacidade * sizeof(int));

    if (grau_entrada == NULL || fila == NULL || ordem == NULL) {
        free(grau_entrada);
        free(fila);
        free(ordem);
        *tamanho = 0;
        return NULL;
    }

    for (int u = 0; u < n; u++) {
        for (No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            grau_entrada[atual->destino]++;
        }
    }

    int inicio = 0;
    int fim = 0;

    for (int v = 0; v < n; v++) {
        if (grau_entrada[v] == 0) {
            fila[fim++] = v;
        }
    }

    int count = 0;

    while (inicio < fim) {
        int u = fila[inicio++];
        ordem[count++] = u;

        for (No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            int v = atual->destino;
            grau_entrada[v]--;
            if (grau_entrada[v] == 0) {
                fila[fim++] = v;
            }
        }
    }

    free(grau_entrada);
    free(fila);

    if (count < n) {
        free(ordem);
        *tamanho = 0;
        return NULL;
    }

    *tamanho = n;
    return ordem;
}

typedef enum {
    NAO_VISITADO = 0,
    VISITANDO = 1,
    FINALIZADO = 2
} EstadoVertice;

static int dfs_visita(GrafoLista *g, int u, int *estado, int *ordem, int *posicao)
{
    estado[u] = VISITANDO;

    for (No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
        int v = atual->destino;

        if (estado[v] == VISITANDO) {
            return -1;
        }

        if (estado[v] == NAO_VISITADO) {
            if (dfs_visita(g, v, estado, ordem, posicao) != 0) {
                return -1;
            }
        }
    }

    estado[u] = FINALIZADO;
    ordem[(*posicao)--] = u;

    return 0;
}

int *ordenacao_topologica_dfs(GrafoLista *g, int *tamanho)
{
    if (tamanho == NULL) {
        return NULL;
    }

    if (g == NULL || g->n < 0) {
        *tamanho = 0;
        return NULL;
    }

    int n = g->n;
    size_t capacidade = n > 0 ? (size_t)n : 1;

    int *estado = calloc(capacidade, sizeof(int));
    int *ordem = malloc(capacidade * sizeof(int));

    if (estado == NULL || ordem == NULL) {
        free(estado);
        free(ordem);
        *tamanho = 0;
        return NULL;
    }

    int posicao = n - 1;
    int houve_ciclo = 0;

    for (int u = 0; u < n && !houve_ciclo; u++) {
        if (estado[u] == NAO_VISITADO) {
            if (dfs_visita(g, u, estado, ordem, &posicao) != 0) {
                houve_ciclo = 1;
            }
        }
    }

    free(estado);

    if (houve_ciclo) {
        free(ordem);
        *tamanho = 0;
        return NULL;
    }

    *tamanho = n;
    return ordem;
}

int eh_dag(GrafoLista *g)
{
    int tamanho;
    int *ordem = ordenacao_topologica_kahn(g, &tamanho);

    if (ordem == NULL) {
        return 0;
    }

    free(ordem);
    return 1;
}
