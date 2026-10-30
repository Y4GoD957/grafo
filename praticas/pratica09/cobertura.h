#ifndef COBERTURA_H
#define COBERTURA_H

/* Grafo nao dirigido simples (sem lacos nem arestas paralelas) por matriz
 * de adjacencia. Vertices numerados de 0 a n-1; adj[u * n + v] == 1 se
 * {u, v} e uma aresta (a matriz e sempre simetrica). */
typedef struct {
    int n;
    unsigned char *adj;
} GrafoCobertura;

/* Uma cobertura e um vetor de n flags: cobertura[v] != 0 indica que v
 * pertence ao conjunto. */

/* Cria um grafo com n vertices e nenhuma aresta (n == 0 e valido). Retorna
 * NULL se n < 0 ou se a alocacao falhar. */
GrafoCobertura *criar_grafo_cobertura(int n);

/* Insere a aresta {u, v}. Retorna 0 em sucesso, -1 se o grafo for nulo,
 * algum vertice for invalido, u == v ou a aresta ja existir. */
int inserir_aresta_cobertura(GrafoCobertura *g, int u, int v);

/* Retorna o numero de arestas, ou -1 se g for nulo. */
int contar_arestas_cobertura(const GrafoCobertura *g);

/* Libera o grafo. Aceita g == NULL sem efeito. */
void liberar_grafo_cobertura(GrafoCobertura *g);

/* Retorna 1 se toda aresta de g tem ao menos um extremo marcado em
 * cobertura, 0 caso contrario, ou -1 se g ou cobertura forem nulos. Um
 * grafo sem arestas e coberto por qualquer conjunto, inclusive o vazio. */
int eh_cobertura(const GrafoCobertura *g, const int *cobertura);

/* Heuristica gulosa: enquanto houver aresta descoberta, escolhe o vertice
 * de maior grau considerando apenas as arestas ainda descobertas (empate
 * pelo menor indice). Grava a cobertura (n flags) em cobertura e retorna o
 * seu tamanho, ou -1 se g ou cobertura forem nulos ou a alocacao falhar
 * (cobertura fica inalterada). O grafo nao e alterado. O resultado e
 * sempre uma cobertura valida, mas nem sempre minima. */
int cobertura_gulosa(const GrafoCobertura *g, int *cobertura);

/* Cobertura de vertices minima exata por backtracking. Ramifica na
 * primeira aresta {u, v} descoberta (ou u entra, ou u fica de fora e todos
 * os seus vizinhos entram) e poda ramos cujo tamanho parcial somado a um
 * limite inferior (emparelhamento maximo guloso nas arestas descobertas)
 * nao melhora a melhor solucao conhecida, inicializada com a heuristica
 * gulosa. Grava uma cobertura minima (n flags) em cobertura e retorna o
 * seu tamanho, ou -1 se g ou cobertura forem nulos ou a alocacao falhar
 * (cobertura fica inalterada). Complexidade exponencial no pior caso. */
int cobertura_minima_bt(const GrafoCobertura *g, int *cobertura);

#endif
