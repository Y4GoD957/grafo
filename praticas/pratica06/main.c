#include <stdio.h>
#include <stdlib.h>

#include "dijkstra.h"
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
        if (dist[v] == DIJKSTRA_INF) {
            printf("dist=INF");
        } else {
            printf("dist=%d", dist[v]);
        }
        printf(", pred=%d\n", pred[v]);
    }
}

/* Compara o caminho retornado com o esperado (terminado em -1). */
static int caminho_igual(const int *obtido, const int *esperado)
{
    if (obtido == NULL) {
        return 0;
    }
    int i = 0;
    while (esperado[i] != -1 && obtido[i] == esperado[i]) {
        i++;
    }
    return esperado[i] == -1 && obtido[i] == -1;
}

static void imprimir_caminho(const char *rotulo, const int *caminho)
{
    printf("  %s: ", rotulo);
    if (caminho == NULL) {
        printf("sem caminho\n");
        return;
    }
    for (int i = 0; caminho[i] != -1; i++) {
        printf(i == 0 ? "%d" : " -> %d", caminho[i]);
    }
    printf("\n");
}

static int posicoes_consistentes(const MinHeap *h)
{
    for (int i = 0; i < h->tamanho; i++) {
        if (h->posicao[h->dados[i].vertice] != i) {
            return 0;
        }
    }
    return 1;
}

static void teste_heap(void)
{
    printf("\n== Min-Heap ==\n");
    MinHeap *h = criar_min_heap(6);
    int ok = h != NULL;
    int chaves[6] = { 40, 10, 30, 20, 50, 5 };

    for (int v = 0; ok && v < 6; v++) {
        ok = inserir_min_heap(h, v, chaves[v]) == 0;
    }
    conferir(ok, "insere 6 vertices");
    conferir(inserir_min_heap(h, 0, 1) == -1, "rejeita vertice repetido");
    conferir(posicoes_consistentes(h), "posicao[] consistente apos insercoes");

    conferir(diminuir_chave(h, 4, 1) == 0, "diminuir_chave(v4, 50 -> 1)");
    conferir(diminuir_chave(h, 4, 99) == -1, "diminuir_chave rejeita aumento");
    conferir(posicoes_consistentes(h), "posicao[] consistente apos diminuir_chave");

    int esperado_v[6] = { 4, 5, 1, 3, 2, 0 };
    int esperado_d[6] = { 1, 5, 10, 20, 30, 40 };
    int ordem = 1;
    int consistente = 1;
    for (int i = 0; i < 6; i++) {
        NoHeap no;
        ordem = ordem && extrair_minimo(h, &no) == 0 &&
                no.vertice == esperado_v[i] && no.distancia == esperado_d[i];
        consistente = consistente && h->posicao[no.vertice] == -1 &&
                      posicoes_consistentes(h);
    }
    conferir(ordem, "extrai em ordem crescente (1, 5, 10, 20, 30, 40)");
    conferir(consistente, "posicao[] consistente apos cada extracao");
    NoHeap vazio;
    conferir(extrair_minimo(h, &vazio) == -1, "extrair de heap vazio retorna -1");
    liberar_min_heap(h);
}

