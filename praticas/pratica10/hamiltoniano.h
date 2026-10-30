#ifndef HAMILTONIANO_H
#define HAMILTONIANO_H

/* Grafo nao dirigido simples (sem lacos nem arestas paralelas) por matriz
 * de adjacencia. Vertices numerados de 0 a n-1; adj[u * n + v] == 1 se
 * {u, v} e uma aresta (a matriz e sempre simetrica). */
typedef struct {
    int n;
    unsigned char *adj;
} GrafoHamiltoniano;

/* Cria um grafo com n vertices e nenhuma aresta. Retorna NULL se n < 0, se
 * a matriz n x n nao couber em size_t ou se a alocacao falhar. */
GrafoHamiltoniano *criar_grafo_hamiltoniano(int n);

/* Insere a aresta {u, v}. Retorna 0 em sucesso, -1 se o grafo for nulo,
 * algum vertice for invalido, u == v ou a aresta ja existir. */
int inserir_aresta_hamiltoniano(GrafoHamiltoniano *g, int u, int v);

/* Retorna 1 se {u, v} e aresta, 0 se nao e, ou -1 se o grafo for nulo ou
 * algum vertice for invalido. */
int tem_aresta_hamiltoniano(const GrafoHamiltoniano *g, int u, int v);

/* Retorna o numero de arestas, ou -1 se g for nulo. */
int contar_arestas_hamiltoniano(const GrafoHamiltoniano *g);

/* Libera o grafo. Aceita g == NULL sem efeito. */
void liberar_grafo_hamiltoniano(GrafoHamiltoniano *g);

/* Procura um caminho hamiltoniano (todos os vertices exatamente uma vez,
 * com aresta entre vertices consecutivos) por backtracking, tentando cada
 * vertice como inicio e estendendo apenas por vizinhos nao visitados.
 * Retorna 1 e grava o caminho em caminho (n posicoes) se existir, 0 se nao
 * existir, ou -1 se g ou caminho forem nulos, n == 0 ou a alocacao falhar.
 * caminho so e alterado quando o retorno e 1. Para n == 1 o caminho e {0}. */
int encontrar_caminho_hamiltoniano_bt(const GrafoHamiltoniano *g, int *caminho);

/* Procura um ciclo hamiltoniano por backtracking com o vertice 0 fixado
 * como inicio. Retorna 1 e grava o ciclo em ciclo (n + 1 posicoes, fechado:
 * ciclo[n] == ciclo[0] == 0) se existir, 0 se nao existir, ou -1 se g ou
 * ciclo forem nulos, n == 0 ou a alocacao falhar. Um grafo simples com
 * menos de 3 vertices nao tem ciclo hamiltoniano (retorna 0 para n = 1, 2).
 * ciclo so e alterado quando o retorno e 1. */
int encontrar_ciclo_hamiltoniano_bt(const GrafoHamiltoniano *g, int *ciclo);

#endif
