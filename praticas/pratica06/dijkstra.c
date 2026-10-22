#include <stdlib.h>

#include "dijkstra.h"

static int vertice_valido_heap(const MinHeap *h, int v)
{
    return v >= 0 && v < h->capacidade;
}

/* Grava o no em dados[i] e atualiza posicao[vertice]. */
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
        if (h->dados[pai].distancia <= no.distancia) {
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
        if (filho + 1 < h->tamanho &&
            h->dados[filho + 1].distancia < h->dados[filho].distancia) {
            filho++;
        }
        if (no.distancia <= h->dados[filho].distancia) {
            break;
        }
        colocar(h, i, h->dados[filho]);
        i = filho;
    }

    colocar(h, i, no);
}

MinHeap *criar_min_heap(int capacidade)
{
    if (capacidade < 0) {
        return NULL;
    }

    MinHeap *h = malloc(sizeof(MinHeap));
    if (h == NULL) {
        return NULL;
    }

    h->capacidade = capacidade;
    h->tamanho = 0;
    h->dados = NULL;
    h->posicao = NULL;

    if (capacidade > 0) {
        h->dados = malloc((size_t)capacidade * sizeof(NoHeap));
        h->posicao = malloc((size_t)capacidade * sizeof(int));
        if (h->dados == NULL || h->posicao == NULL) {
            liberar_min_heap(h);
            return NULL;
        }
        for (int i = 0; i < capacidade; i++) {
            h->posicao[i] = -1;
        }
    }

    return h;
}

void liberar_min_heap(MinHeap *h)
{
    if (h == NULL) {
        return;
    }

    free(h->dados);
    free(h->posicao);
    free(h);
}

int heap_contem(const MinHeap *h, int vertice)
{
    if (h == NULL || !vertice_valido_heap(h, vertice)) {
        return -1;
    }

    return h->posicao[vertice] != -1;
}

int inserir_min_heap(MinHeap *h, int vertice, int distancia)
{
    if (h == NULL || !vertice_valido_heap(h, vertice) ||
        h->posicao[vertice] != -1 || h->tamanho >= h->capacidade) {
        return -1;
    }

    NoHeap no = { vertice, distancia };
    int i = h->tamanho++;
    colocar(h, i, no);
    subir(h, i);

    return 0;
}

int extrair_minimo(MinHeap *h, NoHeap *saida)
{
    if (h == NULL || saida == NULL || h->tamanho == 0) {
        return -1;
    }

    *saida = h->dados[0];
    h->posicao[saida->vertice] = -1;
    h->tamanho--;

    if (h->tamanho > 0) {
        colocar(h, 0, h->dados[h->tamanho]);
        descer(h, 0);
    }

    return 0;
}

int diminuir_chave(MinHeap *h, int vertice, int nova_distancia)
{
    if (h == NULL || !vertice_valido_heap(h, vertice) ||
        h->posicao[vertice] == -1) {
        return -1;
    }

    int i = h->posicao[vertice];
    if (nova_distancia > h->dados[i].distancia) {
        return -1;
    }

    h->dados[i].distancia = nova_distancia;
    subir(h, i);

    return 0;
}

int dijkstra(GrafoListaPonderado *g, int origem, int *dist, int *pred)
{
    if (g == NULL || dist == NULL || pred == NULL ||
        origem < 0 || origem >= g->n) {
        return -1;
    }

    /* Rejeita pesos fora de [0, PESO_MAXIMO] antes de alterar dist/pred. */
    for (int u = 0; u < g->n; u++) {
        for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
            if (e->peso < 0 || e->peso > PESO_MAXIMO) {
                return -1;
            }
        }
    }

    MinHeap *h = criar_min_heap(g->n);
    if (h == NULL) {
        return -1;
    }

    for (int v = 0; v < g->n; v++) {
        dist[v] = DIJKSTRA_INF;
        pred[v] = -1;
    }

    dist[origem] = 0;
    inserir_min_heap(h, origem, 0);

    NoHeap atual;
    while (extrair_minimo(h, &atual) == 0) {
        int u = atual.vertice;

        for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
            /* evita estouro de int em dist[u] + peso */
            if (e->peso > DIJKSTRA_INF - dist[u]) {
                continue;
            }

            int nova = dist[u] + e->peso;
            if (nova < dist[e->destino]) {
                dist[e->destino] = nova;
                pred[e->destino] = u;
                if (heap_contem(h, e->destino)) {
                    diminuir_chave(h, e->destino, nova);
                } else {
                    inserir_min_heap(h, e->destino, nova);
                }
            }
        }
    }

    liberar_min_heap(h);
    return 0;
}

/* Detecta ciclo na cadeia de predecessores a partir de v (lebre e
 * tartaruga). Retorna 1 se houver ciclo. */
static int tem_ciclo(const int *pred, int v)
{
    int lenta = v;
    int rapida = v;

    for (;;) {
        if (rapida < 0 || pred[rapida] < 0) {
            return 0;
        }
        rapida = pred[pred[rapida]];
        lenta = pred[lenta];
        if (rapida >= 0 && rapida == lenta) {
            return 1;
        }
    }
}

int *reconstruir_caminho(int origem, int destino, int *pred)
{
    if (pred == NULL || origem < 0 || destino < 0) {
        return NULL;
    }

    if (origem == destino) {
        int *unico = malloc(2 * sizeof(int));
        if (unico != NULL) {
            unico[0] = origem;
            unico[1] = -1;
        }
        return unico;
    }

    if (tem_ciclo(pred, destino)) {
        return NULL;
    }

    /* Conta os vertices; a cadeia, sem ciclo, termina em -1 ou na origem. */
    int tamanho = 1;
    for (int v = destino; v != origem; tamanho++) {
        v = pred[v];
        if (v < 0) {
            return NULL;
        }
    }

    int *caminho = malloc((size_t)(tamanho + 1) * sizeof(int));
    if (caminho == NULL) {
        return NULL;
    }

    caminho[tamanho] = -1;
    int v = destino;
    for (int i = tamanho - 1; i >= 0; i--) {
        caminho[i] = v;
        v = pred[v];
    }

    return caminho;
}
