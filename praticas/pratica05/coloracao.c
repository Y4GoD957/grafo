#include <stdlib.h>

#include "coloracao.h"

#define SEM_COR 0
#define SEM_LADO (-1)

typedef struct {
    int vertice;
    int grau;
} VerticeGrau;

static int tem_laco(const GrafoLista *g)
{
    for (int u = 0; u < g->n; u++) {
        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            if (atual->destino == u) {
                return 1;
            }
        }
    }

    return 0;
}

/* Colore os vertices visitando-os na sequencia dada por ordem[] (uma
 * permutacao de 0..n-1), sempre com a menor cor positiva que nenhum
 * vizinho ja colorido usa.
 *
 * O resultado depende da ordem: vizinhos coloridos antes restringem as
 * escolhas dos seguintes, e uma ordem ruim pode forcar cores extras
 * (ver o exemplo de melhora do Welsh-Powell em main.c).
 *
 * Um vertice tem no maximo n - 1 vizinhos, entao alguma cor em 1..n
 * esta sempre livre. marca[c] == u + 1 indica que a cor c ja foi vista
 * num vizinho de u; usar o indice do vertice como carimbo evita zerar o
 * vetor a cada vertice. */
static int *colorir_na_ordem(GrafoLista *g, const int *ordem, int *num_cores)
{
    int n = g->n;
    int *cores = malloc((size_t)n * sizeof(int));
    int *marca = malloc((size_t)(n + 1) * sizeof(int));
    if (cores == NULL || marca == NULL) {
        free(cores);
        free(marca);
        *num_cores = -1;
        return NULL;
    }

    for (int v = 0; v < n; v++) {
        cores[v] = SEM_COR;
    }
    for (int c = 0; c <= n; c++) {
        marca[c] = 0;
    }

    int maior_cor = 0;

    for (int i = 0; i < n; i++) {
        int u = ordem[i];

        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            int cor_vizinho = cores[atual->destino];
            if (cor_vizinho != SEM_COR) {
                marca[cor_vizinho] = u + 1;
            }
        }

        int cor = 1;
        while (marca[cor] == u + 1) {
            cor++;
        }

        cores[u] = cor;
        if (cor > maior_cor) {
            maior_cor = cor;
        }
    }

    free(marca);
    *num_cores = maior_cor;
    return cores;
}

/* Validacoes comuns as duas coloracoes. Retorna 1 se a coloracao pode
 * prosseguir; caso contrario ja preencheu *num_cores. */
static int pode_colorir(GrafoLista *g, int *num_cores)
{
    if (g == NULL || tem_laco(g)) {
        *num_cores = -1;
        return 0;
    }

    /* criar_grafo_lista nunca produz n < 0, mas rejeita-lo aqui tambem
     * garante ao compilador que os malloc seguintes recebem n > 0. */
    if (g->n <= 0) {
        *num_cores = g->n == 0 ? 0 : -1;
        return 0;
    }

    return 1;
}

int *coloracao_gulosa(GrafoLista *g, int *num_cores)
{
    if (num_cores == NULL) {
        return NULL;
    }
    if (!pode_colorir(g, num_cores)) {
        return NULL;
    }

    int *ordem = malloc((size_t)g->n * sizeof(int));
    if (ordem == NULL) {
        *num_cores = -1;
        return NULL;
    }

    for (int v = 0; v < g->n; v++) {
        ordem[v] = v;
    }

    int *cores = colorir_na_ordem(g, ordem, num_cores);
    free(ordem);
    return cores;
}

/* Grau decrescente; empate pelo menor indice. Como a comparacao nunca
 * devolve 0 para vertices distintos, a ordem final e deterministica
 * mesmo com qsort, que nao e estavel. */
