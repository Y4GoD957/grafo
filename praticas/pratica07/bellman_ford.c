#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "bellman_ford.h"

/* Infinito interno (long long) das distancias de trabalho. */
#define INF_TRABALHO LLONG_MAX

int bellman_ford_entrada_valida(const GrafoListaPonderado *g, int origem)
{
    if (g == NULL || g->n < 0 || g->n > MAX_VERTICES ||
        origem < 0 || origem >= g->n) {
        return 0;
    }

    for (int u = 0; u < g->n; u++) {
        for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
            if (e->destino < 0 || e->destino >= g->n ||
                e->peso < -PESO_MAXIMO || e->peso > PESO_MAXIMO) {
                return 0;
            }
        }
    }

    return 1;
}

/* Converte uma distancia de trabalho para int. Sem ciclo negativo, todo
 * valor finito e o peso de um caminho simples e ja cabe em int; o limite
 * so atua quando ha ciclo negativo, caso em que dist nao e significativa. */
static int para_int(long long d)
{
    if (d == INF_TRABALHO) {
        return BF_INF;
    }
    if (d > (long long)BF_INF - 1) {
        return BF_INF - 1;
    }
    if (d < (long long)INT_MIN) {
        return INT_MIN;
    }
    return (int)d;
}

int bellman_ford_detalhado(GrafoListaPonderado *g, int origem, int *dist,
                           int *pred, int *vertice_afetado)
{
    if (vertice_afetado != NULL) {
        *vertice_afetado = -1;
    }

    if (dist == NULL || pred == NULL || !bellman_ford_entrada_valida(g, origem)) {
        return 0;
    }

    int n = g->n;

    /* Distancias de trabalho em long long: sem ciclo negativo cada valor e
     * um caminho simples (|d| < INT_MAX). Com ciclo negativo, cada
     * relaxacao estende em um arco o caminho de um rotulo existente, logo
     * |d| <= n * m * PESO_MAXIMO <= 2000 * 4e6 * 1e6 = 8e15, muito abaixo
     * de LLONG_MAX; assim nao ha overflow e nenhum grafo valido e
     * confundido com ciclo negativo por falta de representacao. */
    long long *d = malloc((size_t)n * sizeof(long long));
    int *p = malloc((size_t)n * sizeof(int));
    if (d == NULL || p == NULL) {
        free(d);
        free(p);
        return 0;
    }

    for (int v = 0; v < n; v++) {
        d[v] = INF_TRABALHO;
        p[v] = -1;
    }
    d[origem] = 0;

    /* n-1 rodadas de relaxacao; encerra cedo se uma rodada nao muda nada. */
    for (int rodada = 1; rodada < n; rodada++) {
        int mudou = 0;
        for (int u = 0; u < n; u++) {
            if (d[u] == INF_TRABALHO) {
                continue;
            }
            for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
                long long nova = d[u] + e->peso;
                if (nova < d[e->destino]) {
                    d[e->destino] = nova;
                    p[e->destino] = u;
                    mudou = 1;
                }
            }
        }
        if (!mudou) {
            break;
        }
    }

    /* n-esima passada: qualquer relaxacao restante revela ciclo negativo
     * alcancavel. Registra o vertice relaxado e guarda o predecessor. */
    int afetado = -1;
    for (int u = 0; u < n && afetado < 0; u++) {
        if (d[u] == INF_TRABALHO) {
            continue;
        }
        for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
            long long nova = d[u] + e->peso;
            if (nova < d[e->destino]) {
                d[e->destino] = nova;
                p[e->destino] = u;
                afetado = e->destino;
                break;
            }
        }
    }

    for (int v = 0; v < n; v++) {
        dist[v] = para_int(d[v]);
        pred[v] = p[v];
    }

    free(d);
    free(p);

    if (afetado >= 0) {
        if (vertice_afetado != NULL) {
            *vertice_afetado = afetado;
        }
        return 0;
    }

    return 1;
}

int bellman_ford(GrafoListaPonderado *g, int origem, int *dist, int *pred)
{
    return bellman_ford_detalhado(g, origem, dist, pred, NULL);
}

/* Procura o arco u -> v em g; retorna 0 e grava o peso em *peso se existir. */
static int peso_do_arco(const GrafoListaPonderado *g, int u, int v, int *peso)
{
    for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
        if (e->destino == v) {
            *peso = e->peso;
            return 0;
        }
    }

    return -1;
}

int *reconstruir_ciclo_negativo(GrafoListaPonderado *g, const int *pred,
                                int vertice_afetado)
{
    if (g == NULL || pred == NULL ||
        vertice_afetado < 0 || vertice_afetado >= g->n) {
        return NULL;
    }

    /* Retroceder n vezes garante parar dentro de um ciclo: uma cadeia de
     * predecessores sem ciclo teria no maximo n-1 arcos antes de -1. */
    int v = vertice_afetado;
    for (int i = 0; i < g->n; i++) {
        v = pred[v];
        if (v < 0 || v >= g->n) {
            return NULL;
        }
    }

    /* v esta no ciclo: conta seus vertices seguindo pred ate voltar a v. */
    int tamanho = 1;
    for (int u = pred[v]; u != v; u = pred[u]) {
        if (u < 0 || u >= g->n || ++tamanho > g->n) {
            return NULL;
        }
    }

    int *ciclo = malloc((size_t)(tamanho + 2) * sizeof(int));
    if (ciclo == NULL) {
        return NULL;
    }

    /* pred anda para tras nos arcos; preenche de tras para frente para
     * obter a ordem de arcos e fecha o ciclo repetindo o primeiro. */
    int u = v;
    for (int i = tamanho; i >= 1; i--) {
        ciclo[i] = u;
        u = pred[u];
    }
    ciclo[0] = ciclo[tamanho];
    ciclo[tamanho + 1] = -1;

    /* Confere os arcos reais e o peso total (long long, sem overflow). */
    long long total = 0;
    for (int i = 0; i < tamanho; i++) {
        int peso;
        if (peso_do_arco(g, ciclo[i], ciclo[i + 1], &peso) != 0) {
            free(ciclo);
            return NULL;
        }
        total += peso;
    }

    if (total >= 0) {
        free(ciclo);
        return NULL;
    }

    return ciclo;
}

void imprimir_ciclo(const int *ciclo)
{
    if (ciclo == NULL) {
        printf("(sem ciclo)\n");
        return;
    }

    for (int i = 0; ciclo[i] != -1; i++) {
        printf(i == 0 ? "%d" : " -> %d", ciclo[i]);
    }
    printf("\n");
}
