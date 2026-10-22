#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "agm.h"

#define MAX_N 16

static int falhas = 0;

static void conferir(int condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "OK" : "FALHA", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void imprimir_arvore(const char *nome, const Aresta *mst, int k, long long total)
{
    printf("  %s:", nome);
    for (int i = 0; i < k; i++) {
        printf(" (%d-%d:%d)", mst[i].u, mst[i].v, mst[i].peso);
    }
    printf("  peso total = %lld\n", total);
}

/* Confere de forma independente se mst[0..k-1] e uma floresta geradora
 * valida do grafo dado: cada aresta existe na entrada (com o mesmo peso),
 * nao ha ciclo, o peso informado bate e o numero de arestas e n - c, onde
 * c e o numero de componentes do grafo. */
static int floresta_valida(int n, const Aresta *arestas, int m, const Aresta *mst,
                           int k, long long total)
{
    DSU *d = criar_dsu(n);
    DSU *g = criar_dsu(n);
    int ok = d != NULL && g != NULL;
    long long soma = 0;

    for (int i = 0; ok && i < k; i++) {
        int existe = 0;
        for (int j = 0; j < m; j++) {
            int mesmo = arestas[j].u == mst[i].u && arestas[j].v == mst[i].v;
            int inverso = arestas[j].u == mst[i].v && arestas[j].v == mst[i].u;
            existe = existe || ((mesmo || inverso) && arestas[j].peso == mst[i].peso);
        }
        ok = existe && unir(d, mst[i].u, mst[i].v) == 1;
        soma += mst[i].peso;
    }

    int componentes = n;
    for (int j = 0; ok && j < m; j++) {
        if (unir(g, arestas[j].u, arestas[j].v) == 1) {
            componentes--;
        }
    }

    ok = ok && soma == total && k == n - componentes;
    liberar_dsu(d);
    liberar_dsu(g);
    return ok;
}

static void teste_dsu(void)
{
    printf("\n== Union-Find ==\n");
    DSU *d = criar_dsu(8);
    conferir(d != NULL, "cria DSU com 8 elementos");
    conferir(criar_dsu(-1) == NULL, "n negativo rejeitado");

    int unioes_ok = 1;
    for (int i = 0; i < 7; i++) {
        unioes_ok = unioes_ok && unir(d, i, i + 1) == 1;
    }
    conferir(unioes_ok, "7 unioes em cadeia retornam 1");
    conferir(unir(d, 0, 7) == 0, "unir elementos ja conectados retorna 0");

    int raiz = find(d, 0);
    int todos = 1;
    for (int i = 0; i < 8; i++) {
        todos = todos && find(d, i) == raiz;
    }
    conferir(raiz >= 0 && todos, "todos os elementos compartilham a mesma raiz");

    int comprimido = 1;
    for (int i = 0; i < 8; i++) {
        comprimido = comprimido && (d->pai[i] == raiz || i == raiz);
    }
    conferir(comprimido, "compressao de caminho: todos apontam direto para a raiz");

    int rank_max = 0;
    for (int i = 0; i < 8; i++) {
        rank_max = d->rank[i] > rank_max ? d->rank[i] : rank_max;
    }
    conferir(rank_max <= 3, "uniao por rank: rank <= log2(8) = 3");

    conferir(find(d, -1) == -1 && find(d, 8) == -1 && find(NULL, 0) == -1,
             "find rejeita indice invalido e DSU nulo");
    conferir(unir(d, 0, 8) == -1 && unir(NULL, 0, 1) == -1,
             "unir rejeita indice invalido e DSU nulo");
    liberar_dsu(d);
    liberar_dsu(NULL);

    /* A raiz do conjunto de maior rank permanece raiz (uniao por rank). */
    DSU *e = criar_dsu(4);
    unir(e, 0, 1);
    unir(e, 2, 3);
    unir(e, 0, 2);
    int r = find(e, 0);
    conferir(e->rank[r] == 2 && find(e, 3) == r, "uniao de arvores de rank 1 gera rank 2");
    unir(e, 0, 3);
    conferir(e->rank[find(e, 0)] == 2, "uniao redundante nao muda o rank");
    liberar_dsu(e);
}

static void teste_classico(void)
{
    printf("\n== Grafo classico de 9 vertices (peso da AGM = 37) ==\n");
    const Aresta arestas[] = {
        { 0, 1, 4 }, { 0, 7, 8 }, { 1, 2, 8 }, { 1, 7, 11 }, { 2, 3, 7 },
        { 2, 8, 2 }, { 2, 5, 4 }, { 3, 4, 9 }, { 3, 5, 14 }, { 4, 5, 10 },
        { 5, 6, 2 }, { 6, 7, 1 }, { 6, 8, 6 }, { 7, 8, 7 }
    };
    int m = 14;
    Aresta mst[8];
    long long total = -1;

    int k = kruskal(9, arestas, m, mst, &total);
    imprimir_arvore("Kruskal", mst, k, total);
    conferir(k == 8 && total == 37, "Kruskal: 8 arestas, peso 37");
    conferir(floresta_valida(9, arestas, m, mst, k, total), "Kruskal: arvore geradora valida");

    /* Kruskal escolhe em ordem crescente de peso: confere extremos e ordem. */
    int pesos[8], ordenados = 1;
    for (int i = 0; i < 8; i++) {
        pesos[i] = mst[i].peso;
    }
    for (int i = 0; i + 1 < 8; i++) {
        ordenados = ordenados && pesos[i] <= pesos[i + 1];
    }
    conferir(ordenados && pesos[0] == 1 && pesos[7] == 9,
             "Kruskal escolhe as arestas em ordem crescente de peso");

    long long total_prim = -1;
    int kp = prim(9, arestas, m, 0, mst, &total_prim);
    imprimir_arvore("Prim (origem 0)", mst, kp, total_prim);
    conferir(kp == 8 && total_prim == 37, "Prim: 8 arestas, peso 37");
    conferir(floresta_valida(9, arestas, m, mst, kp, total_prim), "Prim: arvore geradora valida");

    int todas_origens = 1;
    for (int s = 0; s < 9; s++) {
        long long t = -1;
        todas_origens = todas_origens && prim(9, arestas, m, s, mst, &t) == 8 && t == 37;
    }
    conferir(todas_origens, "Prim tem peso 37 a partir de qualquer origem");
}

static void teste_unica(void)
{
    printf("\n== Grafo com AGM unica (arestas exatas) ==\n");
    const Aresta arestas[] = {
        { 0, 1, 1 }, { 1, 2, 2 }, { 2, 3, 3 }, { 3, 0, 10 }, { 0, 2, 20 }
    };
    Aresta mst[3];
    long long total;

    int k = kruskal(4, arestas, 5, mst, &total);
    conferir(k == 3 && total == 6 &&
             mst[0].u == 0 && mst[0].v == 1 && mst[0].peso == 1 &&
             mst[1].u == 1 && mst[1].v == 2 && mst[1].peso == 2 &&
             mst[2].u == 2 && mst[2].v == 3 && mst[2].peso == 3,
             "Kruskal: (0-1:1) (1-2:2) (2-3:3)");

    k = prim(4, arestas, 5, 0, mst, &total);
    conferir(k == 3 && total == 6 &&
             mst[0].u == 0 && mst[0].v == 1 &&
             mst[1].u == 1 && mst[1].v == 2 &&
             mst[2].u == 2 && mst[2].v == 3,
             "Prim: mesma arvore, com u = pai e v = vertice adicionado");
}

static void teste_casos_especiais(void)
{
    printf("\n== Casos especiais ==\n");
    Aresta mst[8];
    long long total;

    /* Desconexo: componentes {0,1,2}, {3,4}, {5}. */
    const Aresta desc[] = { { 0, 1, 3 }, { 1, 2, 1 }, { 0, 2, 2 }, { 3, 4, 7 } };
    int k = kruskal(6, desc, 4, mst, &total);
    conferir(k == 3 && total == 10 && floresta_valida(6, desc, 4, mst, k, total),
             "grafo desconexo: Kruskal devolve floresta (3 arestas, peso 10)");
    k = prim(6, desc, 4, 0, mst, &total);
    conferir(k == 2 && total == 3 && floresta_valida(3, desc, 3, mst, k, total),
             "grafo desconexo: Prim cobre so a componente da origem (2 arestas, peso 3)");
    k = prim(6, desc, 4, 5, mst, &total);
    conferir(k == 0 && total == 0, "origem isolada: 0 arestas, peso 0");

    /* Arestas paralelas, laco, pesos negativos e zero. */
    const Aresta esp[] = {
        { 0, 1, 5 }, { 0, 1, -2 }, { 1, 1, -100 }, { 1, 2, 0 }, { 2, 0, 4 },
        { 2, 3, -7 }
    };
    k = kruskal(4, esp, 6, mst, &total);
    conferir(k == 3 && total == -9 && floresta_valida(4, esp, 6, mst, k, total),
             "Kruskal: paralelas, laco ignorado, pesos negativos e zero (peso -9)");
    k = prim(4, esp, 6, 0, mst, &total);
    conferir(k == 3 && total == -9 && floresta_valida(4, esp, 6, mst, k, total),
             "Prim: mesmo resultado (peso -9)");

    /* Empates: todas as arestas com o mesmo peso. */
    const Aresta emp[] = { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 }, { 3, 0, 1 }, { 0, 2, 1 } };
    k = kruskal(4, emp, 5, mst, &total);
    long long total_p;
    int kp = prim(4, emp, 5, 0, mst, &total_p);
    conferir(k == 3 && kp == 3 && total == 3 && total_p == 3, "empates: ambos com peso 3");

    /* Vertice unico, sem vertices, sem arestas. */
    conferir(kruskal(1, NULL, 0, NULL, &total) == 0 && total == 0,
             "um vertice: Kruskal retorna 0 arestas (mst e arestas podem ser NULL)");
    conferir(prim(1, NULL, 0, 0, NULL, &total) == 0 && total == 0,
             "um vertice: Prim retorna 0 arestas");
    conferir(kruskal(0, NULL, 0, NULL, &total) == 0 && total == 0,
             "zero vertices: Kruskal retorna 0");
    conferir(kruskal(3, NULL, 0, mst, &total) == 0 && total == 0,
             "sem arestas: floresta vazia");

    /* Pesos extremos: o total nao cabe em int. */
    const Aresta ext[] = { { 0, 1, INT_MAX }, { 1, 2, INT_MAX }, { 2, 3, INT_MAX } };
    long long esperado = 3LL * INT_MAX;
    k = kruskal(4, ext, 3, mst, &total);
    conferir(k == 3 && total == esperado, "Kruskal: peso total acima de INT_MAX em long long");
    k = prim(4, ext, 3, 0, mst, &total);
    conferir(k == 3 && total == esperado, "Prim: peso total acima de INT_MAX em long long");
    const Aresta neg[] = { { 0, 1, INT_MIN }, { 1, 2, INT_MIN } };
    k = kruskal(3, neg, 2, mst, &total);
    conferir(k == 2 && total == 2LL * INT_MIN, "pesos INT_MIN: total em long long");
    k = prim(3, neg, 2, 1, mst, &total);
    conferir(k == 2 && total == 2LL * INT_MIN, "Prim com pesos INT_MIN");
}

