#include <stdio.h>
#include <stdlib.h>

#include "grafo_lista.h"
#include "busca_largura.h"
#include "busca_profundidade.h"

static void separador(const char *titulo)
{
    printf("\n--- %s ---\n", titulo);
}

/* Cria um grafo com n vertices e insere as arestas do vetor
 * arestas[i] = {u, v}. Encerra o programa se a criacao falhar. */
static GrafoLista *montar_grafo(int n, int arestas[][2], int total_arestas)
{
    GrafoLista *g = criar_grafo_lista(n);
    if (g == NULL) {
        fprintf(stderr, "Falha ao criar grafo com %d vertices.\n", n);
        exit(1);
    }

    for (int i = 0; i < total_arestas; i++) {
        inserir_aresta_lista(g, arestas[i][0], arestas[i][1]);
    }

    return g;
}

static void demonstrar_bfs(GrafoLista *g, int origem)
{
    int *dist = malloc((size_t)g->n * sizeof(int));
    int *pred = malloc((size_t)g->n * sizeof(int));
    if (dist == NULL || pred == NULL) {
        fprintf(stderr, "Falha ao alocar memoria para BFS.\n");
        free(dist);
        free(pred);
        return;
    }

    bfs(g, origem, dist, pred);

    printf("BFS a partir do vertice %d:\n", origem);
    printf("%-10s%-12s%-12s\n", "vertice", "distancia", "predecessor");
    for (int v = 0; v < g->n; v++) {
        if (dist[v] == BFS_SEM_DISTANCIA) {
            printf("%-10d%-12s%-12s\n", v, "inf", "-");
        } else {
            printf("%-10d%-12d%-12d\n", v, dist[v],
                   pred[v] == BFS_SEM_PREDECESSOR ? -1 : pred[v]);
        }
    }

    free(dist);
    free(pred);
}

static void demonstrar_dfs(GrafoLista *g)
{
    int *visitado = calloc((size_t)g->n, sizeof(int));
    int *entrada = malloc((size_t)g->n * sizeof(int));
    int *saida = malloc((size_t)g->n * sizeof(int));
    if (visitado == NULL || entrada == NULL || saida == NULL) {
        fprintf(stderr, "Falha ao alocar memoria para DFS.\n");
        free(visitado);
        free(entrada);
        free(saida);
        return;
    }

    int tempo = 0;
    printf("DFS recursiva (percorrendo todas as raizes desconexas):\n");
    for (int v = 0; v < g->n; v++) {
        if (!visitado[v]) {
            printf("  Nova raiz de componente: %d\n", v);
            dfs_recursiva(g, v, visitado, entrada, saida, &tempo);
        }
    }

    printf("%-10s%-10s%-10s\n", "vertice", "entrada", "saida");
    for (int v = 0; v < g->n; v++) {
        printf("%-10d%-10d%-10d\n", v, entrada[v], saida[v]);
    }

    free(visitado);
    free(entrada);
    free(saida);
}

static void demonstrar_dfs_iterativa(GrafoLista *g, int origem)
{
    int *visitado = calloc((size_t)g->n, sizeof(int));
    if (visitado == NULL) {
        fprintf(stderr, "Falha ao alocar memoria para DFS iterativa.\n");
        return;
    }

    dfs_iterativa(g, origem, visitado);

    printf("DFS iterativa (com Pilha) a partir de %d - vertices visitados: ", origem);
    for (int v = 0; v < g->n; v++) {
        if (visitado[v]) {
            printf("%d ", v);
        }
    }
    printf("\n");

    free(visitado);
}

int main(void)
{
    separador("Grafo conexo (arvore + aresta extra em 5 vertices)");
    {
        int arestas[][2] = { {0, 1}, {0, 2}, {1, 3}, {1, 4} };
        GrafoLista *g = montar_grafo(5, arestas, 4);

        demonstrar_bfs(g, 0);
        demonstrar_dfs(g);
        demonstrar_dfs_iterativa(g, 0);
        printf("Componentes conexas: %d (esperado: 1)\n", contar_componentes(g));
        printf("Possui ciclo: %s (esperado: NAO)\n", tem_ciclo(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    separador("Grafo desconexo: A-B, C-D, E (3 componentes)");
    {
        /* vertices: 0=A 1=B 2=C 3=D 4=E */
        int arestas[][2] = { {0, 1}, {2, 3} };
        GrafoLista *g = montar_grafo(5, arestas, 2);

        demonstrar_bfs(g, 0);
        printf("BFS a partir de E (vertice 4, isolado):\n");
        demonstrar_bfs(g, 4);
        printf("Componentes conexas: %d (esperado: 3)\n", contar_componentes(g));

        liberar_grafo_lista(g);
    }

    separador("Grafo aciclico (arvore)");
    {
        int arestas[][2] = { {0, 1}, {0, 2}, {2, 3}, {2, 4} };
        GrafoLista *g = montar_grafo(5, arestas, 4);

        printf("Possui ciclo: %s (esperado: NAO)\n", tem_ciclo(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    separador("Grafo ciclico (arvore anterior + aresta 3-4)");
    {
        int arestas[][2] = { {0, 1}, {0, 2}, {2, 3}, {2, 4}, {3, 4} };
        GrafoLista *g = montar_grafo(5, arestas, 5);

        printf("Possui ciclo: %s (esperado: SIM)\n", tem_ciclo(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    separador("Grafo bipartido: quadrado A-B-C-D-A");
    {
        /* vertices: 0=A 1=B 2=C 3=D */
        int arestas[][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
        GrafoLista *g = montar_grafo(4, arestas, 4);

        printf("Eh bipartido: %s (esperado: SIM)\n", eh_bipartido(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    separador("Grafo nao bipartido: triangulo A-B-C");
    {
        int arestas[][2] = { {0, 1}, {1, 2}, {2, 0} };
        GrafoLista *g = montar_grafo(3, arestas, 3);

        printf("Eh bipartido: %s (esperado: NAO)\n", eh_bipartido(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    separador("Grafo desconexo com componente bipartida e componente nao bipartida");
    {
        /* vertices 0-3: quadrado bipartido; vertices 4-6: triangulo (nao bipartido) */
        int arestas[][2] = { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {4, 5}, {5, 6}, {6, 4} };
        GrafoLista *g = montar_grafo(7, arestas, 7);

        printf("Componentes conexas: %d (esperado: 2)\n", contar_componentes(g));
        printf("Eh bipartido (geral): %s (esperado: NAO, por causa do triangulo)\n",
               eh_bipartido(g) ? "SIM" : "NAO");

        liberar_grafo_lista(g);
    }

    return 0;
}
