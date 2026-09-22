#include <stdio.h>
#include <stdlib.h>

#include "conectividade.h"
#include "grafo_lista.h"
#include "planaridade.h"

#define MAX_ARESTAS 20
#define MAX_ESPERADOS 12

/* Caso de teste: grafo nao direcionado e os resultados esperados. */
typedef struct {
    const char *nome;
    int n;
    int m;
    int arestas[MAX_ARESTAS][2];
    int num_articulacoes;
    int articulacoes[MAX_ESPERADOS];
    int num_pontes;
    Ponte pontes[MAX_ESPERADOS];
    ResultadoPlanaridade planaridade;
} Caso;

static const Caso casos[] = {
    { "Vertice unico", 1, 0, { { 0, 0 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    { "Aresta unica (K2)", 2, 1, { { 0, 1 } },
      0, { 0 }, 1, { { 0, 1 } }, PLANARIDADE_PLANAR },

    { "K2 com laco em 1 (laco ignorado)", 2, 2, { { 0, 1 }, { 1, 1 } },
      0, { 0 }, 1, { { 0, 1 } }, PLANARIDADE_PLANAR },

    { "A. Caminho P4: 0-1-2-3", 4, 3, { { 0, 1 }, { 1, 2 }, { 2, 3 } },
      2, { 1, 2 }, 3, { { 0, 1 }, { 1, 2 }, { 2, 3 } }, PLANARIDADE_PLANAR },

    { "B. Ciclo C4: 0-1-2-3-0", 4, 4, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    { "C. Estrela K1,4 (centro 0)", 5, 4, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } },
      1, { 0 }, 4, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } }, PLANARIDADE_PLANAR },

    { "Triangulo 0-1-2 com cauda 2-3-4", 5, 5,
      { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 2, 3 }, { 3, 4 } },
      2, { 2, 3 }, 2, { { 2, 3 }, { 3, 4 } }, PLANARIDADE_PLANAR },

    /* Tres componentes: P3 (0-1-2), gravata borboleta (triangulos 3-4-5
     * e 5-6-7 unidos em 5) e a aresta 8-9. A DFS da segunda componente
     * comeca em 3, entao 5 so e detectado pela regra da nao-raiz. */
    { "G. Desconexo: P3 + gravata + K2", 10, 9,
      { { 0, 1 }, { 1, 2 },
        { 3, 4 }, { 4, 5 }, { 5, 3 }, { 5, 6 }, { 6, 7 }, { 7, 5 },
        { 8, 9 } },
      2, { 1, 5 }, 3, { { 0, 1 }, { 1, 2 }, { 8, 9 } }, PLANARIDADE_PLANAR },

    { "K4 completo", 4, 6, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    { "D. K5 completo", 5, 10,
      { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 1, 2 },
        { 1, 3 }, { 1, 4 }, { 2, 3 }, { 2, 4 }, { 3, 4 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_EULER },

    { "E. K3,3 (A = {0,1,2}, B = {3,4,5})", 6, 9,
      { { 0, 3 }, { 0, 4 }, { 0, 5 }, { 1, 3 }, { 1, 4 },
        { 1, 5 }, { 2, 3 }, { 2, 4 }, { 2, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_K33 },

    /* K5 com a aresta 0-1 trocada pelo caminho 0-5-1: passa em Euler
     * (m = 11 <= 12), mas contem subdivisao de K5. */
    { "F. K5 subdividido (0-1 vira 0-5-1)", 6, 11,
      { { 0, 5 }, { 5, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 1, 2 },
        { 1, 3 }, { 1, 4 }, { 2, 3 }, { 2, 4 }, { 3, 4 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_K5 },

    /* K3,3 com 0-3 trocada por 0-6-3 e 1-4 trocada por 1-7-8-4. */
    { "F. K3,3 subdividido (0-6-3 e 1-7-8-4)", 9, 12,
      { { 0, 6 }, { 6, 3 }, { 0, 4 }, { 0, 5 }, { 1, 3 }, { 1, 7 },
        { 7, 8 }, { 8, 4 }, { 1, 5 }, { 2, 3 }, { 2, 4 }, { 2, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_K33 },

    { "K6 completo (viola Euler: 15 > 12)", 6, 15,
      { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 2, 3 },
        { 2, 4 }, { 2, 5 }, { 3, 4 }, { 3, 5 }, { 4, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_EULER },

    /* Petersen: n = 10, passa em Euler (15 <= 24) e todo vertice tem
     * grau 3, logo nao ha subdivisao de K5; contem subdivisao de K3,3. */
    { "Petersen (n = 10)", 10, 15,
      { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 0 },
        { 0, 5 }, { 1, 6 }, { 2, 7 }, { 3, 8 }, { 4, 9 },
        { 5, 7 }, { 7, 9 }, { 9, 6 }, { 6, 8 }, { 8, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_NAO_PLANAR_K33 },

    /* Octaedro: planar maximal, m = 3n - 6 exatamente. */
    { "Octaedro (m = 3n - 6)", 6, 12,
      { { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 }, { 1, 2 }, { 1, 3 },
        { 1, 4 }, { 1, 5 }, { 2, 4 }, { 2, 5 }, { 3, 4 }, { 3, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    /* Prisma triangular: 6 vertices de grau 3 como K3,3, mas planar.
     * Um teste baseado apenas em graus acusaria falso positivo. */
    { "Prisma triangular", 6, 9,
      { { 0, 1 }, { 1, 2 }, { 2, 0 }, { 3, 4 }, { 4, 5 },
        { 5, 3 }, { 0, 3 }, { 1, 4 }, { 2, 5 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    { "Roda W5 (centro 0 + ciclo 1..5)", 6, 10,
      { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 1 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    { "Desconexo planar: K4 + C3", 7, 9,
      { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 },
        { 4, 5 }, { 5, 6 }, { 6, 4 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_PLANAR },

    /* n > 10: so Euler e aplicado; sem a busca de Kuratowski nao ha
     * conclusao, mesmo o ciclo sendo planar. */
    { "Ciclo C12 (n > 10)", 12, 12,
      { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 },
        { 6, 7 }, { 7, 8 }, { 8, 9 }, { 9, 10 }, { 10, 11 }, { 11, 0 } },
      0, { 0 }, 0, { { 0, 0 } }, PLANARIDADE_INCONCLUSIVO },
};

static const char *descrever_planaridade(ResultadoPlanaridade r)
{
    switch (r) {
    case PLANARIDADE_NAO_PLANAR_EULER:
        return "NAO planar (viola m <= 3n - 6)";
    case PLANARIDADE_NAO_PLANAR_K5:
        return "NAO planar (contem subdivisao de K5)";
    case PLANARIDADE_NAO_PLANAR_K33:
        return "NAO planar (contem subdivisao de K3,3)";
    case PLANARIDADE_PLANAR:
        return "planar (passa em Euler e busca exaustiva sem K5/K3,3)";
    case PLANARIDADE_INCONCLUSIVO:
        return "inconclusivo (passa na condicao necessaria de Euler; "
               "Kuratowski nao executado para n > 10)";
    default:
        return "erro";
    }
}

static GrafoLista *montar_grafo(const Caso *c)
{
    GrafoLista *g = criar_grafo_lista(c->n);
    if (g == NULL) {
        return NULL;
    }

    for (int i = 0; i < c->m; i++) {
        if (inserir_aresta_lista(g, c->arestas[i][0], c->arestas[i][1]) != 0) {
            liberar_grafo_lista(g);
            return NULL;
        }
    }

    return g;
}

static void imprimir_vertices(const int *vertices, int quantidade)
{
    if (quantidade == 0) {
        printf("(nenhum)");
    }
    for (int i = 0; i < quantidade; i++) {
        printf("%d%s", vertices[i], i + 1 < quantidade ? ", " : "");
    }
}

static void imprimir_pontes(const Ponte *pontes, int quantidade)
{
    if (quantidade == 0) {
        printf("(nenhuma)");
    }
    for (int i = 0; i < quantidade; i++) {
        printf("(%d,%d)%s", pontes[i].u, pontes[i].v, i + 1 < quantidade ? ", " : "");
    }
}

static const char *marca(int ok)
{
    return ok ? "[OK]" : "[FALHOU]";
}

/* Executa todos os algoritmos sobre o caso e compara com o esperado.
 * Retorna o numero de verificacoes que falharam. */
static int executar_caso(const Caso *c)
{
    printf("=== %s ===\n", c->nome);

    GrafoLista *g = montar_grafo(c);
    int *eh_articulacao = malloc((size_t)c->n * sizeof(int));
    int *articulacoes = malloc((size_t)c->n * sizeof(int));
    Ponte *pontes = malloc((size_t)c->n * sizeof(Ponte));
    if (g == NULL || eh_articulacao == NULL || articulacoes == NULL || pontes == NULL) {
        printf("Falha ao montar o caso.\n\n");
        liberar_grafo_lista(g);
        free(eh_articulacao);
        free(articulacoes);
        free(pontes);
        return 1;
    }

    int falhas = 0;
    int m = contar_arestas(g);
    printf("n = %d, m = %d\n", g->n, m);

    /* Articulacoes */
    dfs_articulacoes(g, eh_articulacao);
    int num_articulacoes = 0;
    for (int v = 0; v < g->n; v++) {
        if (eh_articulacao[v]) {
            articulacoes[num_articulacoes++] = v;
        }
    }

    int ok = num_articulacoes == c->num_articulacoes;
    for (int i = 0; ok && i < num_articulacoes; i++) {
        ok = articulacoes[i] == c->articulacoes[i];
    }
    printf("Articulacoes: ");
    imprimir_vertices(articulacoes, num_articulacoes);
    printf(" | esperado: ");
    imprimir_vertices(c->articulacoes, c->num_articulacoes);
    printf(" %s\n", marca(ok));
    falhas += !ok;

    /* Pontes */
    int num_pontes = detectar_pontes(g, pontes);
    ok = num_pontes == c->num_pontes;
    for (int i = 0; ok && i < num_pontes; i++) {
        ok = pontes[i].u == c->pontes[i].u && pontes[i].v == c->pontes[i].v;
    }
    printf("Pontes:       ");
    imprimir_pontes(pontes, num_pontes);
    printf(" | esperado: ");
    imprimir_pontes(c->pontes, c->num_pontes);
    printf(" %s\n", marca(ok));
    falhas += !ok;

    /* Planaridade */
    printf("Euler (m <= 3n - 6): %s\n",
           eh_planar_euler(g) ? "passa na condicao necessaria" : "violada");

    if (g->n <= LIMITE_KURATOWSKI) {
        int ramos[6];
        if (tem_subdivisao_k5(g, ramos) == 1) {
            printf("Subdivisao de K5: sim, ramos {");
            imprimir_vertices(ramos, 5);
            printf("}\n");
        } else {
            printf("Subdivisao de K5: nao\n");
        }
        if (tem_subdivisao_k33(g, ramos) == 1) {
            printf("Subdivisao de K3,3: sim, lados {");
            imprimir_vertices(ramos, 3);
            printf("} e {");
            imprimir_vertices(ramos + 3, 3);
            printf("}\n");
        } else {
            printf("Subdivisao de K3,3: nao\n");
        }
    } else {
        printf("Busca de Kuratowski nao executada (n > %d)\n", LIMITE_KURATOWSKI);
    }

    ResultadoPlanaridade r = verificar_planaridade(g);
    ok = r == c->planaridade;
    printf("Conclusao: %s %s\n", descrever_planaridade(r), marca(ok));
    if (!ok) {
        printf("  esperado: %s\n", descrever_planaridade(c->planaridade));
    }
    falhas += !ok;

    printf("\n");

    liberar_grafo_lista(g);
    free(eh_articulacao);
    free(articulacoes);
    free(pontes);
    return falhas;
}

int main(void)
{
    int total_casos = (int)(sizeof(casos) / sizeof(casos[0]));
    int falhas = 0;

    for (int i = 0; i < total_casos; i++) {
        falhas += executar_caso(&casos[i]);
    }

    printf("========================================\n");
    printf("Casos: %d | Verificacoes com falha: %d\n", total_casos, falhas);

    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
