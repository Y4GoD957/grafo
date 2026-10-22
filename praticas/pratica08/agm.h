#ifndef AGM_H
#define AGM_H

/* Arvore geradora minima (AGM) de grafos nao dirigidos ponderados.
 * Vertices numerados de 0 a n-1. O grafo e dado por uma lista de arestas
 * (Aresta); cada aresta {u, v, peso} e nao dirigida. Pesos podem ser
 * qualquer int (inclusive negativos ou zero); arestas paralelas e lacos
 * sao aceitos (lacos nunca entram na AGM). */

/* Union-Find: pai[] com compressao de caminho, rank[] para uniao por rank. */
typedef struct {
    int *pai, *rank;
    int n;
} DSU;

typedef struct {
    int u, v, peso;
} Aresta;

/* Cria um DSU com n conjuntos unitarios. Retorna NULL se n < 0 ou se a
 * alocacao falhar. Libere com liberar_dsu. */
DSU *criar_dsu(int n);

/* Libera o DSU. Aceita d == NULL sem efeito. */
void liberar_dsu(DSU *d);

/* Retorna o representante do conjunto de x, comprimindo o caminho (todos
 * os vertices visitados passam a apontar diretamente para a raiz).
 * Retorna -1 se d for nulo ou x for invalido. */
int find(DSU *d, int x);

/* Une os conjuntos de x e y por rank. Retorna 1 se uniu, 0 se ja estavam
 * no mesmo conjunto, -1 se d for nulo ou algum vertice for invalido. */
int unir(DSU *d, int x, int y);

/* Kruskal: ordena as arestas por (peso, u, v) e escolhe as que ligam
 * componentes distintas (O(E log E)). Grava a AGM (ou a floresta geradora
 * minima, se o grafo for desconexo) em mst, que deve ter espaco para pelo
 * menos n-1 arestas (nenhuma se n <= 1), e o peso total em *peso_total
 * (long long, sem overflow; pode ser NULL). Nao altera o vetor arestas.
 * Retorna o numero de arestas escolhidas (n-1 se o grafo for conexo) ou
 * -1 se n < 0, m < 0, arestas for nulo com m > 0, mst for nulo com n > 1
 * ou alguma aresta tiver vertice invalido; nesse caso mst e *peso_total
 * nao sao alterados. Desempates sao deterministicos. */
int kruskal(int n, const Aresta *arestas, int m, Aresta *mst, long long *peso_total);

/* Prim com Min-Heap e diminuicao de chave (O((V+E) log V)), a partir de
 * origem. Cresce a arvore apenas na componente conexa da origem: grava
 * ate n-1 arestas em mst (mesmas regras de espaco que em kruskal) e o peso
 * total em *peso_total (pode ser NULL). Cada aresta gravada tem u = pai
 * na arvore e v = vertice adicionado. Retorna o numero de arestas (o
 * tamanho da componente menos 1) ou -1 nas mesmas condicoes de kruskal,
 * ou se origem estiver fora de [0, n-1] ou a alocacao falhar. */
int prim(int n, const Aresta *arestas, int m, int origem, Aresta *mst, long long *peso_total);

#endif
