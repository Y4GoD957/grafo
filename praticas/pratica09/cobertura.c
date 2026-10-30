#include <stdlib.h>
#include <string.h>

#include "cobertura.h"

static int vertice_valido(const GrafoCobertura *g, int v)
{
    return v >= 0 && v < g->n;
}

static int tem_aresta(const GrafoCobertura *g, int u, int v)
{
    return g->adj[(size_t)u * (size_t)g->n + (size_t)v];
}

GrafoCobertura *criar_grafo_cobertura(int n)
{
    if (n < 0) {
        return NULL;
    }

    GrafoCobertura *g = malloc(sizeof(GrafoCobertura));
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

int inserir_aresta_cobertura(GrafoCobertura *g, int u, int v)
{
    if (g == NULL || !vertice_valido(g, u) || !vertice_valido(g, v) ||
        u == v || tem_aresta(g, u, v)) {
        return -1;
    }

    g->adj[(size_t)u * (size_t)g->n + (size_t)v] = 1;
    g->adj[(size_t)v * (size_t)g->n + (size_t)u] = 1;

    return 0;
}

int contar_arestas_cobertura(const GrafoCobertura *g)
{
    if (g == NULL) {
        return -1;
    }

    int total = 0;
    for (int u = 0; u < g->n; u++) {
        for (int v = u + 1; v < g->n; v++) {
            total += tem_aresta(g, u, v);
        }
    }

    return total;
}

void liberar_grafo_cobertura(GrafoCobertura *g)
{
    if (g == NULL) {
        return;
    }

    free(g->adj);
    free(g);
}

int eh_cobertura(const GrafoCobertura *g, const int *cobertura)
{
    if (g == NULL || cobertura == NULL) {
        return -1;
    }

    for (int u = 0; u < g->n; u++) {
        for (int v = u + 1; v < g->n; v++) {
            if (tem_aresta(g, u, v) && !cobertura[u] && !cobertura[v]) {
                return 0;
            }
        }
    }

    return 1;
}

int cobertura_gulosa(const GrafoCobertura *g, int *cobertura)
{
    if (g == NULL || cobertura == NULL) {
        return -1;
    }

    int n = g->n;
    int *grau = calloc((size_t)n + 1, sizeof(int));
    int *escolhido = calloc((size_t)n + 1, sizeof(int));
    if (grau == NULL || escolhido == NULL) {
        free(grau);
        free(escolhido);
        return -1;
    }

    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            grau[u] += tem_aresta(g, u, v);
        }
    }

    int tamanho = 0;
    for (;;) {
        int melhor = -1;
        for (int v = 0; v < n; v++) {
            if (grau[v] > 0 && (melhor < 0 || grau[v] > grau[melhor])) {
                melhor = v;
            }
        }
        if (melhor < 0) {
            break;
        }

        escolhido[melhor] = 1;
        tamanho++;
        for (int w = 0; w < n; w++) {
            if (tem_aresta(g, melhor, w) && !escolhido[w]) {
                grau[w]--;
            }
        }
        grau[melhor] = 0;
    }

    memcpy(cobertura, escolhido, (size_t)n * sizeof(int));

    free(grau);
    free(escolhido);
    return tamanho;
}

typedef struct {
    const GrafoCobertura *g;
    int m;
    int *eu, *ev;   /* arestas {eu[i], ev[i]} com eu[i] < ev[i] */
    int *atual;     /* cobertura parcial */
    int *melhor;    /* melhor cobertura completa conhecida */
    int melhor_tam;
    int *usado;     /* marcas do limite inferior */
    int *pilha;     /* vertices adicionados no ramo corrente, para desfazer */
    int topo;
} Busca;

/* Tamanho de um emparelhamento guloso nas arestas descobertas a partir de
 * ini: arestas disjuntas exigem vertices distintos, logo e um limite
 * inferior para o que ainda falta escolher. */
static int limite_inferior(Busca *b, int ini)
{
    memset(b->usado, 0, (size_t)b->g->n * sizeof(int));

    int total = 0;
    for (int i = ini; i < b->m; i++) {
        int u = b->eu[i];
        int v = b->ev[i];
        if (!b->atual[u] && !b->atual[v] && !b->usado[u] && !b->usado[v]) {
            b->usado[u] = 1;
            b->usado[v] = 1;
            total++;
        }
    }

    return total;
}

static void buscar(Busca *b, int idx, int tamanho)
{
    while (idx < b->m && (b->atual[b->eu[idx]] || b->atual[b->ev[idx]])) {
        idx++;
    }

    if (idx == b->m) {
        if (tamanho < b->melhor_tam) {
            memcpy(b->melhor, b->atual, (size_t)b->g->n * sizeof(int));
            b->melhor_tam = tamanho;
        }
        return;
    }

    if (tamanho + limite_inferior(b, idx) >= b->melhor_tam) {
        return;
    }

    int u = b->eu[idx];

    b->atual[u] = 1;
    buscar(b, idx + 1, tamanho + 1);
    b->atual[u] = 0;

    int base = b->topo;
    for (int w = 0; w < b->g->n; w++) {
        if (tem_aresta(b->g, u, w) && !b->atual[w]) {
            b->atual[w] = 1;
            b->pilha[b->topo++] = w;
        }
    }
    buscar(b, idx + 1, tamanho + (b->topo - base));
    while (b->topo > base) {
        b->atual[b->pilha[--b->topo]] = 0;
    }
}

static void liberar_busca(Busca *b)
{
    free(b->eu);
    free(b->ev);
    free(b->atual);
    free(b->melhor);
    free(b->usado);
    free(b->pilha);
}

int cobertura_minima_bt(const GrafoCobertura *g, int *cobertura)
{
    if (g == NULL || cobertura == NULL) {
        return -1;
    }

    int n = g->n;
    int m = contar_arestas_cobertura(g);

    Busca b = { g, m, NULL, NULL, NULL, NULL, 0, NULL, NULL, 0 };
    b.eu = malloc(((size_t)m + 1) * sizeof(int));
    b.ev = malloc(((size_t)m + 1) * sizeof(int));
    b.atual = calloc((size_t)n + 1, sizeof(int));
    b.melhor = calloc((size_t)n + 1, sizeof(int));
    b.usado = calloc((size_t)n + 1, sizeof(int));
    b.pilha = calloc((size_t)n + 1, sizeof(int));
    if (b.eu == NULL || b.ev == NULL || b.atual == NULL || b.melhor == NULL ||
        b.usado == NULL || b.pilha == NULL) {
        liberar_busca(&b);
        return -1;
    }

    int k = 0;
    for (int u = 0; u < n; u++) {
        for (int v = u + 1; v < n; v++) {
            if (tem_aresta(g, u, v)) {
                b.eu[k] = u;
                b.ev[k] = v;
                k++;
            }
        }
    }

    b.melhor_tam = cobertura_gulosa(g, b.melhor);
    if (b.melhor_tam < 0) {
        liberar_busca(&b);
        return -1;
    }

    buscar(&b, 0, 0);

    memcpy(cobertura, b.melhor, (size_t)n * sizeof(int));
    int resultado = b.melhor_tam;

    liberar_busca(&b);
    return resultado;
}
