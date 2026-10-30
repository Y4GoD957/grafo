#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "hamiltoniano.h"
#include "tps.h"

#define N_MAX 12
#define N_MAX_ORACULO_TSP 9
#define TEMPO_MINIMO_MS 50.0
#define SEMENTE_EXPERIMENTO 20251030u

static int total_verificacoes = 0;
static int total_falhas = 0;

static void verificar(int condicao, const char *descricao)
{
    total_verificacoes++;
    if (!condicao) {
        total_falhas++;
        printf("  [FALHA] %s\n", descricao);
    }
}

static uint32_t estado_prng = 2463534242u;

static void semear(uint32_t semente)
{
    estado_prng = semente;
}

static uint32_t proximo(void)
{
    estado_prng ^= estado_prng << 13;
    estado_prng ^= estado_prng >> 17;
    estado_prng ^= estado_prng << 5;
    return estado_prng;
}

/* Grafo auxiliar dos testes: vizinhos[u] e a mascara dos vizinhos de u.
 * Alimenta o grafo da biblioteca e os oraculos, que nao usam nenhuma funcao
 * de hamiltoniano.c nem de tps.c. */
typedef struct {
    int n;
    uint32_t vizinhos[N_MAX];
} Mascaras;

static void mascaras_vazio(Mascaras *m, int n)
{
    m->n = n;
    for (int i = 0; i < N_MAX; i++) {
        m->vizinhos[i] = 0;
    }
}

static void mascaras_aresta(Mascaras *m, int u, int v)
{
    m->vizinhos[u] |= 1u << v;
    m->vizinhos[v] |= 1u << u;
}

static int mascaras_adjacente(const Mascaras *m, int u, int v)
{
    return (int)(m->vizinhos[u] >> v & 1u);
}

static GrafoHamiltoniano *construir_grafo(const Mascaras *m)
{
    GrafoHamiltoniano *g = criar_grafo_hamiltoniano(m->n);
    for (int u = 0; g != NULL && u < m->n; u++) {
        for (int v = u + 1; v < m->n; v++) {
            if (mascaras_adjacente(m, u, v)) {
                inserir_aresta_hamiltoniano(g, u, v);
            }
        }
    }
    return g;
}

static int grafo_igual(const GrafoHamiltoniano *g, const Mascaras *m)
{
    if (g->n != m->n) {
        return 0;
    }
    for (int u = 0; u < m->n; u++) {
        for (int v = 0; v < m->n; v++) {
            if (g->adj[u * m->n + v] != mascaras_adjacente(m, u, v)) {
                return 0;
            }
        }
    }
    return 1;
}

/* Oraculo hamiltoniano: programacao dinamica sobre subconjuntos.
 * pontas[S] = conjunto de vertices em que termina algum caminho simples
 * que visita exatamente S. Com inicio_fixo >= 0 so conta caminhos que
 * comecam nesse vertice. */
static uint32_t pontas[1u << N_MAX];

static uint32_t oraculo_pontas_completas(const Mascaras *m, int inicio_fixo)
{
    uint32_t completo = (1u << m->n) - 1u;

    for (uint32_t s = 0; s <= completo; s++) {
        pontas[s] = 0;
    }
    for (int v = 0; v < m->n; v++) {
        if (inicio_fixo < 0 || v == inicio_fixo) {
            pontas[1u << v] = 1u << v;
        }
    }
    for (uint32_t s = 1; s <= completo; s++) {
        for (int v = 0; v < m->n; v++) {
            if (!(pontas[s] >> v & 1u)) {
                continue;
            }
            uint32_t candidatos = m->vizinhos[v] & ~s;
            for (int w = 0; w < m->n; w++) {
                if (candidatos >> w & 1u) {
                    pontas[s | 1u << w] |= 1u << w;
                }
            }
        }
    }

    return pontas[completo];
}

static int oraculo_caminho(const Mascaras *m)
{
    return oraculo_pontas_completas(m, -1) != 0;
}

static int oraculo_ciclo(const Mascaras *m)
{
    if (m->n < 3) {
        return 0;
    }
    return (oraculo_pontas_completas(m, 0) & m->vizinhos[0]) != 0;
}

static int sequencia_de_vertices_distintos(const int *seq, int n)
{
    uint32_t visto = 0;
    for (int i = 0; i < n; i++) {
        if (seq[i] < 0 || seq[i] >= n || (visto >> seq[i] & 1u)) {
            return 0;
        }
        visto |= 1u << seq[i];
    }
    return 1;
}

static int caminho_valido(const Mascaras *m, const int *caminho)
{
    if (!sequencia_de_vertices_distintos(caminho, m->n)) {
        return 0;
    }
    for (int i = 0; i + 1 < m->n; i++) {
        if (!mascaras_adjacente(m, caminho[i], caminho[i + 1])) {
            return 0;
        }
    }
    return 1;
}

static int ciclo_valido(const Mascaras *m, const int *ciclo)
{
    if (ciclo[m->n] != ciclo[0] || !caminho_valido(m, ciclo)) {
        return 0;
    }
    return mascaras_adjacente(m, ciclo[m->n - 1], ciclo[m->n]);
}

typedef struct {
    int caminho;
    int ciclo;
} Existencia;

/* Confronta as duas buscas com os oraculos e valida as sequencias devolvidas.
 * esperado_* < 0 significa "sem expectativa previa". */