static int comparar_grau_decrescente(const void *a, const void *b)
{
    const VerticeGrau *va = a;
    const VerticeGrau *vb = b;

    if (va->grau != vb->grau) {
        return va->grau > vb->grau ? -1 : 1;
    }
    if (va->vertice != vb->vertice) {
        return va->vertice < vb->vertice ? -1 : 1;
    }
    return 0;
}

/* Welsh-Powell: vertices de grau alto sao os mais restritos (tem mais
 * vizinhos disputando cores), entao colori-los primeiro, enquanto ha
 * mais cores livres, tende a evitar cores extras no fim. Colorir de
 * forma gulosa nessa ordem equivale a descricao classica ("pinte com a
 * cor c cada vertice da lista, em ordem, que nao seja vizinho de outro
 * ja pintado com c"). Os indices dos vertices nao mudam: so a ordem de
 * visita. */
int *coloracao_welsh_powell(GrafoLista *g, int *num_cores)
{
    if (num_cores == NULL) {
        return NULL;
    }
    if (!pode_colorir(g, num_cores)) {
        return NULL;
    }

    int n = g->n;
    VerticeGrau *por_grau = malloc((size_t)n * sizeof(VerticeGrau));
    int *ordem = malloc((size_t)n * sizeof(int));
    if (por_grau == NULL || ordem == NULL) {
        free(por_grau);
        free(ordem);
        *num_cores = -1;
        return NULL;
    }

    for (int v = 0; v < n; v++) {
        por_grau[v].vertice = v;
        por_grau[v].grau = grau_lista(g, v);
    }

    qsort(por_grau, (size_t)n, sizeof(VerticeGrau), comparar_grau_decrescente);

    for (int i = 0; i < n; i++) {
        ordem[i] = por_grau[i].vertice;
    }
    free(por_grau);

    int *cores = colorir_na_ordem(g, ordem, num_cores);
    free(ordem);
    return cores;
}

/* 2-coloracao por BFS, independente das funcoes gulosas acima.
 *
 * Cada componente comeca com a raiz no lado 0; todo vizinho ainda sem
 * lado vai para o lado oposto. Na BFS, os lados alternam conforme a
 * paridade da distancia a raiz. Encontrar uma aresta entre dois
 * vertices do mesmo lado significa um ciclo de comprimento impar (a
 * aresta fecha dois caminhos de mesma paridade), e grafos com ciclo
 * impar nao sao bipartidos. Um laco u--u e o caso extremo: u e vizinho
 * de si mesmo e tem o mesmo lado, entao tambem e rejeitado. */
int eh_bipartido(GrafoLista *g)
{
    if (g == NULL) {
        return -1;
    }

    if (g->n == 0) {
        return 1;
    }

    int *lado = malloc((size_t)g->n * sizeof(int));
    /* Cada vertice entra na fila uma unica vez, entao um vetor de n
     * posicoes com indices de inicio e fim basta. */
    int *fila = malloc((size_t)g->n * sizeof(int));
    if (lado == NULL || fila == NULL) {
        free(lado);
        free(fila);
        return -1;
    }

    for (int v = 0; v < g->n; v++) {
        lado[v] = SEM_LADO;
    }

    int bipartido = 1;

    /* Toda raiz ainda sem lado inicia uma nova componente; comecar so
     * pelo vertice 0 ignoraria ciclos impares em outras componentes. */
    for (int raiz = 0; raiz < g->n && bipartido; raiz++) {
        if (lado[raiz] != SEM_LADO) {
            continue;
        }

        int inicio = 0;
        int fim = 0;
        lado[raiz] = 0;
        fila[fim++] = raiz;

        while (bipartido && inicio < fim) {
            int u = fila[inicio++];

            for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
                int v = atual->destino;

                if (lado[v] == SEM_LADO) {
                    lado[v] = 1 - lado[u];
                    fila[fim++] = v;
                } else if (lado[v] == lado[u]) {
                    bipartido = 0;
                    break;
                }
            }
        }
    }

    free(lado);
    free(fila);
    return bipartido;
}
