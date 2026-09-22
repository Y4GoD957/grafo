#ifndef CONECTIVIDADE_H
#define CONECTIVIDADE_H

#include "grafo_lista.h"

/* Aresta de corte (ponte) normalizada com u < v. */
typedef struct {
    int u, v;
} Ponte;

/* Encontra os vertices de articulacao (vertices de corte) de um grafo
 * nao direcionado pelo algoritmo de Tarjan (DFS com descoberta[] e
 * low[]). A DFS e iniciada a partir de todo vertice ainda nao
 * descoberto, de modo que grafos desconexos sao tratados componente
 * a componente.
 *
 * eh_articulacao deve apontar para um vetor com g->n posicoes; ao
 * final, eh_articulacao[v] == 1 se v e articulacao e 0 caso contrario.
 * Retorna a quantidade de articulacoes, ou -1 se g ou eh_articulacao
 * forem nulos ou se a alocacao falhar. */
int dfs_articulacoes(GrafoLista *g, int *eh_articulacao);

/* Detecta as pontes (arestas de corte) de um grafo nao direcionado:
 * uma aresta de arvore (u, v) da DFS e ponte quando low[v] >
 * descoberta[u]. Tambem percorre todas as componentes.
 *
 * pontes deve apontar para um vetor com pelo menos g->n posicoes (um
 * grafo com n vertices tem no maximo n - 1 pontes). Cada ponte aparece
 * uma unica vez, normalizada como (min, max), e o vetor sai ordenado.
 * Retorna a quantidade de pontes, ou -1 se g ou pontes forem nulos ou
 * se a alocacao falhar. */
int detectar_pontes(GrafoLista *g, Ponte *pontes);

#endif
