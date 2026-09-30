#ifndef GRAFO_LISTA_H
#define GRAFO_LISTA_H

/* Grafo nao direcionado representado por lista de adjacencia.
 * Vertices numerados de 0 a n-1. */
typedef struct No {
    int destino;
    struct No *prox;
} No;

typedef struct {
    int n;
    No **adj;
} GrafoLista;

/* Cria um grafo com n vertices (n == 0 gera um grafo vazio valido).
 * Retorna NULL se n < 0 ou se a alocacao falhar. */
GrafoLista *criar_grafo_lista(int n);

/* Insere a aresta (u, v). Retorna 0 em sucesso, -1 se o grafo for
 * nulo, algum vertice for invalido ou a aresta ja existir. */
int inserir_aresta_lista(GrafoLista *g, int u, int v);

/* Remove a aresta (u, v). Retorna 0 em sucesso, -1 se o grafo for
 * nulo, algum vertice for invalido ou a aresta nao existir. */
int remover_aresta_lista(GrafoLista *g, int u, int v);

/* Retorna o grau do vertice v, ou -1 se o grafo for nulo ou o
 * vertice for invalido. */
int grau_lista(const GrafoLista *g, int v);

/* Retorna 1 se u e v sao adjacentes, 0 se nao sao, ou -1 se o grafo
 * for nulo ou algum vertice for invalido. */
int sao_adjacentes_lista(const GrafoLista *g, int u, int v);

/* Libera toda a memoria alocada para o grafo (nos, vetor de listas e
 * estrutura). Aceita g == NULL sem efeito. */
void liberar_grafo_lista(GrafoLista *g);

#endif
