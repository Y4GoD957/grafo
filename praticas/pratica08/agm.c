#include <stdlib.h>

#include "agm.h"

/* ---------- Union-Find ---------- */

DSU *criar_dsu(int n)
{
    if (n < 0) {
        return NULL;
    }

    DSU *d = malloc(sizeof(DSU));
    if (d == NULL) {
        return NULL;
    }

    d->n = n;
    d->pai = NULL;
    d->rank = NULL;

    if (n > 0) {
        d->pai = malloc((size_t)n * sizeof(int));
        d->rank = calloc((size_t)n, sizeof(int));
        if (d->pai == NULL || d->rank == NULL) {
            liberar_dsu(d);
            return NULL;
        }
        for (int i = 0; i < n; i++) {
            d->pai[i] = i;
        }
    }

    return d;
}

void liberar_dsu(DSU *d)
{
    if (d == NULL) {
        return;
    }

    free(d->pai);
    free(d->rank);
    free(d);
}

int find(DSU *d, int x)
{
    if (d == NULL || x < 0 || x >= d->n) {
        return -1;
    }

    int raiz = x;
    while (d->pai[raiz] != raiz) {
        raiz = d->pai[raiz];
    }

    /* compressao de caminho: todo vertice visitado aponta para a raiz */
    while (d->pai[x] != raiz) {
        int prox = d->pai[x];
        d->pai[x] = raiz;
        x = prox;
    }

    return raiz;
}

int unir(DSU *d, int x, int y)
{
    int rx = find(d, x);
    int ry = find(d, y);

    if (rx < 0 || ry < 0) {
        return -1;
    }
    if (rx == ry) {
        return 0;
    }

    if (d->rank[rx] < d->rank[ry]) {
        d->pai[rx] = ry;
    } else if (d->rank[rx] > d->rank[ry]) {
        d->pai[ry] = rx;
    } else {
        d->pai[ry] = rx;
        d->rank[rx]++;
    }

    return 1;
}

/* ---------- Entrada comum ---------- */

static int entrada_valida(int n, const Aresta *arestas, int m, const Aresta *mst)
{
    if (n < 0 || m < 0 || (m > 0 && arestas == NULL) || (n > 1 && mst == NULL)) {
        return 0;
    }

    for (int i = 0; i < m; i++) {
        if (arestas[i].u < 0 || arestas[i].u >= n ||
            arestas[i].v < 0 || arestas[i].v >= n) {
            return 0;
        }
    }

    return 1;
}

/* ---------- Kruskal ---------- */

static int comparar_aresta(const void *a, const void *b)
{
    const Aresta *x = a;
    const Aresta *y = b;

    if (x->peso != y->peso) {
        return x->peso < y->peso ? -1 : 1;
    }
    if (x->u != y->u) {
        return x->u < y->u ? -1 : 1;
    }
    if (x->v != y->v) {
        return x->v < y->v ? -1 : 1;
    }
    return 0;
}

int kruskal(int n, const Aresta *arestas, int m, Aresta *mst, long long *peso_total)
{
    if (!entrada_valida(n, arestas, m, mst)) {
        return -1;
    }

    Aresta *ordenadas = NULL;
    DSU *d = criar_dsu(n);
    if (d == NULL) {
        return -1;
    }

    if (m > 0) {
        ordenadas = malloc((size_t)m * sizeof(Aresta));
        if (ordenadas == NULL) {
            liberar_dsu(d);
            return -1;
        }
        for (int i = 0; i < m; i++) {
            ordenadas[i] = arestas[i];
        }
        qsort(ordenadas, (size_t)m, sizeof(Aresta), comparar_aresta);
    }

    int escolhidas = 0;
    long long total = 0;

    for (int i = 0; i < m && escolhidas < n - 1; i++) {
        if (unir(d, ordenadas[i].u, ordenadas[i].v) == 1) {
            mst[escolhidas++] = ordenadas[i];
            total += ordenadas[i].peso;
        }
    }

    if (peso_total != NULL) {
        *peso_total = total;
    }

    free(ordenadas);
    liberar_dsu(d);
    return escolhidas;
}

/* ---------- Prim com Min-Heap ---------- */

typedef struct {
    int vertice;
    int chave;
} NoHeap;

typedef struct {
    NoHeap *dados;
    int *posicao; /* vertice -> indice no heap, ou -1 se ausente */
    int tamanho;
} MinHeap;

/* Ordem total: menor chave primeiro; empate pelo menor vertice. */
static int menor(NoHeap a, NoHeap b)
{
    return a.chave < b.chave || (a.chave == b.chave && a.vertice < b.vertice);
}

static void colocar(MinHeap *h, int i, NoHeap no)
{
    h->dados[i] = no;
    h->posicao[no.vertice] = i;
}

