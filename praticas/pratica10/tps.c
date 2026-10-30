#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tps.h"

static int vertice_valido(const GrafoTSP *g, int v)
{
    return v >= 0 && v < g->n;
}

static int distancia(const GrafoTSP *g, int u, int v)
{
    return g->peso[(size_t)u * (size_t)g->n + (size_t)v];
}

GrafoTSP *criar_grafo_tsp(int n)
{
    if (n < 0 || (n > 0 && (size_t)n > SIZE_MAX / (size_t)n / sizeof(int))) {
        return NULL;
    }

    GrafoTSP *g = malloc(sizeof(GrafoTSP));
    if (g == NULL) {
        return NULL;
    }

    g->n = n;
    g->peso = calloc((size_t)n * (size_t)n + 1, sizeof(int));
    if (g->peso == NULL) {
        free(g);
        return NULL;
    }

    return g;
}

int inserir_peso_tsp(GrafoTSP *g, int u, int v, int peso)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v) ||
        u == v || peso < 0 || peso > PESO_MAXIMO_TSP) {
        return -1;
    }

    g->peso[(size_t)u * (size_t)g->n + (size_t)v] = peso;
    g->peso[(size_t)v * (size_t)g->n + (size_t)u] = peso;

    return 0;
}

void liberar_grafo_tsp(GrafoTSP *g)
{
    if (g == NULL) {
        return;
    }

    free(g->peso);
    free(g);
}

GrafoTSP *grafo_para_tsp(const GrafoHamiltoniano *g)
{
    if (g == NULL) {
        return NULL;
    }

    GrafoTSP *t = criar_grafo_tsp(g->n);
    if (t == NULL) {
        return NULL;
    }

    for (int u = 0; u < g->n; u++) {
        for (int v = u + 1; v < g->n; v++) {
            inserir_peso_tsp(t, u, v, tem_aresta_hamiltoniano(g, u, v) ? 1 : 2);
        }
    }

    return t;
}

static int entrada_valida(const GrafoTSP *g, int origem, const int *tour, const long long *custo)
{
    if (g == NULL || g->peso == NULL || tour == NULL || custo == NULL ||
        g->n < 3 || !vertice_valido(g, origem)) {
        return 0;
    }

    for (int u = 0; u < g->n; u++) {
        if (distancia(g, u, u) != 0) {
            return 0;
        }
        for (int v = u + 1; v < g->n; v++) {
            int p = distancia(g, u, v);
            if (p < 0 || p > PESO_MAXIMO_TSP || p != distancia(g, v, u)) {
                return 0;
            }
        }
    }

    return 1;
}

static long long custo_do_tour(const GrafoTSP *g, const int *tour)
{
    long long total = 0;
    for (int i = 0; i < g->n; i++) {
        total += distancia(g, tour[i], tour[i + 1]);
    }
    return total;
}

typedef struct {
    const GrafoTSP *g;
    int origem;
    int *atual;
    int *visitado;
    int *melhor;
    long long melhor_custo;
    long long permutacoes;
} Enumeracao;

/* atual[0..tamanho-1] e um prefixo de custo parcial; completa em todas as
 * ordens possiveis dos vertices restantes, em ordem lexicografica. */
static void enumerar(Enumeracao *e, int tamanho, long long parcial)
{
    int n = e->g->n;
    int ultimo = e->atual[tamanho - 1];

    if (tamanho == n) {
        long long total = parcial + distancia(e->g, ultimo, e->origem);
        e->permutacoes++;
        if (total < e->melhor_custo) {
            memcpy(e->melhor, e->atual, (size_t)n * sizeof(int));
            e->melhor_custo = total;
        }
        return;
    }

    for (int v = 0; v < n; v++) {
        if (!e->visitado[v]) {
            e->visitado[v] = 1;
            e->atual[tamanho] = v;
            enumerar(e, tamanho + 1, parcial + distancia(e->g, ultimo, v));
            e->visitado[v] = 0;
        }
    }
}

