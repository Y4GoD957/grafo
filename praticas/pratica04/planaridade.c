#include <string.h>

#include "planaridade.h"

/* Estrategia da busca por subdivisoes (Kuratowski, n <= 10)
 *
 * Uma subdivisao de H (K5 ou K3,3) em G e formada por:
 *   - vertices de ramificacao em G, um para cada vertice de H;
 *   - para cada aresta (a, b) de H, um caminho em G de a ate b;
 *   - esses caminhos sao internamente disjuntos: nenhum vertice
 *     interno e ramificacao e nenhum vertice interno e usado por dois
 *     caminhos diferentes.
 *
 * A busca testa todas as escolhas de vertices de ramificacao (todos os
 * subconjuntos de 5 vertices para K5; todos os de 6 vertices, divididos
 * de todas as formas em dois lados de 3, para K3,3). Para cada escolha,
 * tenta ligar os pares exigidos um por vez com backtracking: enumera
 * cada caminho simples possivel do par atual passando so por vertices
 * livres, marca seus internos como usados e segue para o proximo par;
 * se algum par nao puder ser ligado, desfaz a ultima escolha.
 *
 * A busca e exaustiva, entao para n <= 10 ela decide exatamente se G
 * contem uma subdivisao de H; junto com o Teorema de Kuratowski, isso
 * decide a planaridade. O custo e exponencial, por isso o limite. */

#define MAX_PARES 10

typedef struct {
    int n;
    int adj[LIMITE_KURATOWSKI][LIMITE_KURATOWSKI];
    int grau[LIMITE_KURATOWSKI];
    int eh_ramo[LIMITE_KURATOWSKI];
    int usado[LIMITE_KURATOWSKI];
    int pares[MAX_PARES][2];
    int num_pares;
} BuscaSubdivisao;

int contar_arestas(const GrafoLista *g)
{
    if (g == NULL) {
        return -1;
    }

    /* Cada aresta u--v (u != v) aparece em adj[u] e em adj[v]; contar
     * apenas destino > u conta cada uma exatamente uma vez. Lacos
     * (armazenados uma unica vez por inserir_aresta_lista) ficam fora. */
    int m = 0;
    for (int u = 0; u < g->n; u++) {
        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            if (atual->destino > u) {
                m++;
            }
        }
    }

    return m;
}

int eh_planar_euler(GrafoLista *g)
{
    if (g == NULL) {
        return -1;
    }

    /* Com n < 3, 3n - 6 e negativo e a formula nao se aplica; todo
     * grafo simples com ate 2 vertices e planar. */
    if (g->n < 3) {
        return 1;
    }

    return contar_arestas(g) <= 3 * g->n - 6;
}

/* Copia g para uma matriz de adjacencia (sem lacos), mais conveniente
 * para as consultas repetidas da forca bruta. */
static void preparar_busca(const GrafoLista *g, BuscaSubdivisao *b)
{
    memset(b, 0, sizeof(*b));
    b->n = g->n;

    for (int u = 0; u < g->n; u++) {
        for (const No *atual = g->adj[u]; atual != NULL; atual = atual->prox) {
            if (atual->destino != u) {
                b->adj[u][atual->destino] = 1;
                b->grau[u]++;
            }
        }
    }
}

static int ligar_pares(BuscaSubdivisao *b, int k);

/* Estende o caminho do par k que chegou ao vertice atual. Ao alcancar o
 * destino, o caminho esta completo e a busca passa ao par k + 1. So
 * vertices que nao sao ramificacao e ainda nao estao usados podem ser
 * internos; a marcacao em usado[] impede que dois caminhos (ou o mesmo
 * caminho duas vezes) compartilhem um vertice interno. */
static int estender_caminho(BuscaSubdivisao *b, int k, int atual)
{
    int destino = b->pares[k][1];

    for (int y = 0; y < b->n; y++) {
        if (!b->adj[atual][y]) {
            continue;
        }

        if (y == destino) {
            if (ligar_pares(b, k + 1)) {
                return 1;
            }
        } else if (!b->eh_ramo[y] && !b->usado[y]) {
            b->usado[y] = 1;
            if (estender_caminho(b, k, y)) {
                return 1;
            }
            b->usado[y] = 0;
        }
    }

    return 0;
}

static int ligar_pares(BuscaSubdivisao *b, int k)
{
    if (k == b->num_pares) {
        return 1;
    }

    return estender_caminho(b, k, b->pares[k][0]);
}

/* Testa uma escolha concreta de vertices de ramificacao, cujos pares a
 * ligar ja estao em b->pares. grau_minimo e o grau de cada vertice em H
 * (4 em K5, 3 em K3,3): um ramo com grau menor em G nao pode ser a
 * origem de tantos caminhos disjuntos, entao a escolha e descartada
 * sem backtracking (poda segura, nao muda o resultado). */