static Existencia checar_hamiltoniano(const Mascaras *m, const char *nome, int esperado_caminho,
                                      int esperado_ciclo)
{
    Existencia e = { 0, 0 };
    GrafoHamiltoniano *g = construir_grafo(m);
    int caminho[N_MAX + 1];
    int ciclo[N_MAX + 1];

    if (g == NULL) {
        verificar(0, "alocacao do grafo de teste");
        return e;
    }

    int ref_caminho = oraculo_caminho(m);
    int ref_ciclo = oraculo_ciclo(m);
    int rc = encontrar_caminho_hamiltoniano_bt(g, caminho);
    int rl = encontrar_ciclo_hamiltoniano_bt(g, ciclo);

    int ok = rc == ref_caminho && rl == ref_ciclo;
    ok &= rc != 1 || caminho_valido(m, caminho);
    ok &= rl != 1 || ciclo_valido(m, ciclo);
    ok &= grafo_igual(g, m);
    if (esperado_caminho >= 0) {
        ok &= rc == esperado_caminho;
    }
    if (esperado_ciclo >= 0) {
        ok &= rl == esperado_ciclo;
    }
    verificar(ok, nome);

    e.caminho = rc == 1;
    e.ciclo = rl == 1;
    liberar_grafo_hamiltoniano(g);
    return e;
}

static void testar_hamiltoniano_conhecidos(void)
{
    Mascaras m;
    char nome[64];

    printf("Hamiltoniano: casos conhecidos\n");

    mascaras_vazio(&m, 1);
    checar_hamiltoniano(&m, "um vertice", 1, 0);
    {
        GrafoHamiltoniano *g = construir_grafo(&m);
        int caminho[1] = { -1 };
        verificar(encontrar_caminho_hamiltoniano_bt(g, caminho) == 1 && caminho[0] == 0,
                  "n = 1: o caminho e {0}");
        liberar_grafo_hamiltoniano(g);
    }

    mascaras_vazio(&m, 2);
    checar_hamiltoniano(&m, "dois vertices sem aresta", 0, 0);
    mascaras_aresta(&m, 0, 1);
    checar_hamiltoniano(&m, "uma aresta", 1, 0);

    for (int n = 3; n <= 5; n += 2) {
        mascaras_vazio(&m, n);
        snprintf(nome, sizeof nome, "sem arestas n=%d", n);
        checar_hamiltoniano(&m, nome, 0, 0);
    }

    for (int n = 3; n <= N_MAX; n++) {
        mascaras_vazio(&m, n);
        for (int v = 0; v + 1 < n; v++) {
            mascaras_aresta(&m, v, v + 1);
        }
        snprintf(nome, sizeof nome, "caminho n=%d: so caminho", n);
        checar_hamiltoniano(&m, nome, 1, 0);

        mascaras_aresta(&m, n - 1, 0);
        snprintf(nome, sizeof nome, "ciclo n=%d: caminho e ciclo", n);
        checar_hamiltoniano(&m, nome, 1, 1);
    }

    for (int n = 3; n <= 8; n++) {
        mascaras_vazio(&m, n);
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                mascaras_aresta(&m, u, v);
            }
        }
        snprintf(nome, sizeof nome, "completo n=%d", n);
        checar_hamiltoniano(&m, nome, 1, 1);
    }

    for (int n = 4; n <= N_MAX; n++) {
        mascaras_vazio(&m, n);
        for (int v = 1; v < n; v++) {
            mascaras_aresta(&m, 0, v);
        }
        snprintf(nome, sizeof nome, "estrela n=%d: nenhum dos dois", n);
        checar_hamiltoniano(&m, nome, 0, 0);
    }

    mascaras_vazio(&m, 6);
    for (int base = 0; base < 6; base += 3) {
        mascaras_aresta(&m, base, base + 1);
        mascaras_aresta(&m, base + 1, base + 2);
        mascaras_aresta(&m, base, base + 2);
    }
    checar_hamiltoniano(&m, "dois triangulos desconexos", 0, 0);

    mascaras_vazio(&m, 4);
    mascaras_aresta(&m, 0, 1);
    mascaras_aresta(&m, 1, 2);
    mascaras_aresta(&m, 0, 2);
    checar_hamiltoniano(&m, "triangulo mais vertice isolado", 0, 0);

    /* o vertice 0 tem grau 1: ha caminho, mas nao ha ciclo */
    mascaras_vazio(&m, 4);
    mascaras_aresta(&m, 0, 1);
    mascaras_aresta(&m, 1, 2);
    mascaras_aresta(&m, 2, 3);
    mascaras_aresta(&m, 1, 3);
    checar_hamiltoniano(&m, "caminho sem ciclo (pipa)", 1, 0);

    /* K4 sem uma aresta ainda tem ciclo hamiltoniano */
    mascaras_vazio(&m, 4);
    mascaras_aresta(&m, 0, 1);
    mascaras_aresta(&m, 1, 2);
    mascaras_aresta(&m, 2, 3);
    mascaras_aresta(&m, 3, 0);
    mascaras_aresta(&m, 0, 2);
    checar_hamiltoniano(&m, "K4 menos uma aresta", 1, 1);
}