static void teste_grafo_principal(void)
{
    printf("\n== Grafo A: rotas alternativas e inalcancavel (origem 0) ==\n");
    GrafoListaPonderado *g = criar_grafo_ponderado(7);
    int arestas[][3] = {
        { 0, 1, 7 }, { 0, 2, 9 }, { 0, 5, 14 }, { 1, 2, 10 }, { 1, 3, 15 },
        { 2, 3, 11 }, { 2, 5, 2 }, { 3, 4, 6 }, { 4, 5, 9 }
    };
    int ok = g != NULL;
    for (int i = 0; ok && i < 9; i++) {
        ok = inserir_aresta_ponderada(g, arestas[i][0], arestas[i][1],
                                      arestas[i][2]) == 0;
    }
    conferir(ok, "grafo montado (vertice 6 isolado)");

    int dist[7], pred[7];
    conferir(dijkstra(g, 0, dist, pred) == 0, "dijkstra retorna 0");
    imprimir_tabela(7, dist, pred);

    int dist_esp[7] = { 0, 7, 9, 20, 20, 11, DIJKSTRA_INF };
    int pred_esp[7] = { -1, 0, 0, 2, 5, 2, -1 };
    int igual = 1;
    for (int v = 0; v < 7; v++) {
        igual = igual && dist[v] == dist_esp[v] && pred[v] == pred_esp[v];
    }
    conferir(igual, "distancias e predecessores esperados");

    int *p = reconstruir_caminho(0, 4, pred);
    imprimir_caminho("0 -> 4", p);
    int esp04[] = { 0, 2, 5, 4, -1 };
    conferir(caminho_igual(p, esp04), "caminho 0,2,5,4 (custo 20; 0,2,3,4 custaria 26)");
    free(p);

    p = reconstruir_caminho(0, 5, pred);
    imprimir_caminho("0 -> 5", p);
    int esp05[] = { 0, 2, 5, -1 };
    conferir(caminho_igual(p, esp05), "0 -> 5 por 2 (11) em vez da aresta direta (14)");
    free(p);

    p = reconstruir_caminho(0, 0, pred);
    imprimir_caminho("0 -> 0", p);
    int esp00[] = { 0, -1 };
    conferir(caminho_igual(p, esp00), "origem == destino retorna {0, -1}");
    free(p);

    p = reconstruir_caminho(0, 6, pred);
    imprimir_caminho("0 -> 6", p);
    conferir(p == NULL, "destino inalcancavel retorna NULL");

    conferir(dijkstra(g, 7, dist, pred) == -1, "origem invalida rejeitada");
    conferir(dijkstra(g, -1, dist, pred) == -1, "origem negativa rejeitada");
    conferir(dijkstra(NULL, 0, dist, pred) == -1, "grafo nulo rejeitado");
    conferir(dijkstra(g, 0, NULL, pred) == -1, "dist nulo rejeitado");
    liberar_grafo_ponderado(g);
}

static void teste_zero(void)
{
    printf("\n== Grafo B: arestas de peso zero (origem 0) ==\n");
    GrafoListaPonderado *g = criar_grafo_ponderado(4);
    int ok = g != NULL;
    ok = ok && inserir_aresta_ponderada(g, 0, 1, 0) == 0;
    ok = ok && inserir_aresta_ponderada(g, 1, 2, 0) == 0;
    ok = ok && inserir_aresta_ponderada(g, 0, 2, 5) == 0;
    ok = ok && inserir_aresta_ponderada(g, 2, 3, 1) == 0;
    conferir(ok, "grafo montado");

    int dist[4], pred[4];
    conferir(dijkstra(g, 0, dist, pred) == 0, "dijkstra retorna 0");
    imprimir_tabela(4, dist, pred);
    conferir(dist[0] == 0 && dist[1] == 0 && dist[2] == 0 && dist[3] == 1,
             "distancias 0, 0, 0, 1");
    int *p = reconstruir_caminho(0, 3, pred);
    imprimir_caminho("0 -> 3", p);
    int esp[] = { 0, 1, 2, 3, -1 };
    conferir(caminho_igual(p, esp), "caminho 0,1,2,3 por arestas de peso zero");
    free(p);
    liberar_grafo_ponderado(g);
}

