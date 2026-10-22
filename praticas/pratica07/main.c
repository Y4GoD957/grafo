#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "bellman_ford.h"
#include "grafo_ponderado.h"

static int falhas = 0;

static void conferir(int condicao, const char *descricao)
{
    printf("  [%s] %s\n", condicao ? "OK" : "FALHA", descricao);
    if (!condicao) {
        falhas++;
    }
}

static void imprimir_tabela(int n, const int *dist, const int *pred)
{
    for (int v = 0; v < n; v++) {
        printf("  v%d: ", v);
        if (dist[v] == BF_INF) {
            printf("dist=INF");
        } else {
            printf("dist=%d", dist[v]);
        }
        printf(", pred=%d\n", pred[v]);
    }
}

/* Monta um grafo com n vertices a partir de arcos {u, v, peso}. */
static GrafoListaPonderado *montar(int n, int m, const int arcos[][3])
{
    GrafoListaPonderado *g = criar_grafo_ponderado(n);
    for (int i = 0; g != NULL && i < m; i++) {
        if (inserir_arco_ponderado(g, arcos[i][0], arcos[i][1], arcos[i][2]) != 0) {
            liberar_grafo_ponderado(g);
            return NULL;
        }
    }
    return g;
}

static int peso_arco(const GrafoListaPonderado *g, int u, int v, long long *peso)
{
    for (const NoPonderado *e = g->adj[u]; e != NULL; e = e->prox) {
        if (e->destino == v) {
            *peso = e->peso;
            return 1;
        }
    }
    return 0;
}

/* Confere de forma independente: ciclo fechado, terminado em -1, com todos
 * os arcos existentes em g e peso total negativo. Se pertence_a != -1, exige
 * que o vertice faca parte do ciclo. */
static int ciclo_negativo_valido(const GrafoListaPonderado *g, const int *ciclo,
                                 int pertence_a)
{
    if (ciclo == NULL) {
        return 0;
    }
    int k = 0;
    while (ciclo[k] != -1) {
        k++;
    }
    if (k < 2 || ciclo[0] != ciclo[k - 1]) {
        return 0;
    }
    long long total = 0;
    int contem = pertence_a < 0;
    for (int i = 0; i + 1 < k; i++) {
        long long w;
        if (ciclo[i] >= g->n || !peso_arco(g, ciclo[i], ciclo[i + 1], &w)) {
            return 0;
        }
        total += w;
        contem = contem || ciclo[i] == pertence_a;
    }
    return contem && total < 0;
}

/* Roda bellman_ford_detalhado esperando ciclo negativo e valida o ciclo. */
static int ciclo_detectado_e_valido(GrafoListaPonderado *g, int origem,
                                    int pertence_a, int imprimir)
{
    int dist[MAX_VERTICES], pred[MAX_VERTICES], afetado;
    if (bellman_ford_detalhado(g, origem, dist, pred, &afetado) != 0 ||
        afetado < 0) {
        return 0;
    }
    int *ciclo = reconstruir_ciclo_negativo(g, pred, afetado);
    if (imprimir) {
        printf("  ciclo: ");
        imprimir_ciclo(ciclo);
    }
    int ok = ciclo_negativo_valido(g, ciclo, pertence_a);
    free(ciclo);
    return ok;
}

static void teste_sem_ciclo(void)
{
    printf("\n== Pesos negativos sem ciclo negativo (origem 0) ==\n");
    const int arcos[][3] = {
        { 0, 1, 4 }, { 0, 2, 5 }, { 1, 2, -3 }, { 2, 3, 4 }, { 3, 4, -2 },
        { 4, 3, 5 }
    };
    GrafoListaPonderado *g = montar(7, 6, arcos);
    int dist[7], pred[7];
    conferir(g != NULL, "grafo montado (vertices 5 e 6 desconectados)");
    conferir(bellman_ford(g, 0, dist, pred) == 1, "bellman_ford retorna 1");
    imprimir_tabela(7, dist, pred);

    int dist_esp[7] = { 0, 4, 1, 5, 3, BF_INF, BF_INF };
    int pred_esp[7] = { -1, 0, 1, 2, 3, -1, -1 };
    int igual = 1;
    for (int v = 0; v < 7; v++) {
        igual = igual && dist[v] == dist_esp[v] && pred[v] == pred_esp[v];
    }
    conferir(igual, "distancias e predecessores esperados (0,4,1,5,3,INF,INF)");
    conferir(bellman_ford(g, 0, dist, pred) == 1 &&
             bellman_ford_entrada_valida(g, 0) == 1,
             "resultado repetido e entrada valida");
    liberar_grafo_ponderado(g);
}