static void testar_hamiltoniano_invalidos(void)
{
    printf("Hamiltoniano: argumentos invalidos\n");

    GrafoHamiltoniano *g = criar_grafo_hamiltoniano(3);
    GrafoHamiltoniano *vazio = criar_grafo_hamiltoniano(0);
    int saida[4] = { 9, 9, 9, 9 };

    verificar(criar_grafo_hamiltoniano(-1) == NULL, "n negativo");
    verificar(vazio != NULL && vazio->n == 0, "n = 0 cria grafo vazio valido");
    verificar(inserir_aresta_hamiltoniano(g, 0, 1) == 0, "inserir aresta");
    verificar(inserir_aresta_hamiltoniano(g, 1, 0) == -1, "aresta repetida");
    verificar(inserir_aresta_hamiltoniano(g, 2, 2) == -1, "laco");
    verificar(inserir_aresta_hamiltoniano(g, 0, 3) == -1, "vertice invalido");
    verificar(inserir_aresta_hamiltoniano(NULL, 0, 1) == -1, "grafo nulo");
    verificar(tem_aresta_hamiltoniano(g, 1, 0) == 1 && tem_aresta_hamiltoniano(g, 1, 2) == 0,
              "consulta de aresta");
    verificar(tem_aresta_hamiltoniano(g, -1, 0) == -1, "consulta com vertice invalido");
    verificar(contar_arestas_hamiltoniano(g) == 1 && contar_arestas_hamiltoniano(NULL) == -1,
              "contagem de arestas");
    verificar(encontrar_caminho_hamiltoniano_bt(NULL, saida) == -1, "caminho: grafo nulo");
    verificar(encontrar_caminho_hamiltoniano_bt(g, NULL) == -1, "caminho: vetor nulo");
    verificar(encontrar_caminho_hamiltoniano_bt(vazio, saida) == -1, "caminho: n = 0");
    verificar(encontrar_ciclo_hamiltoniano_bt(NULL, saida) == -1, "ciclo: grafo nulo");
    verificar(encontrar_ciclo_hamiltoniano_bt(g, NULL) == -1, "ciclo: vetor nulo");
    verificar(encontrar_ciclo_hamiltoniano_bt(vazio, saida) == -1, "ciclo: n = 0");
    verificar(encontrar_ciclo_hamiltoniano_bt(g, saida) == 0 && saida[0] == 9 && saida[3] == 9,
              "saida inalterada quando nao ha ciclo");

    liberar_grafo_hamiltoniano(NULL);
    liberar_grafo_hamiltoniano(vazio);
    liberar_grafo_hamiltoniano(g);
}

static void testar_hamiltoniano_exaustivo(void)
{
    printf("Hamiltoniano: todos os grafos simples com 1 a 6 vertices\n");

    int falhas_antes = total_falhas;
    long grafos = 0, com_ambos = 0, so_caminho = 0, nenhum = 0;

    for (int n = 1; n <= 6; n++) {
        int pares = n * (n - 1) / 2;
        int pu[15], pv[15], k = 0;
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                pu[k] = u;
                pv[k] = v;
                k++;
            }
        }

        for (uint32_t conjunto = 0; conjunto < (1u << pares); conjunto++) {
            Mascaras m;
            mascaras_vazio(&m, n);
            for (int i = 0; i < pares; i++) {
                if (conjunto >> i & 1u) {
                    mascaras_aresta(&m, pu[i], pv[i]);
                }
            }

            Existencia e = checar_hamiltoniano(&m, "grafo exaustivo", -1, -1);
            grafos++;
            if (e.ciclo) {
                com_ambos++;
            } else if (e.caminho) {
                so_caminho++;
            } else {
                nenhum++;
            }
        }
    }

    printf("  %ld grafos: %ld com ciclo (e caminho), %ld so com caminho, %ld sem nenhum; "
           "%d falha(s)\n",
           grafos, com_ambos, so_caminho, nenhum, total_falhas - falhas_antes);
}

static void testar_hamiltoniano_aleatorio(void)
{
    printf("Hamiltoniano: grafos aleatorios reprodutiveis (7 a 12 vertices)\n");

    static const int densidades[] = { 20, 35, 50, 70 };
    int falhas_antes = total_falhas;
    int grafos = 0, com_ciclo = 0, so_caminho = 0, nenhum = 0;

    semear(777u);
    for (int n = 7; n <= N_MAX; n++) {
        for (int d = 0; d < 4; d++) {
            for (int repeticao = 0; repeticao < 40; repeticao++) {
                Mascaras m;
                mascaras_vazio(&m, n);
                for (int u = 0; u < n; u++) {
                    for (int v = u + 1; v < n; v++) {
                        if ((int)(proximo() % 100) < densidades[d]) {
                            mascaras_aresta(&m, u, v);
                        }
                    }
                }
                Existencia e = checar_hamiltoniano(&m, "grafo aleatorio", -1, -1);
                grafos++;
                if (e.ciclo) {
                    com_ciclo++;
                } else if (e.caminho) {
                    so_caminho++;
                } else {
                    nenhum++;
                }
            }
        }
    }

    printf("  %d grafos: %d com ciclo, %d so com caminho, %d sem nenhum; %d falha(s)\n", grafos,
           com_ciclo, so_caminho, nenhum, total_falhas - falhas_antes);
}

/* Oraculo de TSP: Held-Karp (programacao dinamica sobre subconjuntos),
 * custo minimo de um ciclo hamiltoniano de g (n <= N_MAX_ORACULO_TSP). */