int tsp_forca_bruta_contado(const GrafoTSP *g, int origem, int *tour, long long *custo,
                            long long *permutacoes)
{
    if (!entrada_valida(g, origem, tour, custo)) {
        return -1;
    }

    int n = g->n;
    Enumeracao e = { g, origem, malloc((size_t)n * sizeof(int)),
                     calloc((size_t)n, sizeof(int)), malloc((size_t)n * sizeof(int)),
                     INT64_MAX, 0 };
    int resultado = -1;

    if (e.atual != NULL && e.visitado != NULL && e.melhor != NULL) {
        e.atual[0] = origem;
        e.visitado[origem] = 1;
        enumerar(&e, 1, 0);

        memcpy(tour, e.melhor, (size_t)n * sizeof(int));
        tour[n] = origem;
        *custo = e.melhor_custo;
        if (permutacoes != NULL) {
            *permutacoes = e.permutacoes;
        }
        resultado = 0;
    }

    free(e.atual);
    free(e.visitado);
    free(e.melhor);
    return resultado;
}

int tsp_forca_bruta(const GrafoTSP *g, int origem, int *tour, long long *custo)
{
    return tsp_forca_bruta_contado(g, origem, tour, custo, NULL);
}

int tsp_vizinho_mais_proximo(const GrafoTSP *g, int origem, int *tour, long long *custo)
{
    if (!entrada_valida(g, origem, tour, custo)) {
        return -1;
    }

    int n = g->n;
    int *visitado = calloc((size_t)n, sizeof(int));
    int *ordem = malloc(((size_t)n + 1) * sizeof(int));
    int resultado = -1;

    if (visitado != NULL && ordem != NULL) {
        ordem[0] = origem;
        visitado[origem] = 1;

        for (int i = 1; i < n; i++) {
            int atual = ordem[i - 1];
            int proximo = -1;
            for (int v = 0; v < n; v++) {
                if (!visitado[v] &&
                    (proximo < 0 || distancia(g, atual, v) < distancia(g, atual, proximo))) {
                    proximo = v;
                }
            }
            visitado[proximo] = 1;
            ordem[i] = proximo;
        }
        ordem[n] = origem;

        memcpy(tour, ordem, ((size_t)n + 1) * sizeof(int));
        *custo = custo_do_tour(g, tour);
        resultado = 0;
    }

    free(visitado);
    free(ordem);
    return resultado;
}

int tsp_insercao(const GrafoTSP *g, int origem, int *tour, long long *custo)
{
    if (!entrada_valida(g, origem, tour, custo)) {
        return -1;
    }

    int n = g->n;
    int *visitado = calloc((size_t)n, sizeof(int));
    int *ordem = malloc(((size_t)n + 1) * sizeof(int));
    int resultado = -1;

    if (visitado != NULL && ordem != NULL) {
        int segundo = -1;
        for (int v = 0; v < n; v++) {
            if (v != origem &&
                (segundo < 0 || distancia(g, origem, v) < distancia(g, origem, segundo))) {
                segundo = v;
            }
        }
        ordem[0] = origem;
        ordem[1] = segundo;
        visitado[origem] = 1;
        visitado[segundo] = 1;

        for (int tamanho = 2; tamanho < n; tamanho++) {
            int escolhido = -1;
            int menor_distancia = 0;
            for (int v = 0; v < n; v++) {
                if (visitado[v]) {
                    continue;
                }
                int d = PESO_MAXIMO_TSP + 1;
                for (int i = 0; i < tamanho; i++) {
                    if (distancia(g, v, ordem[i]) < d) {
                        d = distancia(g, v, ordem[i]);
                    }
                }
                if (escolhido < 0 || d < menor_distancia) {
                    escolhido = v;
                    menor_distancia = d;
                }
            }

            int posicao = 0;
            long long menor_acrescimo = 0;
            for (int p = 0; p < tamanho; p++) {
                int i = ordem[p];
                int j = ordem[(p + 1) % tamanho];
                long long acrescimo = (long long)distancia(g, i, escolhido) +
                                      distancia(g, escolhido, j) - distancia(g, i, j);
                if (p == 0 || acrescimo < menor_acrescimo) {
                    posicao = p;
                    menor_acrescimo = acrescimo;
                }
            }

            memmove(&ordem[posicao + 2], &ordem[posicao + 1],
                    (size_t)(tamanho - posicao - 1) * sizeof(int));
            ordem[posicao + 1] = escolhido;
            visitado[escolhido] = 1;
        }
        ordem[n] = origem;

        memcpy(tour, ordem, ((size_t)n + 1) * sizeof(int));
        *custo = custo_do_tour(g, tour);
        resultado = 0;
    }

    free(visitado);
    free(ordem);
    return resultado;
}