static void teste_ciclos(void)
{
    printf("\n== Ciclos negativos alcancaveis ==\n");

    /* Ciclo 1 -> 2 -> 3 -> 1 (peso -1) que nao contem a origem 0. */
    const int a1[][3] = { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, -3 }, { 3, 1, 1 } };
    GrafoListaPonderado *g = montar(4, 4, a1);
    int dist[4], pred[4];
    conferir(bellman_ford(g, 0, dist, pred) == 0, "ciclo que nao contem a origem: retorna 0");
    conferir(ciclo_detectado_e_valido(g, 0, 1, 1), "ciclo valido, negativo e contem o vertice 1");
    liberar_grafo_ponderado(g);

    /* Alcancado por uma cauda 0 -> 1 -> 2; ciclo 3 -> 4 -> 5 -> 3 (peso -1). */
    const int a2[][3] = {
        { 0, 1, 2 }, { 1, 2, 2 }, { 2, 3, 2 }, { 3, 4, 1 }, { 4, 5, -3 },
        { 5, 3, 1 }
    };
    g = montar(6, 6, a2);
    int d6[6], p6[6], afetado;
    conferir(bellman_ford_detalhado(g, 0, d6, p6, &afetado) == 0, "cauda + ciclo: retorna 0");
    int *c = reconstruir_ciclo_negativo(g, p6, afetado);
    printf("  vertice afetado=%d, ", afetado);
    imprimir_ciclo(c);
    int sem_cauda = c != NULL;
    for (int i = 0; c != NULL && c[i] != -1; i++) {
        sem_cauda = sem_cauda && c[i] >= 3;
    }
    conferir(ciclo_negativo_valido(g, c, 3) && sem_cauda,
             "ciclo reconstruido tem so os vertices 3,4,5 (sem a cauda)");
    free(c);
    liberar_grafo_ponderado(g);

    /* Ciclo que contem a origem. */
    const int a3[][3] = { { 0, 1, -1 }, { 1, 0, -1 } };
    g = montar(2, 2, a3);
    conferir(ciclo_detectado_e_valido(g, 0, 0, 1), "ciclo que contem a origem");
    liberar_grafo_ponderado(g);

    /* Ciclo inalcancavel da origem e ignorado. */
    const int a4[][3] = { { 0, 1, 2 }, { 2, 3, -1 }, { 3, 2, -1 } };
    g = montar(4, 3, a4);
    int d4[4], p4[4];
    conferir(bellman_ford(g, 0, d4, p4) == 1, "ciclo inalcancavel e ignorado (retorna 1)");
    imprimir_tabela(4, d4, p4);
    conferir(d4[0] == 0 && d4[1] == 2 && d4[2] == BF_INF && d4[3] == BF_INF &&
             p4[2] == -1 && p4[3] == -1,
             "vertices do ciclo inalcancavel ficam com INF e pred -1");
    liberar_grafo_ponderado(g);
}