static long long held_karp(const GrafoTSP *g)
{
    static long long dp[1u << N_MAX_ORACULO_TSP][N_MAX_ORACULO_TSP];
    const long long infinito = INT64_MAX / 4;
    int n = g->n;
    uint32_t completo = (1u << n) - 1u;

    for (uint32_t s = 0; s <= completo; s++) {
        for (int j = 0; j < n; j++) {
            dp[s][j] = infinito;
        }
    }
    dp[1][0] = 0;

    for (uint32_t s = 1; s <= completo; s += 2) {
        for (int j = 0; j < n; j++) {
            if (!(s >> j & 1u) || dp[s][j] >= infinito) {
                continue;
            }
            for (int k = 1; k < n; k++) {
                if (!(s >> k & 1u)) {
                    long long c = dp[s][j] + g->peso[j * n + k];
                    if (c < dp[s | 1u << k][k]) {
                        dp[s | 1u << k][k] = c;
                    }
                }
            }
        }
    }

    long long melhor = infinito;
    for (int j = 1; j < n; j++) {
        long long c = dp[completo][j] + g->peso[j * n + 0];
        if (c < melhor) {
            melhor = c;
        }
    }
    return melhor;
}

static int proxima_permutacao(int *a, int tamanho)
{
    int i = tamanho - 2;
    while (i >= 0 && a[i] >= a[i + 1]) {
        i--;
    }
    if (i < 0) {
        return 0;
    }
    int j = tamanho - 1;
    while (a[j] <= a[i]) {
        j--;
    }
    int t = a[i];
    a[i] = a[j];
    a[j] = t;
    for (int l = i + 1, r = tamanho - 1; l < r; l++, r--) {
        t = a[l];
        a[l] = a[r];
        a[r] = t;
    }
    return 1;
}

/* Primeiro tour de custo minimo na ordem lexicografica das permutacoes dos
 * vertices diferentes da origem (enumeracao independente de tps.c). */
static long long oraculo_primeiro_otimo(const GrafoTSP *g, int origem, int *tour)
{
    int n = g->n;
    int outros[N_MAX_ORACULO_TSP];
    long long melhor = INT64_MAX;
    int k = 0;

    for (int v = 0; v < n; v++) {
        if (v != origem) {
            outros[k++] = v;
        }
    }
    do {
        long long total = g->peso[origem * n + outros[0]] + g->peso[outros[k - 1] * n + origem];
        for (int i = 0; i + 1 < k; i++) {
            total += g->peso[outros[i] * n + outros[i + 1]];
        }
        if (total < melhor) {
            melhor = total;
            tour[0] = origem;
            memcpy(&tour[1], outros, (size_t)k * sizeof(int));
            tour[n] = origem;
        }
    } while (proxima_permutacao(outros, k));

    return melhor;
}

static int tour_valido(const GrafoTSP *g, int origem, const int *tour, long long custo)
{
    int n = g->n;
    long long soma = 0;

    if (tour[0] != origem || tour[n] != origem || !sequencia_de_vertices_distintos(tour, n)) {
        return 0;
    }
    for (int i = 0; i < n; i++) {
        soma += g->peso[tour[i] * n + tour[i + 1]];
    }
    return soma == custo;
}

static long long fatorial(int n)
{
    long long f = 1;
    for (int i = 2; i <= n; i++) {
        f *= i;
    }
    return f;
}

/* Cada vizinho mais proximo escolhido tem a menor distancia entre os
 * vertices ainda nao visitados e, em empate, o menor indice. */
static int escolhas_do_vizinho_corretas(const GrafoTSP *g, const int *tour)
{
    int n = g->n;
    uint32_t visitado = 1u << tour[0];

    for (int i = 1; i < n; i++) {
        int atual = tour[i - 1];
        for (int v = 0; v < n; v++) {
            if (visitado >> v & 1u) {
                continue;
            }
            int dv = g->peso[atual * n + v];
            int dt = g->peso[atual * n + tour[i]];
            if (dv < dt || (dv == dt && v < tour[i])) {
                return 0;
            }
        }
        visitado |= 1u << tour[i];
    }
    return 1;
}

/* Reimplementacao independente da insercao do mais proximo: em vez do
 * acrescimo d(i,k) + d(k,j) - d(i,j), compara o custo do ciclo inteiro apos
 * cada insercao possivel (mesmos criterios de desempate documentados). */
static long long custo_do_ciclo(const GrafoTSP *g, const int *ciclo, int tamanho)
{
    long long soma = 0;
    for (int i = 0; i < tamanho; i++) {
        soma += g->peso[ciclo[i] * g->n + ciclo[(i + 1) % tamanho]];
    }
    return soma;
}

