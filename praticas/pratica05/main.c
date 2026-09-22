#include <stdio.h>
#include <stdlib.h>

#include "coloracao.h"
#include "grafo_lista.h"

#define MAX_ARESTAS 16
#define MAX_VERTICES 10

/* Caso de teste: grafo nao direcionado e os resultados esperados.
 * cores_gulosa/cores_wp so sao conferidas posicao a posicao quando
 * conferir_vetores == 1; nos demais casos, confere-se a quantidade de
 * cores e a validade da coloracao. num_cores == -1 indica que se espera
 * a rejeicao do grafo (laco). */
typedef struct {
    const char *nome;
    int n;
    int m;
    int arestas[MAX_ARESTAS][2];
    int num_cores_gulosa;
    int num_cores_wp;
    int bipartido;
    int conferir_vetores;
    int cores_gulosa[MAX_VERTICES];
    int cores_wp[MAX_VERTICES];
} Caso;

static const Caso casos[] = {
    { "A. Vertice unico", 1, 0, { { 0, 0 } },
      1, 1, 1, 1, { 1 }, { 1 } },

    { "B. Quatro vertices isolados", 4, 0, { { 0, 0 } },
      1, 1, 1, 1, { 1, 1, 1, 1 }, { 1, 1, 1, 1 } },

    { "C. K2", 2, 1, { { 0, 1 } },
      2, 2, 1, 1, { 1, 2 }, { 1, 2 } },

    /* Welsh-Powell visita 1, 2 (grau 2) e depois 0, 3 (grau 1). */
    { "D. Caminho P4: 0-1-2-3", 4, 3, { { 0, 1 }, { 1, 2 }, { 2, 3 } },
      2, 2, 1, 1, { 1, 2, 1, 2 }, { 2, 1, 2, 1 } },

    { "E. Ciclo C4", 4, 4, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 } },
      2, 2, 1, 0, { 0 }, { 0 } },

    { "F. Ciclo C5 (impar)", 5, 5, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 0 } },
      3, 3, 0, 0, { 0 }, { 0 } },

    { "G. Triangulo K3", 3, 3, { { 0, 1 }, { 1, 2 }, { 2, 0 } },
      3, 3, 0, 0, { 0 }, { 0 } },

    { "H. K4", 4, 6, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } },
      4, 4, 0, 0, { 0 }, { 0 } },

    { "I. Estrela K1,4 (centro 0)", 5, 4, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 } },
      2, 2, 1, 0, { 0 }, { 0 } },

    /* Caminho 0-1-2, ciclo par 3-4-5-6 e vertice isolado 7. */
    { "J. Desconexo bipartido: P3 + C4 + isolado", 8, 6,
      { { 0, 1 }, { 1, 2 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 3 } },
      2, 2, 1, 0, { 0 }, { 0 } },

    /* O vertice 0 esta numa componente bipartida (0-1); o ciclo impar
     * 2-3-4 so e visto se a BFS partir de todas as componentes. */
    { "K. Desconexo com ciclo impar: K2 + K3", 5, 4,
      { { 0, 1 }, { 2, 3 }, { 3, 4 }, { 4, 2 } },
      3, 3, 0, 0, { 0 }, { 0 } },

    /* 0 -- 1 -- 3 -- 2, com 4 isolado.
     * Ordem natural 0,1,2,3,4: 0=1, 1=2, 2=1, e 3 ve as cores 1 e 2 -> 3.
     * Graus 1,2,1,2,0 -> Welsh-Powell visita 1,3,0,2,4:
     * 1=1, 3=2, 0=2, 2=1, 4=1 -> 2 cores. Os vetores exatos distinguem
     * essa ordem da ordem natural e do desempate invertido (3 antes de 1). */
    { "L. Melhora do Welsh-Powell: 0-1-3-2 + isolado 4", 5, 3,
      { { 0, 1 }, { 1, 3 }, { 2, 3 } },
      3, 2, 1, 1, { 1, 2, 1, 3, 1 }, { 2, 1, 1, 2, 1 } },

    { "M. K5", 5, 10,
      { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 1, 2 },
        { 1, 3 }, { 1, 4 }, { 2, 3 }, { 2, 4 }, { 3, 4 } },
      5, 5, 0, 0, { 0 }, { 0 } },

    /* Coroa: K4,4 sem emparelhamento perfeito, com lados intercalados
     * (pares x impares, sem as arestas 2i -- 2i+1). E bipartida, mas a
     * ordem natural usa 4 cores; todos tem grau 3, entao o desempate
     * mantem a mesma ordem e Welsh-Powell tambem usa 4. Ambos continuam
     * sendo heuristicas. */
    { "Coroa S4 (heuristicas longe do otimo 2)", 8, 12,
      { { 0, 3 }, { 0, 5 }, { 0, 7 }, { 2, 1 }, { 2, 5 }, { 2, 7 },
        { 4, 1 }, { 4, 3 }, { 4, 7 }, { 6, 1 }, { 6, 3 }, { 6, 5 } },
      4, 4, 1, 1, { 1, 1, 2, 2, 3, 3, 4, 4 }, { 1, 1, 2, 2, 3, 3, 4, 4 } },

    /* Laco: nenhuma coloracao propria existe e o grafo nao e bipartido. */
    { "Laco em 1 (0-1 e 1-1)", 2, 2, { { 0, 1 }, { 1, 1 } },
      -1, -1, 0, 0, { 0 }, { 0 } },
};

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

