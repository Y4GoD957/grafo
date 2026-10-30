#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hamiltoniano.h"

static int vertice_valido(const GrafoHamiltoniano *g, int v)
{
    return v >= 0 && v < g->n;
}

static int adjacente(const GrafoHamiltoniano *g, int u, int v)
{
    return g->adj[(size_t)u * (size_t)g->n + (size_t)v];
}

GrafoHamiltoniano *criar_grafo_hamiltoniano(int n)
{
    if (n < 0 || (n > 0 && (size_t)n > SIZE_MAX / (size_t)n)) {
        return NULL;
    }

    GrafoHamiltoniano *g = malloc(sizeof(GrafoHamiltoniano));
    if (g == NULL) {
        return NULL;
    }

    g->n = n;
    g->adj = calloc((size_t)n * (size_t)n + 1, sizeof(unsigned char));
    if (g->adj == NULL) {
        free(g);
        return NULL;
    }

    return g;
}

int inserir_aresta_hamiltoniano(GrafoHamiltoniano *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v) ||
        u == v || adjacente(g, u, v)) {
        return -1;
    }

    g->adj[(size_t)u * (size_t)g->n + (size_t)v] = 1;
    g->adj[(size_t)v * (size_t)g->n + (size_t)u] = 1;

    return 0;
}

int tem_aresta_hamiltoniano(const GrafoHamiltoniano *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v)) {
        return -1;
    }

    return adjacente(g, u, v);
}

int contar_arestas_hamiltoniano(const GrafoHamiltoniano *g)
{
    if (g == NULL) {
        return -1;
    }

    int total = 0;
    for (int u = 0; u < g->n; u++) {
        for (int v = u + 1; v < g->n; v++) {
            total += adjacente(g, u, v);
        }
    }

    return total;
}

void liberar_grafo_hamiltoniano(GrafoHamiltoniano *g)
{
    if (g == NULL) {
        return;
    }

    free(g->adj);
    free(g);
}

typedef struct {
    const GrafoHamiltoniano *g;
    int *trilha;
    int *visitado;
    int fechar;  /* 1: o ultimo vertice deve ser adjacente a trilha[0] */
} Busca;

/* trilha[0..tamanho-1] e uma trilha simples valida; tenta completa-la. */
static int estender(Busca *b, int tamanho)
{
    int n = b->g->n;
    int ultimo = b->trilha[tamanho - 1];

    if (tamanho == n) {
        return !b->fechar || adjacente(b->g, ultimo, b->trilha[0]);
    }

    for (int v = 0; v < n; v++) {
        if (!b->visitado[v] && adjacente(b->g, ultimo, v)) {
            b->visitado[v] = 1;
            b->trilha[tamanho] = v;
            if (estender(b, tamanho + 1)) {
                return 1;
            }
            b->visitado[v] = 0;
        }
    }

    return 0;
}

static int buscar(const GrafoHamiltoniano *g, int *saida, int fechar)
{
    if (g == NULL || saida == NULL || g->n == 0) {
        return -1;
    }

    int n = g->n;
    Busca b = { g, malloc(((size_t)n + 1) * sizeof(int)),
                calloc((size_t)n + 1, sizeof(int)), fechar };
    int resultado = -1;

    if (b.trilha != NULL && b.visitado != NULL) {
        int inicio_final = fechar ? 1 : n;

        resultado = 0;
        for (int inicio = 0; inicio < inicio_final && !resultado; inicio++) {
            memset(b.visitado, 0, (size_t)n * sizeof(int));
            b.visitado[inicio] = 1;
            b.trilha[0] = inicio;
            resultado = estender(&b, 1);
        }

        if (resultado) {
            memcpy(saida, b.trilha, (size_t)n * sizeof(int));
            if (fechar) {
                saida[n] = saida[0];
            }
        }
    }

    free(b.trilha);
    free(b.visitado);
    return resultado;
}

int encontrar_caminho_hamiltoniano_bt(const GrafoHamiltoniano *g, int *caminho)
{
    return buscar(g, caminho, 0);
}

int encontrar_ciclo_hamiltoniano_bt(const GrafoHamiltoniano *g, int *ciclo)
{
    if (g != NULL && ciclo != NULL && g->n > 0 && g->n < 3) {
        return 0;
    }

    return buscar(g, ciclo, 1);
}
