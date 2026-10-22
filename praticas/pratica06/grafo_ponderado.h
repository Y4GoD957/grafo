#ifndef GRAFO_PONDERADO_H
#define GRAFO_PONDERADO_H

#include <limits.h>

/* INT_MAX e reservado como "infinito" (distancia de vertices
 * inalcancaveis, ver DIJKSTRA_INF). Logo todo peso valido esta em
 * [0, PESO_MAXIMO] e toda distancia finita e estritamente menor que
 * INT_MAX. */
#define PESO_MAXIMO (INT_MAX - 1)

/* Grafo ponderado por lista de adjacencia. Vertices numerados de 0 a
 * n-1; os pesos sao inteiros em [0, PESO_MAXIMO] (nao negativos, como
 * exige o Dijkstra, e menores que INT_MAX). */
typedef struct NoPonderado {
    int destino;
    int peso;
    struct NoPonderado *prox;
} NoPonderado;

typedef struct {
    int n;
    NoPonderado **adj;
} GrafoListaPonderado;

/* Cria um grafo com n vertices (n == 0 gera um grafo vazio valido).
 * Retorna NULL se n < 0 ou se a alocacao falhar. O chamador deve
 * liberar o grafo com liberar_grafo_ponderado. */
GrafoListaPonderado *criar_grafo_ponderado(int n);

/* Insere o arco dirigido u -> v com o peso dado. Retorna 0 em sucesso,
 * -1 se o grafo for nulo, algum vertice for invalido, u == v, o peso
 * estiver fora de [0, PESO_MAXIMO] (negativo ou INT_MAX), o arco ja existir ou a alocacao falhar. */
int inserir_arco_ponderado(GrafoListaPonderado *g, int u, int v, int peso);

/* Insere a aresta nao dirigida (u, v), ou seja, os arcos u -> v e
 * v -> u com o mesmo peso. Mesmos codigos de retorno de
 * inserir_arco_ponderado; em caso de erro o grafo nao e alterado. */
int inserir_aresta_ponderada(GrafoListaPonderado *g, int u, int v, int peso);

/* Libera todo o grafo. Aceita g == NULL sem efeito. */
void liberar_grafo_ponderado(GrafoListaPonderado *g);

#endif