static void teste_dirigido_e_erros(void)
{
    printf("\n== Grafo C: arcos dirigidos, pesos invalidos, overflow ==\n");
    GrafoListaPonderado *g = criar_grafo_ponderado(3);
    conferir(inserir_arco_ponderado(g, 0, 1, 1) == 0, "arco 0 -> 1 (1)");
    conferir(inserir_arco_ponderado(g, 2, 1, 1) == 0, "arco 2 -> 1 (1)");
    conferir(inserir_arco_ponderado(g, 0, 1, 3) == -1, "arco repetido rejeitado");
    conferir(inserir_arco_ponderado(g, 0, 0, 1) == -1, "laco rejeitado");
    conferir(inserir_arco_ponderado(g, 0, 3, 1) == -1, "vertice invalido rejeitado");
    conferir(inserir_arco_ponderado(g, 1, 2, -4) == -1, "peso negativo rejeitado na insercao");
    conferir(inserir_aresta_ponderada(g, 1, 2, -4) == -1, "peso negativo rejeitado (aresta)");

    int dist[3], pred[3];
    conferir(dijkstra(g, 0, dist, pred) == 0, "dijkstra retorna 0");
    imprimir_tabela(3, dist, pred);
    conferir(dist[2] == DIJKSTRA_INF && pred[2] == -1,
             "v2 inalcancavel (o arco aponta de 2 para 1)");
    int *p = reconstruir_caminho(0, 2, pred);
    conferir(p == NULL, "0 -> 2 retorna NULL");

    /* Forca um peso negativo por fora da API: dijkstra deve recusar. */
    g->adj[0]->peso = -1;
    dist[0] = 123;
    conferir(dijkstra(g, 0, dist, pred) == -1 && dist[0] == 123,
             "dijkstra recusa peso negativo sem alterar dist");

    /* Peso INT_MAX (reservado ao infinito) forcado por fora da API. */
    g->adj[0]->peso = DIJKSTRA_INF;
    for (int v = 0; v < 3; v++) {
        dist[v] = 123;
        pred[v] = 77;
    }
    conferir(dijkstra(g, 0, dist, pred) == -1 &&
             dist[0] == 123 && dist[1] == 123 && dist[2] == 123 &&
             pred[0] == 77 && pred[1] == 77 && pred[2] == 77,
             "dijkstra recusa peso INT_MAX sem alterar dist/pred");
    liberar_grafo_ponderado(g);

    GrafoListaPonderado *r = criar_grafo_ponderado(2);
    conferir(inserir_arco_ponderado(r, 0, 1, DIJKSTRA_INF) == -1,
             "peso INT_MAX rejeitado na insercao (arco)");
    conferir(inserir_aresta_ponderada(r, 0, 1, DIJKSTRA_INF) == -1,
             "peso INT_MAX rejeitado na insercao (aresta)");
    conferir(r != NULL && r->adj[0] == NULL && r->adj[1] == NULL,
             "insercao rejeitada nao altera o grafo");
    conferir(inserir_aresta_ponderada(r, 0, 1, PESO_MAXIMO) == 0,
             "peso INT_MAX - 1 (maximo valido) aceito");
    liberar_grafo_ponderado(r);

    GrafoListaPonderado *o = criar_grafo_ponderado(3);
    inserir_arco_ponderado(o, 0, 1, DIJKSTRA_INF - 1);
    inserir_arco_ponderado(o, 1, 2, 5);
    conferir(dijkstra(o, 0, dist, pred) == 0, "dijkstra com pesos extremos");
    imprimir_tabela(3, dist, pred);
    conferir(dist[1] == DIJKSTRA_INF - 1 && dist[2] == DIJKSTRA_INF && pred[2] == -1,
             "soma que estouraria int nao gera distancia negativa");
    liberar_grafo_ponderado(o);

    conferir(criar_grafo_ponderado(-1) == NULL, "n negativo rejeitado");
    GrafoListaPonderado *vazio = criar_grafo_ponderado(0);
    conferir(vazio != NULL && dijkstra(vazio, 0, dist, pred) == -1,
             "grafo vazio: nenhuma origem valida");
    liberar_grafo_ponderado(vazio);
}

static void teste_pred_invalido(void)
{
    printf("\n== reconstruir_caminho com pred invalido ==\n");
    int ciclo[4] = { -1, 2, 3, 1 };     /* 1 -> 2 -> 3 -> 1 */
    int sem_origem[3] = { -1, -1, 1 };  /* 2 -> 1 -> fim, origem 0 */
    conferir(reconstruir_caminho(0, 3, ciclo) == NULL, "ciclo nos predecessores -> NULL");
    conferir(reconstruir_caminho(0, 2, sem_origem) == NULL, "cadeia nao chega a origem -> NULL");
    conferir(reconstruir_caminho(0, -1, sem_origem) == NULL, "destino negativo -> NULL");
    conferir(reconstruir_caminho(0, 1, NULL) == NULL, "pred nulo -> NULL");
}

int main(void)
{
    printf("Pratica 06 - Menor caminho: Dijkstra com Min-Heap\n");
    teste_heap();
    teste_grafo_principal();
    teste_zero();
    teste_dirigido_e_erros();
    teste_pred_invalido();

    printf("\n%s (%d falha(s))\n",
           falhas == 0 ? "TODOS OS TESTES PASSARAM" : "HA FALHAS", falhas);
    return falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