static void teste_lacos_e_triviais(void)
{
    printf("\n== Lacos negativos e grafos triviais ==\n");
    int dist[3], pred[3], afetado;

    const int a1[][3] = { { 0, 1, 1 }, { 1, 1, -1 } };
    GrafoListaPonderado *g = montar(2, 2, a1);
    conferir(bellman_ford_detalhado(g, 0, dist, pred, &afetado) == 0 && afetado == 1,
             "laco negativo alcancavel: retorna 0, vertice afetado 1");
    int *c = reconstruir_ciclo_negativo(g, pred, afetado);
    printf("  ");
    imprimir_ciclo(c);
    conferir(c != NULL && c[0] == 1 && c[1] == 1 && c[2] == -1 &&
             ciclo_negativo_valido(g, c, 1),
             "ciclo do laco e {1, 1, -1}");
    free(c);
    liberar_grafo_ponderado(g);

    const int a2[][3] = { { 0, 1, 1 }, { 2, 2, -1 } };
    g = montar(3, 2, a2);
    conferir(bellman_ford(g, 0, dist, pred) == 1, "laco negativo inalcancavel e ignorado");
    liberar_grafo_ponderado(g);

    const int a3[][3] = { { 1, 2, -4 } };
    g = montar(3, 1, a3);
    conferir(bellman_ford(g, 0, dist, pred) == 1 && dist[0] == 0 &&
             dist[1] == BF_INF && dist[2] == BF_INF &&
             pred[0] == -1 && pred[1] == -1 && pred[2] == -1,
             "origem sem arcos de saida: so ela e alcancavel");
    liberar_grafo_ponderado(g);

    g = criar_grafo_ponderado(1);
    conferir(bellman_ford(g, 0, dist, pred) == 1 && dist[0] == 0 && pred[0] == -1,
             "grafo com um unico vertice");
    conferir(inserir_arco_ponderado(g, 0, 0, -1) == 0 &&
             bellman_ford(g, 0, dist, pred) == 0 &&
             ciclo_detectado_e_valido(g, 0, 0, 0),
             "um vertice com laco negativo");
    liberar_grafo_ponderado(g);
}

static void teste_invalidos(void)
{
    printf("\n== Entradas invalidas e limites de peso ==\n");
    int dist[3], pred[3], afetado = 5;
    GrafoListaPonderado *g = criar_grafo_ponderado(3);
    inserir_arco_ponderado(g, 0, 1, 1);

    conferir(inserir_arco_ponderado(g, 1, 2, INT_MAX) == -1, "peso INT_MAX rejeitado");
    conferir(inserir_arco_ponderado(g, 1, 2, INT_MIN) == -1, "peso INT_MIN rejeitado");
    conferir(inserir_arco_ponderado(g, 1, 2, PESO_MAXIMO + 1) == -1, "peso PESO_MAXIMO + 1 rejeitado");
    conferir(inserir_arco_ponderado(g, 1, 2, -PESO_MAXIMO - 1) == -1, "peso -PESO_MAXIMO - 1 rejeitado");
    conferir(inserir_arco_ponderado(g, 0, 1, 2) == -1, "arco repetido rejeitado");
    conferir(inserir_arco_ponderado(g, 0, 3, 1) == -1, "vertice invalido rejeitado");
    conferir(inserir_arco_ponderado(NULL, 0, 1, 1) == -1, "grafo nulo rejeitado");
    conferir(inserir_arco_ponderado(g, 1, 2, PESO_MAXIMO) == 0, "peso PESO_MAXIMO aceito");
    conferir(inserir_arco_ponderado(g, 2, 0, -PESO_MAXIMO) == 0, "peso -PESO_MAXIMO aceito");
    conferir(criar_grafo_ponderado(-1) == NULL, "n negativo rejeitado");
    conferir(criar_grafo_ponderado(MAX_VERTICES + 1) == NULL, "n > MAX_VERTICES rejeitado");

    for (int v = 0; v < 3; v++) {
        dist[v] = 123;
        pred[v] = 77;
    }
    int mantidos;
#define MANTIDOS() (dist[0] == 123 && dist[1] == 123 && dist[2] == 123 && \
                    pred[0] == 77 && pred[1] == 77 && pred[2] == 77)
    mantidos = bellman_ford(g, 3, dist, pred) == 0 && MANTIDOS();
    conferir(mantidos, "origem invalida: retorna 0 e nao altera dist/pred");
    mantidos = bellman_ford(g, -1, dist, pred) == 0 && MANTIDOS();
    conferir(mantidos, "origem negativa: retorna 0 e nao altera dist/pred");
    mantidos = bellman_ford(NULL, 0, dist, pred) == 0 && MANTIDOS();
    conferir(mantidos, "grafo nulo: retorna 0");
    mantidos = bellman_ford(g, 0, NULL, pred) == 0 && MANTIDOS();
    conferir(mantidos, "dist nulo: retorna 0");
    mantidos = bellman_ford(g, 0, dist, NULL) == 0 && MANTIDOS();
    conferir(mantidos, "pred nulo: retorna 0");

    /* Peso fora da faixa inserido alterando a estrutura diretamente. */
    g->adj[0]->peso = INT_MAX;
    mantidos = bellman_ford_detalhado(g, 0, dist, pred, &afetado) == 0 &&
               afetado == -1 && MANTIDOS();
    conferir(mantidos && bellman_ford_entrada_valida(g, 0) == 0,
             "peso INT_MAX forcado: retorna 0, afetado -1, dist/pred intactos");
    g->adj[0]->peso = -PESO_MAXIMO - 1;
    conferir(bellman_ford(g, 0, dist, pred) == 0 && MANTIDOS(),
             "peso abaixo do limite forcado: retorna 0, dist/pred intactos");
    g->adj[0]->peso = 1;
