#ifndef BUSCA_LARGURA_H
#define BUSCA_LARGURA_H

#include "grafo_lista.h"

/* Sentinelas usadas por bfs(): vertice sem distancia calculada
 * (inalcancavel) e vertice sem predecessor na arvore de busca. */
#define BFS_SEM_DISTANCIA (-1)
#define BFS_SEM_PREDECESSOR (-1)

/* Fila (FIFO) generica usada pela BFS, implementada como um vetor
 * circular. */
typedef struct {
    int *dados;
    int capacidade, inicio, fim, tamanho;
} Fila;

/* Cria uma fila com a capacidade informada.
 * Retorna NULL se capacidade <= 0 ou se a alocacao falhar. */
Fila *fila_criar(int capacidade);

/* Retorna 1 se a fila estiver vazia (ou for nula), 0 caso contrario. */
int fila_vazia(const Fila *f);

/* Retorna 1 se a fila estiver cheia, 0 caso contrario (ou se for nula). */
int fila_cheia(const Fila *f);

/* Insere valor no fim da fila. Retorna 0 em sucesso, -1 se a fila
 * for nula ou estiver cheia. */
int fila_inserir(Fila *f, int valor);

/* Remove o valor do inicio da fila e o grava em *valor.
 * Retorna 0 em sucesso, -1 se a fila for nula, vazia ou valor for
 * nulo. */
int fila_remover(Fila *f, int *valor);

/* Libera a memoria alocada para a fila. Aceita f == NULL sem efeito. */
void fila_destruir(Fila *f);

/* Busca em largura a partir de origem, usando a Fila acima.
 *
 * dist e pred devem apontar para vetores com g->n posicoes,
 * alocados pelo chamador. Ao final:
 *   - dist[origem] == 0;
 *   - dist[v] == numero de arestas do menor caminho origem -> v,
 *     ou BFS_SEM_DISTANCIA se v for inalcancavel;
 *   - pred[v] == predecessor de v na arvore de BFS, ou
 *     BFS_SEM_PREDECESSOR se v == origem ou for inalcancavel.
 *
 * Nao faz nada se g, dist ou pred forem nulos, ou se origem for um
 * vertice invalido. */
void bfs(GrafoLista *g, int origem, int *dist, int *pred);

/* Verifica se o grafo (nao direcionado) e bipartido, usando
 * 2-coloracao via BFS a partir de cada vertice ainda nao visitado
 * (cobrindo grafos desconexos).
 *
 * Retorna 1 se for bipartido (ou se o grafo for vazio), 0 se houver
 * uma aresta entre vertices de mesma cor, ou -1 se g for nulo. */
int eh_bipartido(GrafoLista *g);

#endif
