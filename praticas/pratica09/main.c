#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "cobertura.h"

#define N_MAX_TESTE 15
#define TEMPO_MINIMO_MS 50.0

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

/* Grafo auxiliar dos testes: vizinhos[u] e a mascara de bits dos vizinhos
 * de u. Serve de entrada para o grafo da biblioteca e para o oraculo, que
 * nao usa nenhuma funcao de cobertura.c. */
typedef struct {
    int n;
    uint32_t vizinhos[N_MAX_TESTE];
} Mascaras;

static void mascaras_vazio(Mascaras *m, int n)
{
    m->n = n;
    for (int i = 0; i < N_MAX_TESTE; i++) {
        m->vizinhos[i] = 0;
    }
}

static void mascaras_aresta(Mascaras *m, int u, int v)
{
    m->vizinhos[u] |= 1u << v;
    m->vizinhos[v] |= 1u << u;
}

static int mascaras_arestas(const Mascaras *m)
{
    int total = 0;
    for (int u = 0; u < m->n; u++) {
        total += __builtin_popcount(m->vizinhos[u]);
    }
    return total / 2;
}

static GrafoCobertura *construir_grafo(const Mascaras *m)
{
    GrafoCobertura *g = criar_grafo_cobertura(m->n);
    for (int u = 0; g != NULL && u < m->n; u++) {
        for (int v = u + 1; v < m->n; v++) {
            if (m->vizinhos[u] >> v & 1u) {
                inserir_aresta_cobertura(g, u, v);
            }
        }
    }
    return g;
}

/* O conjunto de vertices em subconjunto cobre todas as arestas se todo
 * vertice fora dele tem todos os vizinhos dentro. */
static int mascara_cobre(const Mascaras *m, uint32_t subconjunto)
{
    for (int u = 0; u < m->n; u++) {
        if (!(subconjunto >> u & 1u) && (m->vizinhos[u] & ~subconjunto) != 0) {
            return 0;
        }
    }
    return 1;
}

static int oraculo_minimo(const Mascaras *m)
{
    int melhor = m->n;
    for (uint32_t s = 0; s < (1u << m->n); s++) {
        int tamanho = __builtin_popcount(s);
        if (tamanho < melhor && mascara_cobre(m, s)) {
            melhor = tamanho;
        }
    }
    return melhor;
}

static uint32_t flags_para_mascara(const int *flags, int n)
{
    uint32_t s = 0;
    for (int v = 0; v < n; v++) {
        if (flags[v]) {
            s |= 1u << v;
        }
    }
    return s;
}

static int grafo_igual(const GrafoCobertura *g, const Mascaras *m)
{
    for (int u = 0; u < m->n; u++) {
        for (int v = 0; v < m->n; v++) {
            if (g->adj[u * m->n + v] != (m->vizinhos[u] >> v & 1u)) {
                return 0;
            }
        }
    }
    return 1;
}

/* Executa as duas heuristicas em m e as confronta com o oraculo. Retorna o
 * tamanho minimo; grava em *tam_gulosa o tamanho da cobertura gulosa. */
static int checar_grafo(const Mascaras *m, const char *nome, int esperado, int *tam_gulosa)
{
    int ok = 1;
    GrafoCobertura *g = construir_grafo(m);
    int cob_bt[N_MAX_TESTE];
    int cob_gu[N_MAX_TESTE];

    if (g == NULL) {
        verificar(0, "alocacao do grafo de teste");
        return -1;
    }

    int minimo = oraculo_minimo(m);
    int tam_bt = cobertura_minima_bt(g, cob_bt);
    int tam_gu = cobertura_gulosa(g, cob_gu);
    uint32_t s_bt = flags_para_mascara(cob_bt, m->n);
    uint32_t s_gu = flags_para_mascara(cob_gu, m->n);

    ok &= tam_bt == minimo;
    ok &= tam_bt == __builtin_popcount(s_bt);
    ok &= mascara_cobre(m, s_bt) && eh_cobertura(g, cob_bt) == 1;
    ok &= tam_gu == __builtin_popcount(s_gu);
    ok &= mascara_cobre(m, s_gu) && eh_cobertura(g, cob_gu) == 1;
    ok &= tam_gu >= minimo;
    ok &= contar_arestas_cobertura(g) == mascaras_arestas(m) && grafo_igual(g, m);
    if (esperado >= 0) {
        ok &= minimo == esperado && tam_bt == esperado;
    }

    if (!ok) {
        char descricao[96];
        snprintf(descricao, sizeof descricao, "%s (n=%d): bt=%d guloso=%d oraculo=%d",
                 nome, m->n, tam_bt, tam_gu, minimo);
        verificar(0, descricao);
    } else {
        verificar(1, nome);
    }

    if (tam_gulosa != NULL) {
        *tam_gulosa = tam_gu;
    }
    liberar_grafo_cobertura(g);
    return minimo;
}

