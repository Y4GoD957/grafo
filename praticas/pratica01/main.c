#include <stdio.h>

#include "grafo_matriz.h"
#include "grafo_lista.h"

static void separador(const char *titulo)
{
    printf("\n--- %s ---\n", titulo);
}

int main(void)
{
    const int N = 5;

    separador("Criacao dos grafos");
    GrafoMatriz *gm = criar_grafo_matriz(N);
    GrafoLista *gl = criar_grafo_lista(N);

    if (gm == NULL || gl == NULL) {
        printf("Falha ao criar os grafos.\n");
        liberar_grafo_matriz(gm);
        liberar_grafo_lista(gl);
        return 1;
    }
    printf("Grafo matriz criado com %d vertices.\n", gm->n);
    printf("Grafo lista criado com %d vertices.\n", gl->n);

    separador("Insercao de arestas");
    int arestas[][2] = { {0, 1}, {0, 2}, {1, 2}, {1, 3}, {3, 4} };
    int total_arestas = (int)(sizeof(arestas) / sizeof(arestas[0]));

    for (int i = 0; i < total_arestas; i++) {
        int u = arestas[i][0];
        int v = arestas[i][1];
        int ok_m = inserir_aresta_matriz(gm, u, v);
        int ok_l = inserir_aresta_lista(gl, u, v);
        printf("Inserir (%d, %d) -> matriz: %s | lista: %s\n",
               u, v, ok_m == 0 ? "OK" : "FALHOU", ok_l == 0 ? "OK" : "FALHOU");
    }

    separador("Consulta de adjacencia");
    int pares[][2] = { {0, 1}, {0, 3}, {3, 4} };
    for (int i = 0; i < 3; i++) {
        int u = pares[i][0];
        int v = pares[i][1];
        printf("Adjacentes (%d, %d) -> matriz: %d | lista: %d\n",
               u, v, sao_adjacentes_matriz(gm, u, v), sao_adjacentes_lista(gl, u, v));
    }

    separador("Consulta de grau");
    for (int v = 0; v < N; v++) {
        printf("Grau do vertice %d -> matriz: %d | lista: %d\n",
               v, grau_matriz(gm, v), grau_lista(gl, v));
    }

    separador("Insercao de aresta duplicada");
    printf("Inserir (0, 1) novamente -> matriz: %s | lista: %s\n",
           inserir_aresta_matriz(gm, 0, 1) == 0 ? "OK" : "FALHOU (duplicada)",
           inserir_aresta_lista(gl, 0, 1) == 0 ? "OK" : "FALHOU (duplicada)");

    separador("Remocao de aresta");
    printf("Remover (1, 2) -> matriz: %s | lista: %s\n",
           remover_aresta_matriz(gm, 1, 2) == 0 ? "OK" : "FALHOU",
           remover_aresta_lista(gl, 1, 2) == 0 ? "OK" : "FALHOU");

    separador("Consulta apos remocao");
    printf("Adjacentes (1, 2) -> matriz: %d | lista: %d\n",
           sao_adjacentes_matriz(gm, 1, 2), sao_adjacentes_lista(gl, 1, 2));
    printf("Grau do vertice 1 -> matriz: %d | lista: %d\n",
           grau_matriz(gm, 1), grau_lista(gl, 1));

    separador("Remocao de aresta inexistente");
    printf("Remover (1, 2) novamente -> matriz: %s | lista: %s\n",
           remover_aresta_matriz(gm, 1, 2) == 0 ? "OK" : "FALHOU (inexistente)",
           remover_aresta_lista(gl, 1, 2) == 0 ? "OK" : "FALHOU (inexistente)");
    printf("Remover (0, 4) (nunca existiu) -> matriz: %s | lista: %s\n",
           remover_aresta_matriz(gm, 0, 4) == 0 ? "OK" : "FALHOU (inexistente)",
           remover_aresta_lista(gl, 0, 4) == 0 ? "OK" : "FALHOU (inexistente)");

    separador("Casos relevantes da lista encadeada");
    printf("Grau do vertice 1 antes das remocoes -> lista: %d\n", grau_lista(gl, 1));
    printf("Remover primeiro no inserido (1, 3) -> lista: %s\n",
           remover_aresta_lista(gl, 1, 3) == 0 ? "OK" : "FALHOU");
    printf("Remover no intermediario (0, 1) -> lista: %s\n",
           remover_aresta_lista(gl, 0, 1) == 0 ? "OK" : "FALHOU");
    printf("Grau do vertice 1 apos remocoes -> lista: %d\n", grau_lista(gl, 1));

    separador("Vertice invalido e ponteiro nulo");
    printf("Grau do vertice 10 (invalido) -> matriz: %d | lista: %d\n",
           grau_matriz(gm, 10), grau_lista(gl, 10));
    printf("Adjacentes (-1, 0) (invalido) -> matriz: %d | lista: %d\n",
           sao_adjacentes_matriz(gm, -1, 0), sao_adjacentes_lista(gl, -1, 0));
    printf("Grau em grafo nulo -> matriz: %d | lista: %d\n",
           grau_matriz(NULL, 0), grau_lista(NULL, 0));

    separador("Grafo vazio (n = 0)");
    GrafoMatriz *gm_vazio = criar_grafo_matriz(0);
    GrafoLista *gl_vazio = criar_grafo_lista(0);
    printf("Grafo vazio criado -> matriz: %s | lista: %s\n",
           gm_vazio != NULL ? "OK" : "FALHOU", gl_vazio != NULL ? "OK" : "FALHOU");
    printf("Insercao em grafo vazio -> matriz: %s | lista: %s\n",
           inserir_aresta_matriz(gm_vazio, 0, 0) == 0 ? "OK" : "FALHOU (esperado)",
           inserir_aresta_lista(gl_vazio, 0, 0) == 0 ? "OK" : "FALHOU (esperado)");

    separador("Criacao invalida (n < 0)");
    GrafoMatriz *gm_invalido = criar_grafo_matriz(-1);
    GrafoLista *gl_invalido = criar_grafo_lista(-1);
    printf("Criar com n = -1 -> matriz: %s | lista: %s\n",
           gm_invalido == NULL ? "NULL (esperado)" : "ERRO",
           gl_invalido == NULL ? "NULL (esperado)" : "ERRO");

    separador("Liberacao da memoria");
    liberar_grafo_matriz(gm);
    liberar_grafo_lista(gl);
    liberar_grafo_matriz(gm_vazio);
    liberar_grafo_lista(gl_vazio);
    liberar_grafo_matriz(gm_invalido);
    liberar_grafo_lista(gl_invalido);
    liberar_grafo_matriz(NULL);
    liberar_grafo_lista(NULL);
    printf("Toda a memoria alocada foi liberada.\n");

    return 0;
}
