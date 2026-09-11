#include <stdlib.h>

#include "busca_profundidade.h"

Pilha *pilha_criar(int capacidade)
{
    if (capacidade <= 0) {
        return NULL;
    }

    Pilha *p = malloc(sizeof(Pilha));
    if (p == NULL) {
        return NULL;
    }

    p->dados = malloc((size_t)capacidade * sizeof(int));
    if (p->dados == NULL) {
        free(p);
        return NULL;
    }

    p->capacidade = capacidade;
    p->topo = 0;

    return p;
}

int pilha_vazia(const Pilha *p)
{
    return p == NULL || p->topo == 0;
}

int pilha_empilhar(Pilha *p, int valor)
{
    if (p == NULL) {
        return -1;
    }

    if (p->topo == p->capacidade) {
        int nova_capacidade = p->capacidade * 2;
        int *novos_dados = realloc(p->dados, (size_t)nova_capacidade * sizeof(int));
        if (novos_dados == NULL) {
            return -1;
        }
        p->dados = novos_dados;
        p->capacidade = nova_capacidade;
    }

    p->dados[p->topo] = valor;
    p->topo++;

    return 0;
}

int pilha_desempilhar(Pilha *p, int *valor)
{
    if (pilha_vazia(p) || valor == NULL) {
        return -1;
    }

    p->topo--;
    *valor = p->dados[p->topo];

    return 0;
}

void pilha_destruir(Pilha *p)
{
    if (p == NULL) {
        return;
    }

    free(p->dados);
    free(p);
}

void dfs_recursiva(GrafoLista *g, int u, int *visitado, int *entrada, int *saida, int *tempo)
{
    if (g == NULL || visitado == NULL || u < 0 || u >= g->n || visitado[u]) {
        return;
    }

    visitado[u] = 1;
    entrada[u] = ++(*tempo);

    for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
        int v = atual->destino;
        if (!visitado[v]) {
            dfs_recursiva(g, v, visitado, entrada, saida, tempo);
        }
    }

    saida[u] = ++(*tempo);
}

void dfs_iterativa(GrafoLista *g, int origem, int *visitado)
{
    if (g == NULL || visitado == NULL || origem < 0 || origem >= g->n) {
        return;
    }

    Pilha *pilha = pilha_criar(g->n > 0 ? g->n : 1);
    if (pilha == NULL) {
        return;
    }

    pilha_empilhar(pilha, origem);

    int u;
    while (pilha_desempilhar(pilha, &u) == 0) {
        if (visitado[u]) {
            continue;
        }
        visitado[u] = 1;

        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            int v = atual->destino;
            if (!visitado[v]) {
                pilha_empilhar(pilha, v);
            }
        }
    }

    pilha_destruir(pilha);
}

int contar_componentes(GrafoLista *g)
{
    if (g == NULL) {
        return -1;
    }

    if (g->n == 0) {
        return 0;
    }

    int *visitado = calloc((size_t)g->n, sizeof(int));
    int *entrada = malloc((size_t)g->n * sizeof(int));
    int *saida = malloc((size_t)g->n * sizeof(int));
    if (visitado == NULL || entrada == NULL || saida == NULL) {
        free(visitado);
        free(entrada);
        free(saida);
        return -1;
    }

    int tempo = 0;
    int componentes = 0;

    for (int v = 0; v < g->n; v++) {
        if (!visitado[v]) {
            componentes++;
            dfs_recursiva(g, v, visitado, entrada, saida, &tempo);
        }
    }

    free(visitado);
    free(entrada);
    free(saida);

    return componentes;
}

/* DFS auxiliar para tem_ciclo: percorre a partir de u lembrando o
 * pai usado para chegar ate ele. Um vizinho ja visitado que nao seja
 * o pai (incluindo o proprio u, no caso de um laco) indica ciclo. */
static int dfs_tem_ciclo(GrafoLista *g, int u, int pai, int *visitado)
{
    visitado[u] = 1;

    for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
        int v = atual->destino;

        if (!visitado[v]) {
            if (dfs_tem_ciclo(g, v, u, visitado)) {
                return 1;
            }
        } else if (v != pai) {
            return 1;
        }
    }

    return 0;
}

int tem_ciclo(GrafoLista *g)
{
    if (g == NULL || g->n == 0) {
        return 0;
    }

    int *visitado = calloc((size_t)g->n, sizeof(int));
    if (visitado == NULL) {
        return 0;
    }

    int achou_ciclo = 0;

    for (int v = 0; v < g->n && !achou_ciclo; v++) {
        if (!visitado[v]) {
            achou_ciclo = dfs_tem_ciclo(g, v, -1, visitado);
        }
    }

    free(visitado);

    return achou_ciclo;
}