static void oraculo_insercao(const GrafoTSP *g, int origem, int *tour)
{
    int n = g->n;
    int ciclo[N_MAX_ORACULO_TSP + 1];
    int candidato[N_MAX_ORACULO_TSP + 1];
    int tamanho = 2;
    int na_rota[N_MAX_ORACULO_TSP] = { 0 };
    int segundo = origem == 0 ? 1 : 0;

    for (int v = 0; v < n; v++) {
        if (v != origem && g->peso[origem * n + v] < g->peso[origem * n + segundo]) {
            segundo = v;
        }
    }
    ciclo[0] = origem;
    ciclo[1] = segundo;
    na_rota[origem] = na_rota[segundo] = 1;

    while (tamanho < n) {
        int melhor_v = -1, melhor_d = 0;
        for (int v = 0; v < n; v++) {
            int d = 0, primeiro = 1;
            if (na_rota[v]) {
                continue;
            }
            for (int i = 0; i < tamanho; i++) {
                int w = g->peso[v * n + ciclo[i]];
                if (primeiro || w < d) {
                    d = w;
                    primeiro = 0;
                }
            }
            if (melhor_v < 0 || d < melhor_d) {
                melhor_v = v;
                melhor_d = d;
            }
        }

        long long melhor_custo = 0;
        int melhor_pos = -1;
        for (int pos = 1; pos <= tamanho; pos++) {
            /* insere melhor_v entre ciclo[pos - 1] e ciclo[pos % tamanho] */
            for (int i = 0; i < pos; i++) {
                candidato[i] = ciclo[i];
            }
            candidato[pos] = melhor_v;
            for (int i = pos; i < tamanho; i++) {
                candidato[i + 1] = ciclo[i];
            }
            long long c = custo_do_ciclo(g, candidato, tamanho + 1);
            if (melhor_pos < 0 || c < melhor_custo) {
                melhor_custo = c;
                melhor_pos = pos;
            }
        }

        for (int i = tamanho; i > melhor_pos; i--) {
            ciclo[i] = ciclo[i - 1];
        }
        ciclo[melhor_pos] = melhor_v;
        na_rota[melhor_v] = 1;
        tamanho++;
    }

    memcpy(tour, ciclo, (size_t)n * sizeof(int));
    tour[n] = origem;
}

static void preencher_aleatorio(GrafoTSP *g, int modo)
{
    for (int u = 0; u < g->n; u++) {
        for (int v = u + 1; v < g->n; v++) {
            int peso = modo == 0 ? 5 : modo == 1 ? 1 + (int)(proximo() % 20) : (int)(proximo() % 4);
            inserir_peso_tsp(g, u, v, peso);
        }
    }
}

static void testar_tsp_contra_held_karp(void)
{
    printf("TSP: forca bruta, vizinho mais proximo e insercao contra os oraculos\n");

    int falhas_antes = total_falhas;
    long instancias = 0, execucoes = 0, heuristica_acima = 0;

    semear(4242u);
    for (int n = 3; n <= N_MAX_ORACULO_TSP; n++) {
        for (int modo = 0; modo < 3; modo++) {
            int repeticoes = n >= 8 ? 3 : 25;
            for (int r = 0; r < repeticoes; r++) {
                GrafoTSP *g = criar_grafo_tsp(n);
                if (g == NULL) {
                    verificar(0, "alocacao do grafo TSP de teste");
                    return;
                }
                preencher_aleatorio(g, modo);
                long long otimo = held_karp(g);
                instancias++;

                for (int origem = 0; origem < n; origem++) {
                    int t_bf[N_MAX_ORACULO_TSP + 1], t_nn[N_MAX_ORACULO_TSP + 1];
                    int t_in[N_MAX_ORACULO_TSP + 1], t_lex[N_MAX_ORACULO_TSP + 1];
                    long long c_bf = -1, c_nn = -1, c_in = -1, permutacoes = -1;
                    int ok = 1;

                    ok &= tsp_forca_bruta_contado(g, origem, t_bf, &c_bf, &permutacoes) == 0;
                    ok &= tsp_vizinho_mais_proximo(g, origem, t_nn, &c_nn) == 0;
                    ok &= tsp_insercao(g, origem, t_in, &c_in) == 0;
                    ok &= permutacoes == fatorial(n - 1);
                    ok &= c_bf == otimo;
                    ok &= tour_valido(g, origem, t_bf, c_bf);
                    ok &= oraculo_primeiro_otimo(g, origem, t_lex) == otimo &&
                          memcmp(t_lex, t_bf, ((size_t)n + 1) * sizeof(int)) == 0;
                    ok &= tour_valido(g, origem, t_nn, c_nn) && c_nn >= otimo;
                    ok &= escolhas_do_vizinho_corretas(g, t_nn);
                    ok &= tour_valido(g, origem, t_in, c_in) && c_in >= otimo;
                    oraculo_insercao(g, origem, t_lex);
                    ok &= memcmp(t_lex, t_in, ((size_t)n + 1) * sizeof(int)) == 0;
                    verificar(ok, "TSP: tour valido, custo igual ao Held-Karp, (n-1)! permutacoes");

                    heuristica_acima += c_nn > otimo;
                    heuristica_acima += c_in > otimo;
                    execucoes++;
                }
                liberar_grafo_tsp(g);
            }
        }
    }

    printf("  %ld instancias, %ld execucoes (todas as origens), heuristica acima do otimo "
           "%ld vezes, %d falha(s)\n",
           instancias, execucoes, heuristica_acima, total_falhas - falhas_antes);
}

