#ifndef COLORACAO_H
#define COLORACAO_H

#include "grafo_lista.h"

/* Convencao das funcoes de coloracao:
 *
 *   - retornam um vetor alocado dinamicamente com g->n posicoes, em que
 *     cores[v] e a cor do vertice original v; o chamador deve liberar
 *     o vetor com free();
 *   - as cores sao numeradas a partir de 1 (1, 2, ..., *num_cores);
 *   - *num_cores recebe a quantidade de cores efetivamente usadas.
 *
 * Casos especiais:
 *   - grafo vazio (n == 0): retorna NULL e *num_cores = 0;
 *   - g nulo, grafo com laco (u--u, que nao admite coloracao propria,
 *     pois u teria de ter cor diferente de si mesmo) ou falha de
 *     alocacao: retorna NULL e *num_cores = -1.
 *
 * Ambas sao heuristicas: o numero de cores obtido e um limite superior
 * para o numero cromatico, nao necessariamente o seu valor. */

/* Coloracao gulosa na ordem natural dos vertices (0, 1, ..., n-1):
 * cada vertice recebe a menor cor que nenhum vizinho ja colorido usa. */
int *coloracao_gulosa(GrafoLista *g, int *num_cores);

/* Heuristica de Welsh-Powell: aplica a mesma regra gulosa, mas visita
 * os vertices em ordem decrescente de grau (empates: menor indice
 * primeiro). */
int *coloracao_welsh_powell(GrafoLista *g, int *num_cores);

/* Verifica se o grafo nao direcionado e bipartido por 2-coloracao via
 * BFS, iniciada em cada vertice ainda nao visitado (grafos desconexos).
 * Retorna 1 se for bipartido (inclusive o grafo vazio), 0 se nao for
 * (ha um ciclo impar ou um laco) e -1 se g for nulo ou se a alocacao
 * falhar. Um grafo bipartido admite coloracao com no maximo 2 cores. */
int eh_bipartido(GrafoLista *g);

#endif