#undef MANTIDOS

    conferir(reconstruir_ciclo_negativo(NULL, pred, 0) == NULL, "reconstruir: grafo nulo -> NULL");
    conferir(reconstruir_ciclo_negativo(g, NULL, 0) == NULL, "reconstruir: pred nulo -> NULL");
    conferir(reconstruir_ciclo_negativo(g, pred, -1) == NULL, "reconstruir: vertice invalido -> NULL");
    int sem_ciclo[3] = { -1, 0, 1 };
    conferir(reconstruir_ciclo_negativo(g, sem_ciclo, 2) == NULL,
             "reconstruir: pred sem ciclo -> NULL");
    int ciclo_positivo[3] = { 2, 0, 1 };  /* 0 -> 1 -> 2 -> 0, peso 1 + MAX - MAX */
    conferir(reconstruir_ciclo_negativo(g, ciclo_positivo, 0) == NULL,
             "reconstruir: ciclo de peso nao negativo -> NULL");
    liberar_grafo_ponderado(g);
}

static void teste_limites_numericos(void)
{
    printf("\n== Limites numericos (n = %d, peso = %d) ==\n", MAX_VERTICES, PESO_MAXIMO);
    int n = MAX_VERTICES;
    static int dist[MAX_VERTICES], pred[MAX_VERTICES];
    GrafoListaPonderado *g = criar_grafo_ponderado(n);

    for (int v = 0; v + 1 < n; v++) {
        inserir_arco_ponderado(g, v, v + 1, -PESO_MAXIMO);
    }
    conferir(bellman_ford(g, 0, dist, pred) == 1, "cadeia de pesos minimos: retorna 1");
    long long esperado = -(long long)PESO_MAXIMO * (n - 1);
    printf("  dist[%d] = %d\n", n - 1, dist[n - 1]);
    conferir(dist[n - 1] == esperado && esperado > (long long)INT_MIN,
             "distancia -1.999.000.000 representada sem underflow");
    liberar_grafo_ponderado(g);

    g = criar_grafo_ponderado(n);
    for (int v = 0; v + 1 < n; v++) {
        inserir_arco_ponderado(g, v, v + 1, PESO_MAXIMO);
    }
    conferir(bellman_ford(g, 0, dist, pred) == 1, "cadeia de pesos maximos: retorna 1");
    printf("  dist[%d] = %d\n", n - 1, dist[n - 1]);
    conferir(dist[n - 1] == (long long)PESO_MAXIMO * (n - 1) && dist[n - 1] != BF_INF,
             "distancia 1.999.000.000 nao colide com INF nem estoura");
    liberar_grafo_ponderado(g);

    /* Anel com todos os arcos em -PESO_MAXIMO: ciclo negativo gigante. */
    g = criar_grafo_ponderado(n);
    for (int v = 0; v < n; v++) {
        inserir_arco_ponderado(g, v, (v + 1) % n, -PESO_MAXIMO);
    }
    conferir(bellman_ford(g, 0, dist, pred) == 0, "anel negativo de 2000 vertices: retorna 0");
    conferir(ciclo_detectado_e_valido(g, 0, 0, 0), "ciclo do anel reconstruido e negativo");
    liberar_grafo_ponderado(g);

    /* Cadeia de pesos minimos terminando em ciclo: nenhum overflow interno. */
    g = criar_grafo_ponderado(n);
    for (int v = 0; v + 1 < n; v++) {
        inserir_arco_ponderado(g, v, v + 1, -PESO_MAXIMO);
    }
    inserir_arco_ponderado(g, n - 1, n - 2, -PESO_MAXIMO);
    conferir(ciclo_detectado_e_valido(g, 0, n - 1, 0),
             "cadeia longa + ciclo no fim: detectado sem overflow");
    liberar_grafo_ponderado(g);
}

