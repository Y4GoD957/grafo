#ifndef TPS_H
#define TPS_H

#include "hamiltoniano.h"

/* Pesos validos: inteiros em [0, PESO_MAXIMO_TSP]. Como todo tour tem n
 * arestas e n cabe em int, o custo total (long long) nunca estoura. */
#define PESO_MAXIMO_TSP 1000000000

/* Grafo completo ponderado nao dirigido por matriz de pesos. Vertices
 * numerados de 0 a n-1; peso[u * n + v] e o peso da aresta {u, v}. Uma
 * matriz valida e simetrica, tem diagonal nula e pesos em
 * [0, PESO_MAXIMO_TSP]. Todas as funcoes de TSP exigem n >= 3 e uma matriz
 * valida; caso contrario retornam -1 sem alterar tour e custo. */
typedef struct {
    int n;
    int *peso;
} GrafoTSP;

/* Um tour e um vetor de n + 1 vertices que comeca e termina na origem e
 * visita cada um dos demais vertices exatamente uma vez (n arestas). */

/* Cria um grafo com n vertices e todos os pesos nulos. Retorna NULL se
 * n < 0, se a matriz nao couber em size_t ou se a alocacao falhar. */
GrafoTSP *criar_grafo_tsp(int n);

/* Define o peso da aresta {u, v} (nas duas posicoes da matriz). Retorna 0
 * em sucesso, -1 se o grafo for nulo, algum vertice for invalido, u == v ou
 * o peso estiver fora de [0, PESO_MAXIMO_TSP]. */
int inserir_peso_tsp(GrafoTSP *g, int u, int v, int peso);

/* Libera o grafo. Aceita g == NULL sem efeito. */
void liberar_grafo_tsp(GrafoTSP *g);

/* Reducao Hamiltoniano -> TSP: grafo completo com os mesmos vertices em que
 * toda aresta de g tem peso 1 e toda aresta ausente tem peso 2. Para
 * n >= 3, g tem ciclo hamiltoniano se e somente se o tour otimo do
 * resultado custa exatamente n. Retorna NULL se g for nulo ou a alocacao
 * falhar. g nao e alterado. */
GrafoTSP *grafo_para_tsp(const GrafoHamiltoniano *g);

/* TSP exato por forca bruta: fixa a origem e avalia todas as (n-1)!
 * permutacoes dos demais vertices, sem poda, somando o retorno a origem.
 * Em empate de custo fica o primeiro tour na ordem lexicografica das
 * permutacoes. Grava o tour otimo em tour (n + 1 posicoes) e seu custo em
 * *custo. Retorna 0 em sucesso ou -1 em erro (argumento ou matriz invalida,
 * ou falha de alocacao). Complexidade O(n!). */
int tsp_forca_bruta(const GrafoTSP *g, int origem, int *tour, long long *custo);

/* Igual a tsp_forca_bruta; se permutacoes nao for NULL, grava nele quantas
 * permutacoes foram avaliadas (sempre (n-1)! em sucesso). */
int tsp_forca_bruta_contado(const GrafoTSP *g, int origem, int *tour, long long *custo,
                            long long *permutacoes);

/* Vizinho mais proximo: a partir da origem, vai sempre ao vertice nao
 * visitado de menor distancia ao atual (empate pelo menor indice) e por fim
 * retorna a origem. Mesmas regras de retorno de tsp_forca_bruta. */
int tsp_vizinho_mais_proximo(const GrafoTSP *g, int origem, int *tour, long long *custo);

/* Insercao do mais proximo: comeca com a origem e o vertice mais proximo
 * dela (empate pelo menor indice); a cada passo escolhe o vertice nao
 * visitado de menor distancia a qualquer vertice do tour (empate pelo menor
 * indice) e o insere na posicao de menor acrescimo d(i,k) + d(k,j) - d(i,j)
 * entre vertices consecutivos i e j do ciclo (empate pela primeira posicao
 * a partir da origem). Mesmas regras de retorno de tsp_forca_bruta. */
int tsp_insercao(const GrafoTSP *g, int origem, int *tour, long long *custo);

#endif