static void testar_casos_conhecidos(void)
{
    Mascaras m;
    char nome[48];

    printf("Casos conhecidos\n");

    for (int n = 0; n <= 5; n += 5) {
        mascaras_vazio(&m, n);
        snprintf(nome, sizeof nome, "grafo sem arestas n=%d", n);
        checar_grafo(&m, nome, 0, NULL);
    }

    mascaras_vazio(&m, 6);
    mascaras_aresta(&m, 0, 1);
    checar_grafo(&m, "aresta unica com vertices isolados", 1, NULL);
    {
        GrafoCobertura *g = construir_grafo(&m);
        int cob[6];
        cobertura_minima_bt(g, cob);
        verificar(!cob[2] && !cob[3] && !cob[4] && !cob[5], "isolados nao entram (exato)");
        cobertura_gulosa(g, cob);
        verificar(!cob[2] && !cob[3] && !cob[4] && !cob[5], "isolados nao entram (guloso)");
        liberar_grafo_cobertura(g);
    }

    for (int n = 1; n <= N_MAX_TESTE; n++) {
        mascaras_vazio(&m, n);
        for (int v = 0; v + 1 < n; v++) {
            mascaras_aresta(&m, v, v + 1);
        }
        snprintf(nome, sizeof nome, "caminho n=%d", n);
        checar_grafo(&m, nome, n / 2, NULL);
    }

    for (int n = 3; n <= N_MAX_TESTE; n++) {
        mascaras_vazio(&m, n);
        for (int v = 0; v < n; v++) {
            mascaras_aresta(&m, v, (v + 1) % n);
        }
        snprintf(nome, sizeof nome, "ciclo n=%d", n);
        checar_grafo(&m, nome, (n + 1) / 2, NULL);
    }

    for (int n = 1; n <= 10; n++) {
        mascaras_vazio(&m, n);
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                mascaras_aresta(&m, u, v);
            }
        }
        snprintf(nome, sizeof nome, "completo n=%d", n);
        checar_grafo(&m, nome, n - 1, NULL);
    }

    for (int n = 2; n <= N_MAX_TESTE; n++) {
        mascaras_vazio(&m, n);
        for (int v = 1; v < n; v++) {
            mascaras_aresta(&m, 0, v);
        }
        snprintf(nome, sizeof nome, "estrela n=%d", n);
        checar_grafo(&m, nome, 1, NULL);

        GrafoCobertura *g = construir_grafo(&m);
        int cob[N_MAX_TESTE];
        cobertura_minima_bt(g, cob);
        verificar(cob[0] == 1, "estrela: o centro e escolhido (exato)");
        cobertura_gulosa(g, cob);
        verificar(cob[0] == 1, "estrela: o centro e escolhido (guloso)");
        liberar_grafo_cobertura(g);
    }

    /* triangulo (2) + caminho de 4 vertices (2) + aresta (1) + isolado */
    mascaras_vazio(&m, 10);
    mascaras_aresta(&m, 0, 1);
    mascaras_aresta(&m, 1, 2);
    mascaras_aresta(&m, 0, 2);
    mascaras_aresta(&m, 3, 4);
    mascaras_aresta(&m, 4, 5);
    mascaras_aresta(&m, 5, 6);
    mascaras_aresta(&m, 7, 8);
    checar_grafo(&m, "grafo desconexo", 5, NULL);
}

