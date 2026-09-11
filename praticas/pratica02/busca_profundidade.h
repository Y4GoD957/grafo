#ifndef BUSCA_PROFUNDIDADE_H
#define BUSCA_PROFUNDIDADE_H

#include "grafo_lista.h"

/* Pilha (LIFO) generica usada pela DFS iterativa, implementada como
 * um vetor que cresce dinamicamente quando necessario. */
typedef struct {
    int *dados;
    int topo, capacidade;
} Pilha;

/* Cria uma pilha com a capacidade inicial informada.
 * Retorna NULL se capacidade <= 0 ou se a alocacao falhar. */
Pilha *pilha_criar(int capacidade);

/* Retorna 1 se a pilha estiver vazia (ou for nula), 0 caso contrario. */
int pilha_vazia(const Pilha *p);

/* Empilha valor no topo da pilha, aumentando a capacidade
 * automaticamente se necessario. Retorna 0 em sucesso, -1 se a
 * pilha for nula ou se a realocacao falhar. */
int pilha_empilhar(Pilha *p, int valor);

/* Remove o valor do topo da pilha e o grava em *valor.
 * Retorna 0 em sucesso, -1 se a pilha for nula, vazia ou valor for
 * nulo. */
int pilha_desempilhar(Pilha *p, int *valor);

/* Libera a memoria alocada para a pilha. Aceita p == NULL sem efeito. */
void pilha_destruir(Pilha *p);

/* Busca em profundidade recursiva a partir de u.
 *
 * visitado, entrada e saida devem apontar para vetores com g->n
 * posicoes, alocados e zerados/inicializados pelo chamador antes da
 * primeira chamada (visitado com 0). tempo aponta para um contador
 * global de passos, tambem inicializado pelo chamador (0), que e
 * incrementado a cada entrada/saida de vertice para manter os
 * timestamps consistentes entre chamadas (inclusive para varias
 * raizes, no caso de grafos desconexos).
 *
 * Ao final da chamada em u (e de toda a sua subarvore):
 *   - visitado[u] == 1;
 *   - entrada[u] < saida[u];
 *
 * Nao faz nada se g ou visitado forem nulos, se u for invalido ou
 * se u ja tiver sido visitado. */
void dfs_recursiva(GrafoLista *g, int u, int *visitado, int *entrada, int *saida, int *tempo);

/* Busca em profundidade iterativa a partir de origem, usando a
 * Pilha acima em vez de recursao.
 *
 * visitado deve apontar para um vetor com g->n posicoes, alocado e
 * zerado pelo chamador. Marca como visitados todos os vertices
 * alcancaveis a partir de origem. Nao faz nada se g ou visitado
 * forem nulos, ou se origem for invalida. */
void dfs_iterativa(GrafoLista *g, int origem, int *visitado);

/* Conta o numero de componentes conexas do grafo, percorrendo-o com
 * dfs_recursiva a partir de cada vertice ainda nao visitado.
 * Retorna -1 se g for nulo. */
int contar_componentes(GrafoLista *g);

/* Detecta se o grafo (nao direcionado) possui ciclo, usando DFS com
 * rastreamento do vertice pai: um vizinho ja visitado que nao seja
 * o pai indica um ciclo (isso tambem cobre laços, u--u).
 * Retorna 1 se houver ciclo, 0 caso contrario, ou 0 se g for nulo. */
int tem_ciclo(GrafoLista *g);

#endif