static void teste_invalidos(void)
{
    printf("\n== Entradas invalidas ==\n");
    const Aresta ok[] = { { 0, 1, 1 }, { 1, 2, 1 } };
    const Aresta ruim_alto[] = { { 0, 3, 1 } };
    const Aresta ruim_baixo[] = { { -1, 0, 1 } };
    Aresta mst[4];
    long long total = 99;

    mst[0].peso = 77;
    conferir(kruskal(-1, ok, 2, mst, &total) == -1, "Kruskal: n negativo");
    conferir(kruskal(3, ok, -1, mst, &total) == -1, "Kruskal: m negativo");
    conferir(kruskal(3, NULL, 2, mst, &total) == -1, "Kruskal: arestas nulo com m > 0");
    conferir(kruskal(3, ok, 2, NULL, &total) == -1, "Kruskal: mst nulo");
    conferir(kruskal(3, ruim_alto, 1, mst, &total) == -1, "Kruskal: vertice >= n");
    conferir(kruskal(3, ruim_baixo, 1, mst, &total) == -1, "Kruskal: vertice negativo");
    conferir(total == 99 && mst[0].peso == 77, "Kruskal: erro nao altera mst nem peso_total");

    conferir(prim(3, ok, 2, 3, mst, &total) == -1, "Prim: origem >= n");
    conferir(prim(3, ok, 2, -1, mst, &total) == -1, "Prim: origem negativa");
    conferir(prim(0, NULL, 0, 0, mst, &total) == -1, "Prim: n = 0 (sem origem valida)");
    conferir(prim(-1, ok, 2, 0, mst, &total) == -1, "Prim: n negativo");
    conferir(prim(3, ok, -1, 0, mst, &total) == -1, "Prim: m negativo");
    conferir(prim(3, NULL, 2, 0, mst, &total) == -1, "Prim: arestas nulo com m > 0");
    conferir(prim(3, ok, 2, 0, NULL, &total) == -1, "Prim: mst nulo");
    conferir(prim(3, ruim_alto, 1, 0, mst, &total) == -1, "Prim: vertice >= n");
    conferir(total == 99 && mst[0].peso == 77, "Prim: erro nao altera mst nem peso_total");

    conferir(kruskal(3, ok, 2, mst, NULL) == 2 && prim(3, ok, 2, 0, mst, NULL) == 2,
             "peso_total pode ser NULL");
}

