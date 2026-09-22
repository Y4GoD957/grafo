#include <stdlib.h>

#include "conectividade.h"

/* Estado compartilhado pela DFS de Tarjan.
 *
 * descoberta[u]: instante (tempo) em que u foi descoberto pela DFS;
 *                -1 indica que u ainda nao foi visitado.
 * low[u]:        menor tempo de descoberta alcancavel a partir da
 *                subarvore de u usando arestas de arvore para baixo e
 *                no maximo uma aresta de retorno. Se low[v] ainda e
 *                grande em relacao a descoberta[u], a subarvore de v
 *                nao tem "atalho" para cima de u.
 * pai[u]:        pai de u na arvore da DFS (-1 para a raiz).
 *
 * eh_articulacao e pontes sao opcionais (podem ser NULL), para que a
 * mesma DFS atenda as duas funcoes publicas. */
typedef struct {
    int *descoberta;
    int *low;
    int *pai;
    int tempo;
    int *eh_articulacao;
    Ponte *pontes;
    int num_pontes;
} EstadoTarjan;

static int minimo(int a, int b)
{
    return a < b ? a : b;
}

static void dfs_tarjan(GrafoLista *g, int u, EstadoTarjan *e)
{
    e->descoberta[u] = e->tempo;
    e->low[u] = e->tempo;
    e->tempo++;

    int filhos = 0;

    for (No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
        int v = atual->destino;

        /* Um laco u--u nunca separa o grafo: nao e ponte nem ajuda low. */
        if (v == u) {
            continue;
        }

        if (e->descoberta[v] == -1) {
            /* Aresta de arvore u -> v. */
            e->pai[v] = u;
            filhos++;
            dfs_tarjan(g, v, e);

            e->low[u] = minimo(e->low[u], e->low[v]);

            /* Regra da nao-raiz: se nenhum vertice da subarvore de v
             * alcanca um ancestral estrito de u, remover u desconecta
             * essa subarvore. */
            if (e->eh_articulacao != NULL && e->pai[u] != -1 &&
                e->low[v] >= e->descoberta[u]) {
                e->eh_articulacao[u] = 1;
            }

            /* Regra da ponte: a subarvore de v nao alcanca nem u nem
             * seus ancestrais por outro caminho, logo (u, v) e a unica
             * ligacao. A desigualdade e estrita: low[v] == descoberta[u]
             * significa um ciclo passando por u. */
            if (e->pontes != NULL && e->low[v] > e->descoberta[u]) {
                Ponte p = { minimo(u, v), u < v ? v : u };
                e->pontes[e->num_pontes++] = p;
            }
        } else if (v != e->pai[u]) {
            /* Aresta de retorno para um ancestral (a aresta ao pai e a
             * propria aresta de arvore e deve ser ignorada; como o
             * GrafoLista nao admite arestas paralelas, comparar o
             * vertice basta). */
            e->low[u] = minimo(e->low[u], e->descoberta[v]);
        }
    }

    /* Regra da raiz: a raiz so e articulacao se tiver mais de um filho
     * na arvore da DFS, pois filhos distintos da raiz so se ligam por
     * ela (nao existem arestas cruzadas em DFS nao direcionada). */
    if (e->eh_articulacao != NULL && e->pai[u] == -1 && filhos > 1) {
        e->eh_articulacao[u] = 1;
    }
}

/* Executa Tarjan em todas as componentes. Retorna 0 em sucesso ou -1
 * se a alocacao falhar. */
static int executar_tarjan(GrafoLista *g, int *eh_articulacao, Ponte *pontes, int *num_pontes)
{
    size_t tamanho = (size_t)g->n * sizeof(int);
    EstadoTarjan e;

    e.descoberta = malloc(tamanho);
    e.low = malloc(tamanho);
    e.pai = malloc(tamanho);
    if (e.descoberta == NULL || e.low == NULL || e.pai == NULL) {
        free(e.descoberta);
        free(e.low);
        free(e.pai);
        return -1;
    }

    e.tempo = 0;
    e.eh_articulacao = eh_articulacao;
    e.pontes = pontes;
    e.num_pontes = 0;

    for (int v = 0; v < g->n; v++) {
        e.descoberta[v] = -1;
        e.low[v] = -1;
        e.pai[v] = -1;
        if (eh_articulacao != NULL) {
            eh_articulacao[v] = 0;
        }
    }

    /* Cada vertice nao descoberto aqui e raiz de uma nova componente. */
    for (int v = 0; v < g->n; v++) {
        if (e.descoberta[v] == -1) {
            dfs_tarjan(g, v, &e);
        }
    }

    if (num_pontes != NULL) {
        *num_pontes = e.num_pontes;
    }

    free(e.descoberta);
    free(e.low);
    free(e.pai);
    return 0;
}

static int comparar_pontes(const void *a, const void *b)
{
    const Ponte *pa = a;
    const Ponte *pb = b;

    if (pa->u != pb->u) {
        return pa->u < pb->u ? -1 : 1;
    }
    if (pa->v != pb->v) {
        return pa->v < pb->v ? -1 : 1;
    }
    return 0;
}

int dfs_articulacoes(GrafoLista *g, int *eh_articulacao)
{
    if (g == NULL) {
        return -1;
    }
    if (g->n == 0) {
        return 0;
    }
    if (eh_articulacao == NULL) {
        return -1;
    }

    if (executar_tarjan(g, eh_articulacao, NULL, NULL) != 0) {
        return -1;
    }

    int total = 0;
    for (int v = 0; v < g->n; v++) {
        total += eh_articulacao[v];
    }

    return total;
}

int detectar_pontes(GrafoLista *g, Ponte *pontes)
{
    if (g == NULL) {
        return -1;
    }
    if (g->n == 0) {
        return 0;
    }
    if (pontes == NULL) {
        return -1;
    }

    int total;
    if (executar_tarjan(g, NULL, pontes, &total) != 0) {
        return -1;
    }

    qsort(pontes, (size_t)total, sizeof(Ponte), comparar_pontes);

    return total;
}