static void testar_tsp_exemplos_e_invalidos(void)
{
    printf("TSP: exemplo manual, overflow e entradas invalidas\n");

    /* pontos 0..4 em uma reta, d(i, j) = |i - j| */
    GrafoTSP *reta = criar_grafo_tsp(5);
    int tour[6];
    long long custo;
    for (int u = 0; u < 5; u++) {
        for (int v = u + 1; v < 5; v++) {
            inserir_peso_tsp(reta, u, v, v - u);
        }
    }

    static const int esperado_nn[] = { 0, 1, 2, 3, 4, 0 };
    static const int esperado_insercao[] = { 0, 4, 3, 2, 1, 0 };
    verificar(tsp_vizinho_mais_proximo(reta, 0, tour, &custo) == 0 && custo == 8 &&
                  memcmp(tour, esperado_nn, sizeof esperado_nn) == 0,
              "vizinho mais proximo na reta: 0 1 2 3 4 0 (custo 8)");
    verificar(tsp_insercao(reta, 0, tour, &custo) == 0 && custo == 8 &&
                  memcmp(tour, esperado_insercao, sizeof esperado_insercao) == 0,
              "insercao na reta (calculada a mao): 0 4 3 2 1 0 (custo 8)");
    verificar(tsp_forca_bruta(reta, 0, tour, &custo) == 0 && custo == 8, "reta: otimo 8");

    GrafoTSP *g = criar_grafo_tsp(8);
    for (int u = 0; u < 8; u++) {
        for (int v = u + 1; v < 8; v++) {
            inserir_peso_tsp(g, u, v, PESO_MAXIMO_TSP);
        }
    }
    int t8[9];
    long long c;
    verificar(tsp_forca_bruta(g, 3, t8, &c) == 0 && c == 8LL * PESO_MAXIMO_TSP, "overflow: forca bruta");
    verificar(tsp_vizinho_mais_proximo(g, 3, t8, &c) == 0 && c == 8LL * PESO_MAXIMO_TSP,
              "overflow: vizinho mais proximo");
    verificar(tsp_insercao(g, 3, t8, &c) == 0 && c == 8LL * PESO_MAXIMO_TSP, "overflow: insercao");

    int sentinela[9];
    long long custo_sentinela;
    for (int i = 0; i < 9; i++) {
        sentinela[i] = -7;
    }
    memcpy(t8, sentinela, sizeof t8);
    c = custo_sentinela = -7;

#define VERIFICAR_ERRO(chamada, descricao)                                                      \
    do {                                                                                        \
        memcpy(t8, sentinela, sizeof t8);                                                       \
        c = custo_sentinela;                                                                    \
        verificar((chamada) == -1 && c == custo_sentinela &&                                    \
                      memcmp(t8, sentinela, sizeof t8) == 0,                                    \
                  descricao);                                                                   \
    } while (0)

    VERIFICAR_ERRO(tsp_forca_bruta(g, -1, t8, &c), "forca bruta: origem negativa");
    VERIFICAR_ERRO(tsp_forca_bruta(g, 8, t8, &c), "forca bruta: origem fora do grafo");
    VERIFICAR_ERRO(tsp_vizinho_mais_proximo(g, 8, t8, &c), "vizinho: origem fora do grafo");
    VERIFICAR_ERRO(tsp_insercao(g, -1, t8, &c), "insercao: origem negativa");
    VERIFICAR_ERRO(tsp_forca_bruta(NULL, 0, t8, &c), "forca bruta: grafo nulo");
    VERIFICAR_ERRO(tsp_vizinho_mais_proximo(g, 0, NULL, &c), "vizinho: tour nulo");
    VERIFICAR_ERRO(tsp_insercao(g, 0, t8, NULL), "insercao: custo nulo");

    g->peso[1 * 8 + 2] = 7;
    VERIFICAR_ERRO(tsp_forca_bruta(g, 0, t8, &c), "forca bruta: matriz assimetrica");
    VERIFICAR_ERRO(tsp_vizinho_mais_proximo(g, 0, t8, &c), "vizinho: matriz assimetrica");
    VERIFICAR_ERRO(tsp_insercao(g, 0, t8, &c), "insercao: matriz assimetrica");
    g->peso[1 * 8 + 2] = PESO_MAXIMO_TSP;

    g->peso[0 * 8 + 5] = g->peso[5 * 8 + 0] = -1;
    VERIFICAR_ERRO(tsp_forca_bruta(g, 0, t8, &c), "forca bruta: peso negativo");
    g->peso[0 * 8 + 5] = g->peso[5 * 8 + 0] = PESO_MAXIMO_TSP + 1;
    VERIFICAR_ERRO(tsp_insercao(g, 0, t8, &c), "insercao: peso acima do maximo");
    g->peso[0 * 8 + 5] = g->peso[5 * 8 + 0] = PESO_MAXIMO_TSP;
    g->peso[4 * 8 + 4] = 1;
    VERIFICAR_ERRO(tsp_vizinho_mais_proximo(g, 0, t8, &c), "vizinho: diagonal nao nula");
    g->peso[4 * 8 + 4] = 0;
    verificar(tsp_forca_bruta(g, 0, t8, &c) == 0, "matriz restaurada volta a ser valida");

    GrafoTSP *pequeno = criar_grafo_tsp(2);
    VERIFICAR_ERRO(tsp_forca_bruta(pequeno, 0, t8, &c), "forca bruta: n = 2 rejeitado");
    VERIFICAR_ERRO(tsp_vizinho_mais_proximo(pequeno, 0, t8, &c), "vizinho: n = 2 rejeitado");
    VERIFICAR_ERRO(tsp_insercao(pequeno, 0, t8, &c), "insercao: n = 2 rejeitado");
#undef VERIFICAR_ERRO

    verificar(criar_grafo_tsp(-1) == NULL, "n negativo");
    verificar(inserir_peso_tsp(g, 0, 0, 1) == -1, "peso de laco");
    verificar(inserir_peso_tsp(g, 0, 8, 1) == -1, "peso com vertice invalido");
    verificar(inserir_peso_tsp(g, 0, 1, -1) == -1, "peso negativo");
    verificar(inserir_peso_tsp(g, 0, 1, PESO_MAXIMO_TSP + 1) == -1, "peso acima do maximo");
    verificar(inserir_peso_tsp(NULL, 0, 1, 1) == -1, "inserir peso em grafo nulo");
    verificar(grafo_para_tsp(NULL) == NULL, "reducao de grafo nulo");

    liberar_grafo_tsp(NULL);
    liberar_grafo_tsp(pequeno);
    liberar_grafo_tsp(g);
    liberar_grafo_tsp(reta);
}