static void subir(MinHeap *h, int i)
{
    NoHeap no = h->dados[i];

    while (i > 0) {
        int pai = (i - 1) / 2;
        if (!menor(no, h->dados[pai])) {
            break;
        }
        colocar(h, i, h->dados[pai]);
        i = pai;
    }

    colocar(h, i, no);
}

static void descer(MinHeap *h, int i)
{
    NoHeap no = h->dados[i];

    for (;;) {
        int filho = 2 * i + 1;
        if (filho >= h->tamanho) {
            break;
        }
        if (filho + 1 < h->tamanho && menor(h->dados[filho + 1], h->dados[filho])) {
            filho++;
        }
        if (!menor(h->dados[filho], no)) {
            break;
        }
        colocar(h, i, h->dados[filho]);
        i = filho;
    }

    colocar(h, i, no);
}

static void heap_inserir(MinHeap *h, int vertice, int chave)
{
    NoHeap no = { vertice, chave };
    int i = h->tamanho++;
    colocar(h, i, no);
    subir(h, i);
}

static NoHeap heap_extrair(MinHeap *h)
{
    NoHeap menor_no = h->dados[0];
    h->posicao[menor_no.vertice] = -1;
    h->tamanho--;

    if (h->tamanho > 0) {
        colocar(h, 0, h->dados[h->tamanho]);
        descer(h, 0);
    }

    return menor_no;
}

static void heap_diminuir_chave(MinHeap *h, int vertice, int chave)
{
    int i = h->posicao[vertice];
    h->dados[i].chave = chave;
    subir(h, i);
}

int prim(int n, const Aresta *arestas, int m, int origem, Aresta *mst, long long *peso_total)
{
    if (!entrada_valida(n, arestas, m, mst) || origem < 0 || origem >= n) {
        return -1;
    }

    /* Lista de adjacencia compacta (CSR): inicio[v]..inicio[v+1]-1. */
    size_t *inicio = calloc((size_t)n + 1, sizeof(size_t));
    size_t *livre = malloc((size_t)n * sizeof(size_t));
    int *vizinho = malloc(((size_t)m * 2 + 1) * sizeof(int));
    int *peso = malloc(((size_t)m * 2 + 1) * sizeof(int));
    int *pai = malloc((size_t)n * sizeof(int));
    int *peso_pai = malloc((size_t)n * sizeof(int));
    char *na_arvore = calloc((size_t)n, sizeof(char));
    MinHeap h = { malloc((size_t)n * sizeof(NoHeap)), malloc((size_t)n * sizeof(int)), 0 };

    int escolhidas = -1;

    if (inicio && livre && vizinho && peso && pai && peso_pai && na_arvore &&
        h.dados && h.posicao) {
        for (int i = 0; i < m; i++) {
            if (arestas[i].u != arestas[i].v) {
                inicio[arestas[i].u + 1]++;
                inicio[arestas[i].v + 1]++;
            }
        }
        for (int v = 0; v < n; v++) {
            inicio[v + 1] += inicio[v];
            livre[v] = inicio[v];
            pai[v] = -1;
            peso_pai[v] = 0;
            h.posicao[v] = -1;
        }
        for (int i = 0; i < m; i++) {
            int u = arestas[i].u;
            int v = arestas[i].v;
            if (u != v) {
                vizinho[livre[u]] = v;
                peso[livre[u]++] = arestas[i].peso;
                vizinho[livre[v]] = u;
                peso[livre[v]++] = arestas[i].peso;
            }
        }

        long long total = 0;
        escolhidas = 0;
        heap_inserir(&h, origem, 0);

        while (h.tamanho > 0) {
            NoHeap atual = heap_extrair(&h);
            int v = atual.vertice;
            na_arvore[v] = 1;

            if (pai[v] >= 0) {
                Aresta escolhida = { pai[v], v, peso_pai[v] };
                mst[escolhidas++] = escolhida;
                total += peso_pai[v];
            }

            for (size_t k = inicio[v]; k < inicio[v + 1]; k++) {
                int w = vizinho[k];
                if (na_arvore[w]) {
                    continue;
                }
                if (h.posicao[w] < 0) {
                    pai[w] = v;
                    peso_pai[w] = peso[k];
                    heap_inserir(&h, w, peso[k]);
                } else if (peso[k] < h.dados[h.posicao[w]].chave) {
                    pai[w] = v;
                    peso_pai[w] = peso[k];
                    heap_diminuir_chave(&h, w, peso[k]);
                }
            }
        }

        if (peso_total != NULL) {
            *peso_total = total;
        }
    }

    free(inicio);
    free(livre);
    free(vizinho);
    free(peso);
    free(pai);
    free(peso_pai);
    free(na_arvore);
    free(h.dados);
    free(h.posicao);

    return escolhidas;
}
