#include <stdio.h>
#include <stdlib.h>

#include "dag.h"
#include "grafo_lista.h"

static void imprimir_ordem(const char *rotulo, const int *ordem, int tamanho)
{
    printf("%s: ", rotulo);

    if (ordem == NULL || tamanho == 0) {
        printf("(vazio)\n");
        return;
    }

    for (int i = 0; i < tamanho; i++) {
        printf("%d%s", ordem[i], i + 1 < tamanho ? " -> " : "\n");
    }
}

/* Confere se toda aresta u->v de g respeita posicao(u) < posicao(v)
 * na ordenacao produzida. Serve apenas como validacao local. */
static int ordem_e_valida(const GrafoLista *g, const int *ordem, int tamanho)
{
    int *posicao = malloc((size_t)tamanho * sizeof(int));
    if (posicao == NULL) {
        return 0;
    }

    for (int i = 0; i < tamanho; i++) {
        posicao[ordem[i]] = i;
    }

    int valida = 1;
    for (int u = 0; u < g->n && valida; u++) {
        for (No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            if (posicao[u] >= posicao[atual->destino]) {
                valida = 0;
                break;
            }
        }
    }

    free(posicao);
    return valida;
}

static void demonstrar_dag(void)
{
    printf("=== Grafo A: DAG valido ===\n");

    GrafoLista *g = criar_grafo_lista(6);

    inserir_arco_dirigido_lista(g, 5, 2);
    inserir_arco_dirigido_lista(g, 5, 0);
    inserir_arco_dirigido_lista(g, 4, 0);
    inserir_arco_dirigido_lista(g, 4, 1);
    inserir_arco_dirigido_lista(g, 2, 3);
    inserir_arco_dirigido_lista(g, 3, 1);

    printf("eh_dag: %s\n", eh_dag(g) ? "sim" : "nao");

    int tamanho_kahn;
    int *ordem_kahn = ordenacao_topologica_kahn(g, &tamanho_kahn);
    imprimir_ordem("Ordenacao (Kahn)", ordem_kahn, tamanho_kahn);
    if (ordem_kahn != NULL) {
        printf("Ordenacao (Kahn) valida: %s\n",
               ordem_e_valida(g, ordem_kahn, tamanho_kahn) ? "sim" : "nao");
    }
    free(ordem_kahn);

    int tamanho_dfs;
    int *ordem_dfs = ordenacao_topologica_dfs(g, &tamanho_dfs);
    imprimir_ordem("Ordenacao (DFS) ", ordem_dfs, tamanho_dfs);
    if (ordem_dfs != NULL) {
        printf("Ordenacao (DFS) valida: %s\n",
               ordem_e_valida(g, ordem_dfs, tamanho_dfs) ? "sim" : "nao");
    }
    free(ordem_dfs);

    liberar_grafo_lista(g);
    printf("\n");
}

static void demonstrar_ciclo(void)
{
    printf("=== Grafo B: digrafo com ciclo ===\n");

    GrafoLista *g = criar_grafo_lista(3);

    inserir_arco_dirigido_lista(g, 0, 1);
    inserir_arco_dirigido_lista(g, 1, 2);
    inserir_arco_dirigido_lista(g, 2, 0);

    if (!eh_dag(g)) {
        printf("O grafo possui ciclo. Ordenacao topologica impossivel.\n");
    }

    int tamanho_kahn;
    int *ordem_kahn = ordenacao_topologica_kahn(g, &tamanho_kahn);
    printf("Kahn retornou NULL: %s (tamanho = %d)\n",
           ordem_kahn == NULL ? "sim" : "nao", tamanho_kahn);
    free(ordem_kahn);

    int tamanho_dfs;
    int *ordem_dfs = ordenacao_topologica_dfs(g, &tamanho_dfs);
    printf("DFS retornou NULL: %s (tamanho = %d)\n",
           ordem_dfs == NULL ? "sim" : "nao", tamanho_dfs);
    free(ordem_dfs);

    liberar_grafo_lista(g);
    printf("\n");
}

static void demonstrar_casos_extremos(void)
{
    printf("=== Casos extremos ===\n");

    GrafoLista *unico = criar_grafo_lista(1);
    int tamanho;
    int *ordem = ordenacao_topologica_kahn(unico, &tamanho);
    printf("Vertice unico sem arestas -> eh_dag: %s, tamanho: %d\n",
           eh_dag(unico) ? "sim" : "nao", tamanho);
    free(ordem);
    liberar_grafo_lista(unico);

    GrafoLista *desconexo = criar_grafo_lista(5);
    inserir_arco_dirigido_lista(desconexo, 0, 1);
    inserir_arco_dirigido_lista(desconexo, 3, 4);
    ordem = ordenacao_topologica_dfs(desconexo, &tamanho);
    imprimir_ordem("Componentes desconexos (DFS)", ordem, tamanho);
    free(ordem);
    liberar_grafo_lista(desconexo);

    printf("\n");
}

int main(void)
{
    demonstrar_dag();
    demonstrar_ciclo();
    demonstrar_casos_extremos();

    return 0;
}