/* Uma coloracao e valida quando toda cor esta em 1..num_cores, a maior
 * cor usada e num_cores e, para toda aresta (u, v), cores[u] != cores[v]. */
static int coloracao_valida(const GrafoLista *g, const int *cores, int num_cores)
{
    int maior = 0;

    for (int u = 0; u < g->n; u++) {
        if (cores[u] < 1 || cores[u] > num_cores) {
            return 0;
        }
        if (cores[u] > maior) {
            maior = cores[u];
        }
        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            if (cores[u] == cores[atual->destino]) {
                return 0;
            }
        }
    }

    return maior == num_cores;
}

static void imprimir_cores(const int *cores, int n)
{
    for (int v = 0; v < n; v++) {
        printf("%d=%d%s", v, cores[v], v + 1 < n ? " " : "");
    }
}

static const char *marca(int ok)
{
    return ok ? "[OK]" : "[FALHOU]";
}

/* Confere uma coloracao contra o esperado. Retorna 1 se tudo confere. */
static int conferir_coloracao(const char *rotulo, const GrafoLista *g, const int *cores,
                              int num_cores, int esperado, int conferir_vetor,
                              const int *vetor_esperado)
{
    if (esperado == -1) {
        int ok = cores == NULL && num_cores == -1;
        printf("%s: rejeitado (laco nao admite coloracao propria) %s\n", rotulo, marca(ok));
        return ok;
    }

    if (cores == NULL) {
        printf("%s: retornou NULL (num_cores = %d) %s\n", rotulo, num_cores, marca(0));
        return 0;
    }

    int valida = coloracao_valida(g, cores, num_cores);
    int ok = valida && num_cores == esperado;
    if (ok && conferir_vetor) {
        for (int v = 0; v < g->n; v++) {
            ok = ok && cores[v] == vetor_esperado[v];
        }
    }

    printf("%s: %d cor(es) [", rotulo, num_cores);
    imprimir_cores(cores, g->n);
    printf("] valida: %s | esperado: %d cor(es)%s %s\n",
           valida ? "sim" : "NAO", esperado, conferir_vetor ? " e vetor exato" : "",
           marca(ok));
    if (!ok && conferir_vetor) {
        printf("  vetor esperado: [");
        imprimir_cores(vetor_esperado, g->n);
        printf("]\n");
    }

    return ok;
}

/* Executa os tres algoritmos sobre o caso. Retorna o numero de
 * verificacoes que falharam. */
static int executar_caso(const Caso *c)
{
    printf("=== %s ===\n", c->nome);

    GrafoLista *g = montar_grafo(c);
    if (g == NULL) {
        printf("Falha ao montar o caso.\n\n");
        return 1;
    }

    int falhas = 0;
    int num_cores;

    int *gulosa = coloracao_gulosa(g, &num_cores);
    falhas += !conferir_coloracao("Gulosa (ordem natural)", g, gulosa, num_cores,
                                  c->num_cores_gulosa, c->conferir_vetores, c->cores_gulosa);
    free(gulosa);

    int *wp = coloracao_welsh_powell(g, &num_cores);
    falhas += !conferir_coloracao("Welsh-Powell          ", g, wp, num_cores,
                                  c->num_cores_wp, c->conferir_vetores, c->cores_wp);
    free(wp);

    int bipartido = eh_bipartido(g);
    int ok = bipartido == c->bipartido;
    printf("Bipartido (BFS): %s %s\n",
           bipartido == 1 ? "sim, admite coloracao com no maximo 2 cores"
                          : "nao, possui ciclo impar ou laco",
           marca(ok));
    falhas += !ok;

    printf("\n");
    liberar_grafo_lista(g);
    return falhas;
}

/* Casos que nao cabem na tabela: grafo vazio e ponteiro nulo. */
static int executar_casos_limite(void)
{
    printf("=== Casos limite ===\n");

    int falhas = 0;
    int num_cores;

    GrafoLista *vazio = criar_grafo_lista(0);
    int *cores = coloracao_gulosa(vazio, &num_cores);
    int ok = cores == NULL && num_cores == 0;
    printf("Grafo vazio -> gulosa: NULL com 0 cores %s\n", marca(ok));
    falhas += !ok;
    free(cores);

    cores = coloracao_welsh_powell(vazio, &num_cores);
    ok = cores == NULL && num_cores == 0;
    printf("Grafo vazio -> Welsh-Powell: NULL com 0 cores %s\n", marca(ok));
    falhas += !ok;
    free(cores);

    ok = eh_bipartido(vazio) == 1;
    printf("Grafo vazio -> bipartido: sim %s\n", marca(ok));
    falhas += !ok;
    liberar_grafo_lista(vazio);

    ok = coloracao_gulosa(NULL, &num_cores) == NULL && num_cores == -1 &&
         coloracao_welsh_powell(NULL, &num_cores) == NULL && num_cores == -1 &&
         eh_bipartido(NULL) == -1;
    printf("Grafo nulo -> NULL / -1 %s\n\n", marca(ok));
    falhas += !ok;

    return falhas;
}

int main(void)
{
    int total_casos = (int)(sizeof(casos) / sizeof(casos[0]));
    int falhas = 0;

    for (int i = 0; i < total_casos; i++) {
        falhas += executar_caso(&casos[i]);
    }
    falhas += executar_casos_limite();

    printf("========================================\n");
    printf("Casos: %d (+ casos limite) | Verificacoes com falha: %d\n", total_casos, falhas);

    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
