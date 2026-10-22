#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include <limits.h>

#include "grafo_ponderado.h"

/* Sentinela de "infinito": distancia de vertices inalcancaveis.
 * Reservada: distancias finitas sao sempre estritamente menores que
 * INT_MAX (pesos validos vao ate PESO_MAXIMO = INT_MAX - 1). Uma soma
 * que alcance INT_MAX e tratada como "sem caminho". */
#define DIJKSTRA_INF INT_MAX

typedef struct {
    int vertice;
    int distancia;
} NoHeap;

typedef struct {
    NoHeap *dados;
    int capacidade, tamanho;
    int *posicao; /* mapeia vertice -> indice no heap, ou -1 se ausente */
} MinHeap;

/* Cria um Min-Heap vazio para vertices 0..capacidade-1. Retorna NULL
 * se capacidade < 0 ou se a alocacao falhar. */
MinHeap *criar_min_heap(int capacidade);

/* Libera o heap. Aceita h == NULL sem efeito. */
void liberar_min_heap(MinHeap *h);

/* Retorna 1 se o vertice esta no heap, 0 se nao, -1 se h for nulo ou
 * o vertice for invalido. */
int heap_contem(const MinHeap *h, int vertice);

/* Insere (vertice, distancia). Retorna 0 em sucesso, -1 se h for nulo,
 * o vertice for invalido, ja estiver no heap ou o heap estiver cheio. */
int inserir_min_heap(MinHeap *h, int vertice, int distancia);

/* Remove o no de menor distancia e o copia para *saida. Retorna 0 em
 * sucesso, -1 se h ou saida forem nulos ou o heap estiver vazio. */
int extrair_minimo(MinHeap *h, NoHeap *saida);

/* Reduz a distancia de um vertice que ja esta no heap. Retorna 0 em
 * sucesso, -1 se h for nulo, o vertice for invalido ou ausente do
 * heap, ou nova_distancia for maior que a atual. */
int diminuir_chave(MinHeap *h, int vertice, int nova_distancia);

/* Dijkstra a partir de origem, usando Min-Heap. dist e pred devem ter
 * g->n posicoes. Ao final: dist[v] e a menor distancia (DIJKSTRA_INF
 * se v for inalcancavel) e pred[v] o predecessor de v no caminho
 * minimo (-1 para a origem e para vertices inalcancaveis).
 * Pesos fora de [0, PESO_MAXIMO] (negativos ou INT_MAX) nao sao
 * suportados e sao detectados antes de qualquer escrita.
 * Retorna 0 em sucesso, -1 se g/dist/pred forem nulos, a origem for
 * invalida ou houver peso invalido (dist e pred ficam inalterados), ou
 * -1 se a alocacao falhar (dist e pred nao devem ser usados). Somas que estourariam int sao
 * tratadas como "sem caminho melhor". */
int dijkstra(GrafoListaPonderado *g, int origem, int *dist, int *pred);

/* Reconstroi o caminho origem -> destino a partir do vetor pred gerado
 * por dijkstra. Retorna um vetor alocado com malloc, na ordem origem ->
 * destino e terminado por -1 (seguro: vertices sao >= 0). O chamador
 * deve liberar com free. Se origem == destino, retorna {origem, -1}.
 * Retorna NULL se nao houver caminho, se pred for nulo, origem/destino
 * forem negativos, a cadeia de predecessores nao chegar a origem ou
 * contiver ciclo, ou a alocacao falhar. pred deve ser um vetor
 * produzido por dijkstra para um grafo que contenha origem e destino;
 * como a funcao nao recebe o tamanho, indices dentro da cadeia nao sao
 * verificados contra n. */
int *reconstruir_caminho(int origem, int destino, int *pred);

#endif