/* Confere a reducao em m: pesos 1/2, matriz simetrica, mesmo n, grafo de
 * origem intacto; e a equivalencia "ciclo hamiltoniano <=> otimo == n". */
static int checar_reducao(const Mascaras *m)
{
    GrafoHamiltoniano *g = construir_grafo(m);
    GrafoTSP *t = g == NULL ? NULL : grafo_para_tsp(g);
    int ok = 0;

    if (t != NULL) {
        int n = m->n;
        int tour[N_MAX + 1];
        long long custo;

        ok = t->n == n && grafo_igual(g, m);
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                int esperado = u == v ? 0 : mascaras_adjacente(m, u, v) ? 1 : 2;
                ok &= t->peso[u * n + v] == esperado && t->peso[v * n + u] == esperado;
            }
        }
        ok &= tsp_forca_bruta(t, 0, tour, &custo) == 0;
        ok &= custo >= n && (custo == n) == oraculo_ciclo(m);
        ok &= (custo == n) == (encontrar_ciclo_hamiltoniano_bt(g, tour) == 1);
    }

    liberar_grafo_tsp(t);
    liberar_grafo_hamiltoniano(g);
    return ok;
}

static void testar_reducao(void)
{
    printf("Reducao Hamiltoniano -> TSP: ciclo existe <=> custo otimo == n (n >= 3)\n");

    int falhas_antes = total_falhas;
    long grafos = 0, hamiltonianos = 0;

    for (int n = 3; n <= 6; n++) {
        int pares = n * (n - 1) / 2;
        int pu[15], pv[15], k = 0;
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                pu[k] = u;
                pv[k] = v;
                k++;
            }
        }
        for (uint32_t conjunto = 0; conjunto < (1u << pares); conjunto++) {
            Mascaras m;
            mascaras_vazio(&m, n);
            for (int i = 0; i < pares; i++) {
                if (conjunto >> i & 1u) {
                    mascaras_aresta(&m, pu[i], pv[i]);
                }
            }
            verificar(checar_reducao(&m), "reducao em grafo exaustivo");
            grafos++;
            hamiltonianos += oraculo_ciclo(&m);
        }
    }

    semear(31337u);
    for (int n = 7; n <= 9; n++) {
        for (int d = 0; d < 4; d++) {
            for (int r = 0; r < 15; r++) {
                static const int densidades[] = { 30, 45, 60, 80 };
                Mascaras m;
                mascaras_vazio(&m, n);
                for (int u = 0; u < n; u++) {
                    for (int v = u + 1; v < n; v++) {
                        if ((int)(proximo() % 100) < densidades[d]) {
                            mascaras_aresta(&m, u, v);
                        }
                    }
                }
                verificar(checar_reducao(&m), "reducao em grafo aleatorio");
                grafos++;
                hamiltonianos += oraculo_ciclo(&m);
            }
        }
    }

    printf("  %ld grafos (%ld hamiltonianos), %d falha(s)\n", grafos, hamiltonianos,
           total_falhas - falhas_antes);
}

typedef int (*AlgoritmoTSP)(const GrafoTSP *, int, int *, long long *);

static long long ultima_contagem = 0;

static int forca_bruta_contando(const GrafoTSP *g, int origem, int *tour, long long *custo)
{
    return tsp_forca_bruta_contado(g, origem, tour, custo, &ultima_contagem);
}

static double agora_ms(void)
{
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
}

/* Repete o algoritmo ate acumular TEMPO_MINIMO_MS (uma unica execucao ja
 * basta quando ela sozinha passa disso) e devolve o tempo medio. */
static double medir_ms(AlgoritmoTSP algoritmo, const GrafoTSP *g, int *tour, long long *custo,
                       long *execucoes)
{
    long repeticoes = 0;
    double inicio = agora_ms();
    double decorrido;

    do {
        algoritmo(g, 0, tour, custo);
        repeticoes++;
        decorrido = agora_ms() - inicio;
    } while (decorrido < TEMPO_MINIMO_MS);

    *execucoes = repeticoes;
    return decorrido / (double)repeticoes;
}

typedef struct {
    int n;
    long long otimo, vizinho, insercao;
    double ms_exato, ms_vizinho, ms_insercao;
} Resultado;