static void testar_argumentos_invalidos(void)
{
    printf("Argumentos invalidos e eh_cobertura\n");

    GrafoCobertura *g = criar_grafo_cobertura(3);
    int cob[3] = { 0, 0, 0 };

    verificar(criar_grafo_cobertura(-1) == NULL, "n negativo");
    verificar(inserir_aresta_cobertura(g, 0, 1) == 0, "inserir aresta");
    verificar(inserir_aresta_cobertura(g, 1, 0) == -1, "aresta repetida");
    verificar(inserir_aresta_cobertura(g, 2, 2) == -1, "laco");
    verificar(inserir_aresta_cobertura(g, 0, 3) == -1, "vertice invalido");
    verificar(inserir_aresta_cobertura(NULL, 0, 1) == -1, "grafo nulo");
    verificar(contar_arestas_cobertura(NULL) == -1, "contar em grafo nulo");
    verificar(eh_cobertura(NULL, cob) == -1, "eh_cobertura com grafo nulo");
    verificar(eh_cobertura(g, NULL) == -1, "eh_cobertura com vetor nulo");
    verificar(cobertura_gulosa(NULL, cob) == -1, "guloso com grafo nulo");
    verificar(cobertura_gulosa(g, NULL) == -1, "guloso com vetor nulo");
    verificar(cobertura_minima_bt(NULL, cob) == -1, "exato com grafo nulo");
    verificar(cobertura_minima_bt(g, NULL) == -1, "exato com vetor nulo");

    verificar(eh_cobertura(g, cob) == 0, "conjunto vazio nao cobre aresta");
    cob[2] = 1;
    verificar(eh_cobertura(g, cob) == 0, "vertice nao incidente nao cobre");
    cob[1] = 1;
    verificar(eh_cobertura(g, cob) == 1, "um extremo basta");

    liberar_grafo_cobertura(NULL);
    liberar_grafo_cobertura(g);
}

/* Todo grafo simples rotulado com n <= 6 vertices; para n <= 5 tambem
 * confronta eh_cobertura com o teste independente em todos os subconjuntos. */
static void testar_exaustivo(void)
{
    printf("Exaustivo: todos os grafos simples com 1 a 6 vertices\n");

    int falhas_antes = total_falhas;
    long grafos = 0;
    long subconjuntos = 0;

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

            int antes = total_falhas;
            checar_grafo(&m, "grafo exaustivo", -1, NULL);
            grafos++;

            if (n <= 5) {
                GrafoCobertura *g = construir_grafo(&m);
                for (uint32_t s = 0; s < (1u << n); s++) {
                    int flags[N_MAX_TESTE];
                    for (int v = 0; v < n; v++) {
                        flags[v] = (int)(s >> v & 1u) * 7;
                    }
                    verificar(eh_cobertura(g, flags) == mascara_cobre(&m, s),
                              "eh_cobertura confere com o teste independente");
                    subconjuntos++;
                }
                liberar_grafo_cobertura(g);
            }
            if (total_falhas > antes + 20) {
                return;
            }
        }
    }

    printf("  %ld grafos e %ld subconjuntos verificados, %d falha(s)\n", grafos, subconjuntos,
           total_falhas - falhas_antes);
}

static void testar_aleatorios(void)
{
    printf("Grafos aleatorios reprodutiveis (7 a 15 vertices)\n");

    static const int densidades[] = { 15, 30, 50, 70, 90 };
    int falhas_antes = total_falhas;
    int grafos = 0;
    int gulosa_subotima = 0;

    semear(12345u);
    for (int n = 7; n <= N_MAX_TESTE; n++) {
        for (int d = 0; d < 5; d++) {
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
                int tam_gulosa;
                int minimo = checar_grafo(&m, "grafo aleatorio", -1, &tam_gulosa);
                gulosa_subotima += tam_gulosa > minimo;
                grafos++;
            }
        }
    }

    printf("  %d grafos verificados (guloso subotimo em %d), %d falha(s)\n", grafos,
           gulosa_subotima, total_falhas - falhas_antes);
}

typedef int (*Algoritmo)(const GrafoCobertura *, int *);

static double agora_ms(void)
{
    return (double)clock() * 1000.0 / (double)CLOCKS_PER_SEC;
}

