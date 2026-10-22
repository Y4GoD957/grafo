#ifndef GRAFO_PONDERADO_H
#define GRAFO_PONDERADO_H

/* Grafo ponderado por lista de adjacencia, adaptado da pratica 06 para
 * aceitar pesos negativos (Bellman-Ford). Vertices numerados de 0 a n-1.
 *
 * Limites documentados (garantem que as distancias de caminhos simples
 * cabem em int e que as contas internas do Bellman-Ford cabem em
 * long long):
 *   - n <= MAX_VERTICES;
 *   - -PESO_MAXIMO <= peso <= PESO_MAXIMO.
 * Com isso, qualquer caminho simples pesa no maximo
 * (MAX_VERTICES - 1) * PESO_MAXIMO = 1.999.000.000 < INT_MAX, e INT_MAX
 * fica reservado como "infinito". */
#define MAX_VERTICES 2000
#define PESO_MAXIMO 1000000

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
 * Retorna NULL se n < 0, n > MAX_VERTICES ou se a alocacao falhar. O
 * chamador deve liberar o grafo com liberar_grafo_ponderado. */
GrafoListaPonderado *criar_grafo_ponderado(int n);

/* Insere o arco dirigido u -> v com o peso dado. Retorna 0 em sucesso,
 * -1 se o grafo for nulo, algum vertice for invalido, o peso estiver
 * fora de [-PESO_MAXIMO, PESO_MAXIMO], o arco ja existir ou a alocacao
 * falhar. Lacos (u == v) sao permitidos: um laco de peso negativo e um
 * ciclo negativo. */
int inserir_arco_ponderado(GrafoListaPonderado *g, int u, int v, int peso);

/* Libera todo o grafo. Aceita g == NULL sem efeito. */
void liberar_grafo_ponderado(GrafoListaPonderado *g);

#endif
