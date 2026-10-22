#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include <limits.h>

#include "grafo_ponderado.h"

/* Sentinela de "infinito": distancia de vertices inalcancaveis. Como
 * todo caminho simples pesa menos que INT_MAX (ver grafo_ponderado.h),
 * nenhuma distancia finita correta colide com ela. */
#define BF_INF INT_MAX

/* Retorna 1 se (g, origem) e uma entrada aceita por bellman_ford e 0
 * caso contrario: g nulo, origem fora de [0, n-1], n > MAX_VERTICES ou
 * algum peso fora de [-PESO_MAXIMO, PESO_MAXIMO] (por exemplo, inserido
 * alterando a estrutura diretamente). Permite ao chamador distinguir
 * entrada invalida de ciclo negativo, pois bellman_ford usa 0 para ambos. */
int bellman_ford_entrada_valida(const GrafoListaPonderado *g, int origem);

/* Bellman-Ford a partir de origem: n-1 rodadas de relaxacao de todos os
 * arcos (ignorando arcos cuja cauda e inalcancavel) e uma n-esima passada
 * de deteccao. dist e pred devem ter g->n posicoes.
 *
 * Retorna 1 se nao ha ciclo negativo alcancavel a partir da origem. Neste
 * caso dist[v] e a menor distancia (BF_INF se v for inalcancavel) e pred[v]
 * o predecessor no caminho minimo (-1 para a origem e inalcancaveis).
 *
 * Retorna 0 se ha ciclo negativo alcancavel: dist deixa de ser significativa
 * (os valores sao apenas limitados ao intervalo de int) e pred guarda a
 * informacao para reconstruir_ciclo_negativo. Ciclos negativos inalcancaveis
 * da origem sao ignorados.
 *
 * Retorna tambem 0 se a entrada for invalida (ver bellman_ford_entrada_valida)
 * ou a alocacao falhar; nestes casos dist e pred ficam inalterados. A API
 * obrigatoria so tem dois valores de retorno, entao 0 sozinho nao distingue
 * "ciclo negativo" de "entrada invalida". */
int bellman_ford(GrafoListaPonderado *g, int origem, int *dist, int *pred);

/* Igual a bellman_ford; se retornar 0 por ciclo negativo e vertice_afetado
 * nao for NULL, grava nele o vertice relaxado na passada de deteccao
 * (entrada para reconstruir_ciclo_negativo). Nos demais casos (retorno 1,
 * entrada invalida) grava -1. */
int bellman_ford_detalhado(GrafoListaPonderado *g, int origem, int *dist,
                           int *pred, int *vertice_afetado);

/* Reconstroi um ciclo negativo a partir de pred e do vertice afetado
 * devolvido por bellman_ford_detalhado: retrocede n vezes por pred[] (o que
 * garante estar dentro de um ciclo) e percorre o ciclo uma vez. Retorna um
 * vetor alocado com malloc, em ordem de arcos (cada elemento tem um arco
 * para o seguinte), fechado repetindo o primeiro vertice e terminado em -1;
 * ex.: {1, 2, 3, 1, -1}. Um laco negativo em v retorna {v, v, -1}. O
 * chamador libera com free. Retorna NULL se os argumentos forem invalidos,
 * pred nao levar a um ciclo, algum arco do ciclo nao existir em g, ou a
 * soma dos pesos dos arcos do ciclo (conferida em g) nao for negativa. */
int *reconstruir_ciclo_negativo(GrafoListaPonderado *g, const int *pred,
                                int vertice_afetado);

/* Imprime um ciclo no formato "1 -> 2 -> 3 -> 1" (ciclo terminado em -1,
 * como devolvido por reconstruir_ciclo_negativo). NULL imprime
 * "(sem ciclo)". */
void imprimir_ciclo(const int *ciclo);

#endif