static int testar_ramos(BuscaSubdivisao *b, const int *ramos, int num_ramos, int grau_minimo)
{
    for (int i = 0; i < num_ramos; i++) {
        if (b->grau[ramos[i]] < grau_minimo) {
            return 0;
        }
    }

    for (int v = 0; v < b->n; v++) {
        b->eh_ramo[v] = 0;
        b->usado[v] = 0;
    }
    for (int i = 0; i < num_ramos; i++) {
        b->eh_ramo[ramos[i]] = 1;
    }

    return ligar_pares(b, 0);
}

static int contar_bits(unsigned int x)
{
    int total = 0;
    for (; x != 0; x &= x - 1) {
        total++;
    }
    return total;
}

/* Grava em vertices os indices dos bits ligados de mascara (ordem
 * crescente) e retorna quantos sao. */
static int extrair_vertices(unsigned int mascara, int n, int *vertices)
{
    int total = 0;
    for (int v = 0; v < n; v++) {
        if (mascara & (1u << v)) {
            vertices[total++] = v;
        }
    }
    return total;
}

int tem_subdivisao_k5(GrafoLista *g, int ramos[5])
{
    if (g == NULL || g->n > LIMITE_KURATOWSKI) {
        return -1;
    }

    BuscaSubdivisao b;
    preparar_busca(g, &b);

    for (unsigned int mascara = 0; mascara < (1u << g->n); mascara++) {
        if (contar_bits(mascara) != 5) {
            continue;
        }

        int escolhidos[5];
        extrair_vertices(mascara, g->n, escolhidos);

        /* K5: todos os 10 pares de ramos precisam de um caminho. */
        b.num_pares = 0;
        for (int i = 0; i < 5; i++) {
            for (int j = i + 1; j < 5; j++) {
                b.pares[b.num_pares][0] = escolhidos[i];
                b.pares[b.num_pares][1] = escolhidos[j];
                b.num_pares++;
            }
        }

        if (testar_ramos(&b, escolhidos, 5, 4)) {
            if (ramos != NULL) {
                memcpy(ramos, escolhidos, sizeof(escolhidos));
            }
            return 1;
        }
    }

    return 0;
}

int tem_subdivisao_k33(GrafoLista *g, int ramos[6])
{
    if (g == NULL || g->n > LIMITE_KURATOWSKI) {
        return -1;
    }

    BuscaSubdivisao b;
    preparar_busca(g, &b);

    for (unsigned int mascara = 0; mascara < (1u << g->n); mascara++) {
        if (contar_bits(mascara) != 6) {
            continue;
        }

        int seis[6];
        extrair_vertices(mascara, g->n, seis);

        /* Divide os 6 ramos em lados A e B de 3. Fixar seis[0] no lado
         * A evita testar cada divisao duas vezes (A|B e B|A): sobram
         * C(5,2) = 10 divisoes, codificadas pelos bits 1..5 de lado_a. */
        for (unsigned int lado_a = 1; lado_a < (1u << 6); lado_a += 2) {
            if (contar_bits(lado_a) != 3) {
                continue;
            }

            int escolhidos[6];
            int na = 0;
            int nb = 3;
            for (int i = 0; i < 6; i++) {
                if (lado_a & (1u << i)) {
                    escolhidos[na++] = seis[i];
                } else {
                    escolhidos[nb++] = seis[i];
                }
            }

            /* K3,3: cada vertice de A precisa de um caminho ate cada
             * vertice de B (9 pares). */
            b.num_pares = 0;
            for (int i = 0; i < 3; i++) {
                for (int j = 3; j < 6; j++) {
                    b.pares[b.num_pares][0] = escolhidos[i];
                    b.pares[b.num_pares][1] = escolhidos[j];
                    b.num_pares++;
                }
            }

            if (testar_ramos(&b, escolhidos, 6, 3)) {
                if (ramos != NULL) {
                    memcpy(ramos, escolhidos, sizeof(escolhidos));
                }
                return 1;
            }
        }
    }

    return 0;
}

ResultadoPlanaridade verificar_planaridade(GrafoLista *g)
{
    if (g == NULL) {
        return PLANARIDADE_ERRO;
    }

    if (!eh_planar_euler(g)) {
        return PLANARIDADE_NAO_PLANAR_EULER;
    }

    if (g->n > LIMITE_KURATOWSKI) {
        return PLANARIDADE_INCONCLUSIVO;
    }

    if (tem_subdivisao_k5(g, NULL)) {
        return PLANARIDADE_NAO_PLANAR_K5;
    }

    if (tem_subdivisao_k33(g, NULL)) {
        return PLANARIDADE_NAO_PLANAR_K33;
    }

    return PLANARIDADE_PLANAR;
}
