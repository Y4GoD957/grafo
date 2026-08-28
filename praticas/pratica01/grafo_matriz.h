#ifndef GRAFO_MATRIZ_H
#define GRAFO_MATRIZ_H

/* Grafo nao direcionado representado por matriz de adjacencia.
 * Vertices numerados de 0 a n-1. */
typedef struct {
    int n;
    int **adj;
} GrafoMatriz;

/* Cria um grafo com n vertices (n == 0 gera um grafo vazio valido).
 * Retorna NULL se n < 0 ou se a alocacao falhar. */
GrafoMatriz *criar_grafo_matriz(int n);

/* Insere a aresta (u, v). Retorna 0 em sucesso, -1 se o grafo for
 * nulo, algum vertice for invalido ou a aresta ja existir. */
int inserir_aresta_matriz(GrafoMatriz *g, int u, int v);

/* Remove a aresta (u, v). Retorna 0 em sucesso, -1 se o grafo for
 * nulo, algum vertice for invalido ou a aresta nao existir. */
int remover_aresta_matriz(GrafoMatriz *g, int u, int v);

/* Retorna o grau do vertice v, ou -1 se o grafo for nulo ou o
 * vertice for invalido. */
int grau_matriz(const GrafoMatriz *g, int v);

/* Retorna 1 se u e v sao adjacentes, 0 se nao sao, ou -1 se o grafo
 * for nulo ou algum vertice for invalido. */
int sao_adjacentes_matriz(const GrafoMatriz *g, int u, int v);

/* Libera toda a memoria alocada para o grafo (linhas, matriz e
 * estrutura). Aceita g == NULL sem efeito. */
void liberar_grafo_matriz(GrafoMatriz *g);

#endif