/* Repete o algoritmo ate acumular TEMPO_MINIMO_MS e devolve o tempo medio
 * por execucao, contornando a resolucao do relogio. */
static double medir_ms(Algoritmo algoritmo, const GrafoCobertura *g, int *cobertura, long *execucoes)
{
    long repeticoes = 0;
    double inicio = agora_ms();
    double decorrido;

    do {
        algoritmo(g, cobertura);
        repeticoes++;
        decorrido = agora_ms() - inicio;
    } while (decorrido < TEMPO_MINIMO_MS);

    *execucoes = repeticoes;
    return decorrido / (double)repeticoes;
}

/* G(n, p) com p = 40%, semente fixa; regera ate ter ao menos uma aresta. */
static GrafoCobertura *gerar_grafo_experimento(int n)
{
    GrafoCobertura *g = criar_grafo_cobertura(n);
    if (g == NULL) {
        return NULL;
    }

    while (contar_arestas_cobertura(g) == 0) {
        for (int u = 0; u < n; u++) {
            for (int v = u + 1; v < n; v++) {
                if (proximo() % 100 < 40) {
                    inserir_aresta_cobertura(g, u, v);
                }
            }
        }
    }

    return g;
}

static int executar_experimento(void)
{
    static const int tamanhos[] = { 5, 8, 10, 12, 15 };
    int falhas = 0;

    printf("========================================\n");
    printf("EXPERIMENTO DE COBERTURA DE VERTICES\n");
    printf("Grafos G(n, 0.4), semente fixa; mesmo grafo para os dois algoritmos\n");
    printf("Tempo medio por execucao (repetido ate %.0f ms por algoritmo)\n", TEMPO_MINIMO_MS);
    printf("========================================\n");

    semear(2024u);
    for (int i = 0; i < 5; i++) {
        int n = tamanhos[i];
        int cob_bt[N_MAX_TESTE + 3];
        int cob_gu[N_MAX_TESTE + 3];
        long exec_bt, exec_gu;

        GrafoCobertura *g = gerar_grafo_experimento(n);
        if (g == NULL) {
            printf("Falha de alocacao para n=%d\n", n);
            return 1;
        }

        double ms_bt = medir_ms(cobertura_minima_bt, g, cob_bt, &exec_bt);
        double ms_gu = medir_ms(cobertura_gulosa, g, cob_gu, &exec_gu);
        int tam_bt = cobertura_minima_bt(g, cob_bt);
        int tam_gu = cobertura_gulosa(g, cob_gu);
        int valido_bt = eh_cobertura(g, cob_bt);
        int valido_gu = eh_cobertura(g, cob_gu);

        printf("\nVertices: %d\n", n);
        printf("Arestas: %d\n", contar_arestas_cobertura(g));
        printf("Backtracking exato:\n");
        printf("  Tamanho da cobertura minima: %d\n", tam_bt);
        printf("  Tempo de execucao: %.6f ms (%ld execucoes)\n", ms_bt, exec_bt);
        printf("  eh_cobertura: %s\n", valido_bt == 1 ? "valida" : "INVALIDA");
        printf("Heuristica gulosa:\n");
        printf("  Tamanho da cobertura: %d\n", tam_gu);
        printf("  Tempo de execucao: %.6f ms (%ld execucoes)\n", ms_gu, exec_gu);
        printf("  eh_cobertura: %s\n", valido_gu == 1 ? "valida" : "INVALIDA");

        if (valido_bt != 1 || valido_gu != 1 || tam_gu < tam_bt) {
            falhas++;
        }
        liberar_grafo_cobertura(g);
    }

    printf("\n========================================\n");
    return falhas;
}

int main(void)
{
    printf("========================================\n");
    printf("TESTES\n");
    printf("========================================\n");

    testar_casos_conhecidos();
    testar_argumentos_invalidos();
    testar_exaustivo();
    testar_aleatorios();

    printf("\n%d verificacoes, %d falha(s)\n\n", total_verificacoes, total_falhas);

    int falhas_experimento = executar_experimento();

    return total_falhas > 0 || falhas_experimento > 0;
}