static int executar_experimento(void)
{
    static const int tamanhos[] = { 5, 8, 10, 12 };
    Resultado resultados[4];
    int falhas = 0;

    printf("========================================\n");
    printf("PRATICA 10 - EXPERIMENTO DE TSP\n");
    printf("Grafos G(n, 0.5) com semente fixa %u, convertidos por grafo_para_tsp\n",
           SEMENTE_EXPERIMENTO);
    printf("(aresta existente = peso 1, ausente = peso 2); origem 0;\n");
    printf("o mesmo grafo e usado pelos tres algoritmos.\n");
    printf("Tempo medio por execucao, repetindo ate %.0f ms por algoritmo.\n", TEMPO_MINIMO_MS);
    printf("========================================\n");

    semear(SEMENTE_EXPERIMENTO);
    for (int i = 0; i < 4; i++) {
        int n = tamanhos[i];
        int t_bf[N_MAX + 1], t_nn[N_MAX + 1], t_in[N_MAX + 1];
        long long c_bf = 0, c_nn = 0, c_in = 0;
        long e_bf, e_nn, e_in;

        GrafoHamiltoniano *h = criar_grafo_hamiltoniano(n);
        if (h == NULL) {
            printf("Falha de alocacao para n=%d\n", n);
            return 1;
        }
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                if (proximo() % 100 < 50) {
                    inserir_aresta_hamiltoniano(h, u, v);
                }
            }
        }
        GrafoTSP *g = grafo_para_tsp(h);
        if (g == NULL) {
            printf("Falha de alocacao para n=%d\n", n);
            liberar_grafo_hamiltoniano(h);
            return 1;
        }

        double ms_bf = medir_ms(forca_bruta_contando, g, t_bf, &c_bf, &e_bf);
        double ms_nn = medir_ms(tsp_vizinho_mais_proximo, g, t_nn, &c_nn, &e_nn);
        double ms_in = medir_ms(tsp_insercao, g, t_in, &c_in, &e_in);

        int valido_bf = tour_valido(g, 0, t_bf, c_bf);
        int valido_nn = tour_valido(g, 0, t_nn, c_nn);
        int valido_in = tour_valido(g, 0, t_in, c_in);
        int ciclo = encontrar_ciclo_hamiltoniano_bt(h, t_bf);

        printf("\nVertices: %d\n", n);
        printf("Arestas do grafo original: %d\n", contar_arestas_hamiltoniano(h));
        printf("Ciclo hamiltoniano no grafo original: %s (custo otimo %s n = %d)\n",
               ciclo == 1 ? "sim" : "nao", c_bf == n ? "==" : ">", n);
        printf("\nForca bruta exata:\n");
        printf("  Custo otimo: %lld\n", c_bf);
        printf("  Permutacoes avaliadas: %lld (esperado (n-1)! = %lld)\n", ultima_contagem,
               fatorial(n - 1));
        printf("  Tempo de execucao: %.6f ms (%ld execucoes)\n", ms_bf, e_bf);
        printf("Vizinho mais proximo:\n");
        printf("  Custo do tour: %lld (%s otimo)\n", c_nn, c_nn == c_bf ? "igual ao" : "acima do");
        printf("  Tempo de execucao: %.6f ms (%ld execucoes)\n", ms_nn, e_nn);
        printf("Insercao do mais proximo:\n");
        printf("  Custo do tour: %lld (%s otimo)\n", c_in, c_in == c_bf ? "igual ao" : "acima do");
        printf("  Tempo de execucao: %.6f ms (%ld execucoes)\n", ms_in, e_in);
        printf("Validacao dos tours:\n");
        printf("  Forca bruta: %s\n", valido_bf ? "VALIDO" : "INVALIDO");
        printf("  Vizinho mais proximo: %s\n", valido_nn ? "VALIDO" : "INVALIDO");
        printf("  Insercao: %s\n", valido_in ? "VALIDO" : "INVALIDO");

        if (!valido_bf || !valido_nn || !valido_in || c_nn < c_bf || c_in < c_bf ||
            ultima_contagem != fatorial(n - 1) || (c_bf == n) != (ciclo == 1)) {
            falhas++;
        }

        resultados[i] = (Resultado){ n, c_bf, c_nn, c_in, ms_bf, ms_nn, ms_in };
        liberar_grafo_tsp(g);
        liberar_grafo_hamiltoniano(h);
    }

    printf("\n========================================\n");
    printf("RESUMO\n");
    printf("%8s %8s %8s %10s %14s %14s %14s\n", "Vertices", "Exato", "VMP", "Insercao",
           "Tempo exato", "Tempo VMP", "Tempo insercao");
    for (int i = 0; i < 4; i++) {
        printf("%8d %8lld %8lld %10lld %11.6f ms %11.6f ms %11.6f ms\n", resultados[i].n,
               resultados[i].otimo, resultados[i].vizinho, resultados[i].insercao,
               resultados[i].ms_exato, resultados[i].ms_vizinho, resultados[i].ms_insercao);
    }
    printf("========================================\n");

    return falhas;
}

int main(void)
{
    printf("========================================\n");
    printf("TESTES\n");
    printf("========================================\n");

    testar_hamiltoniano_conhecidos();
    testar_hamiltoniano_invalidos();
    testar_hamiltoniano_exaustivo();
    testar_hamiltoniano_aleatorio();
    testar_tsp_contra_held_karp();
    testar_tsp_exemplos_e_invalidos();
    testar_reducao();

    printf("\n%d verificacoes, %d falha(s)\n\n", total_verificacoes, total_falhas);

    int falhas_experimento = executar_experimento();

    return total_falhas > 0 || falhas_experimento > 0;
}