/* Referencia: forca bruta sobre subconjuntos de arestas (grafos pequenos). */
static long long agm_forca_bruta(int n, const Aresta *a, int m, int *conexo)
{
    long long melhor = LLONG_MAX;
    *conexo = 0;
    for (unsigned mascara = 0; mascara < (1u << m); mascara++) {
        int bits = 0;
        for (int i = 0; i < m; i++) {
            bits += (mascara >> i) & 1u;
        }
        if (bits != n - 1) {
            continue;
        }
        DSU *d = criar_dsu(n);
        long long soma = 0;
        int ok = 1;
        for (int i = 0; ok && i < m; i++) {
            if ((mascara >> i) & 1u) {
                ok = unir(d, a[i].u, a[i].v) == 1;
                soma += a[i].peso;
            }
        }
        liberar_dsu(d);
        if (ok) {
            *conexo = 1;
            melhor = soma < melhor ? soma : melhor;
        }
    }
    return melhor;
}

static unsigned long long semente = 2024;

static int aleatorio(int limite)
{
    semente = semente * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((semente >> 33) % (unsigned long long)limite);
}

static void teste_aleatorio(void)
{
    printf("\n== Comparacao com forca bruta e entre Kruskal e Prim ==\n");
    int total_casos = 400, divergencias = 0, conexos = 0, desconexos = 0;

    for (int t = 0; t < total_casos; t++) {
        int n = 1 + aleatorio(6);
        int m = aleatorio(11);
        Aresta arestas[10];
        for (int i = 0; i < m; i++) {
            arestas[i].u = aleatorio(n);
            arestas[i].v = aleatorio(n);
            arestas[i].peso = aleatorio(13) - 4;
        }

        Aresta mk[MAX_N], mp[MAX_N];
        long long tk = 0, tp = 0;
        int kk = kruskal(n, arestas, m, mk, &tk);
        int kp = prim(n, arestas, m, 0, mp, &tp);
        int ok = kk >= 0 && kp >= 0 && floresta_valida(n, arestas, m, mk, kk, tk);

        int conexo;
        long long ref = agm_forca_bruta(n, arestas, m, &conexo);
        if (n == 1) {
            conexo = 1;
            ref = 0;
        }

        if (conexo) {
            ok = ok && kk == n - 1 && kp == n - 1 && tk == ref && tp == ref &&
                 floresta_valida(n, arestas, m, mp, kp, tp);
            conexos++;
        } else {
            /* Prim cobre a componente do vertice 0: confere pelo DSU. */
            DSU *g = criar_dsu(n);
            for (int i = 0; i < m; i++) {
                unir(g, arestas[i].u, arestas[i].v);
            }
            int tamanho = 0;
            for (int v = 0; v < n; v++) {
                tamanho += find(g, v) == find(g, 0);
            }
            liberar_dsu(g);
            ok = ok && kp == tamanho - 1 && kk < n - 1;
            desconexos++;
        }
        if (!ok) {
            divergencias++;
        }
    }

    printf("  %d grafos: %d conexos, %d desconexos, %d divergencias\n",
           total_casos, conexos, desconexos, divergencias);
    conferir(divergencias == 0 && conexos > 50 && desconexos > 20,
             "Kruskal, Prim e forca bruta concordam em todos os casos");
}

int main(void)
{
    printf("Pratica 08 - Arvore Geradora Minima: Kruskal e Prim\n");
    teste_dsu();
    teste_classico();
    teste_unica();
    teste_casos_especiais();
    teste_invalidos();
    teste_aleatorio();

    printf("\n%s (%d falha(s))\n",
           falhas == 0 ? "TODOS OS TESTES PASSARAM" : "HA FALHAS", falhas);
    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
