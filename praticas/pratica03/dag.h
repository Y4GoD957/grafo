#ifndef DAG_H
#define DAG_H

#include "grafo_lista.h"

/* Ordena topologicamente um digrafo usando o algoritmo de Kahn (BFS
 * com grau de entrada). Em sucesso, retorna um vetor alocado
 * dinamicamente com os n vertices na ordem topologica e grava n em
 * *tamanho. Se g nao for um DAG (houver ciclo), retorna NULL e grava
 * 0 em *tamanho. O chamador deve liberar o vetor retornado. */
int *ordenacao_topologica_kahn(GrafoLista *g, int *tamanho);

/* Ordena topologicamente um digrafo usando DFS (ordem reversa de
 * termino). Mesma convencao de retorno de ordenacao_topologica_kahn. */
int *ordenacao_topologica_dfs(GrafoLista *g, int *tamanho);

/* Retorna 1 se g e um DAG (digrafo acíclico), 0 se contem ciclo ou
 * se g for invalido. */
int eh_dag(GrafoLista *g);

#endif