/* Referencia independente: Floyd-Warshall em grafos pequenos. */
static unsigned long long semente = 12345;

static int aleatorio(int limite)
{
    semente = semente * 6364136223846793005ULL + 1442695040888963407ULL;
    return (int)((semente >> 33) % (unsigned long long)limite);
}

static void teste_aleatorio(void)
{
    printf("\n== Comparacao com Floyd-Warshall (grafos pseudoaleatorios) ==\n");
    int total = 600, divergencias = 0, com_ciclo = 0, sem_ciclo = 0;

    for (int t = 0; t < total; t++) {
        int n = 1 + aleatorio(7);
        GrafoListaPonderado *g = criar_grafo_ponderado(n);
        long long fw[7][7];
        const long long INF = 1000000000000LL;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                fw[i][j] = i == j ? 0 : INF;
            }
        }
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (aleatorio(100) < 35) {
                    int w = aleatorio(15) - 4;
                    if (inserir_arco_ponderado(g, u, v, w) == 0 && w < fw[u][v]) {
                        fw[u][v] = w;
                    }
                }
            }
        }
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (fw[i][k] < INF && fw[k][j] < INF &&
                        fw[i][k] + fw[k][j] < fw[i][j]) {
                        fw[i][j] = fw[i][k] + fw[k][j];
                    }
                }
            }
        }

        int origem = aleatorio(n);
        int ciclo_ref = 0;
        for (int v = 0; v < n; v++) {
            ciclo_ref = ciclo_ref || (fw[origem][v] < INF && fw[v][v] < 0);
        }

        int dist[7], pred[7], afetado;
        int r = bellman_ford_detalhado(g, origem, dist, pred, &afetado);
        int ok = r == (ciclo_ref ? 0 : 1);
        if (ok && ciclo_ref) {
            int *c = reconstruir_ciclo_negativo(g, pred, afetado);
            ok = ciclo_negativo_valido(g, c, -1);
            free(c);
            com_ciclo++;
        } else if (ok) {
            for (int v = 0; v < n; v++) {
                long long esp = fw[origem][v] < INF ? fw[origem][v] : BF_INF;
                ok = ok && dist[v] == esp;
            }
            for (int v = 0; v < n; v++) {
                /* pred coerente: dist[v] = dist[pred] + peso do arco */
                if (v != origem && dist[v] != BF_INF) {
                    long long w;
                    ok = ok && pred[v] >= 0 && peso_arco(g, pred[v], v, &w) &&
                         dist[v] == dist[pred[v]] + w;
                }
            }
            sem_ciclo++;
        }
        if (!ok) {
            divergencias++;
        }
        liberar_grafo_ponderado(g);
    }

    printf("  %d grafos: %d com ciclo alcancavel, %d sem ciclo, %d divergencias\n",
           total, com_ciclo, sem_ciclo, divergencias);
    conferir(divergencias == 0 && com_ciclo > 50 && sem_ciclo > 50,
             "resultado e ciclo/distancias concordam com a referencia em todos os casos");
}

int main(void)
{
    printf("Pratica 07 - Menor caminho: Bellman-Ford\n");
    teste_sem_ciclo();
    teste_ciclos();
    teste_lacos_e_triviais();
    teste_invalidos();
    teste_limites_numericos();
    teste_aleatorio();

    printf("\n%s (%d falha(s))\n",
           falhas == 0 ? "TODOS OS TESTES PASSARAM" : "HA FALHAS", falhas);
    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
