#ifndef PLANARIDADE_H
#define PLANARIDADE_H

#include "grafo_lista.h"

/* Maior numero de vertices para o qual a busca exaustiva por
 * subdivisoes de K5 e K3,3 (Kuratowski) e executada. */
#define LIMITE_KURATOWSKI 10

typedef enum {
    /* m > 3n - 6: viola a condicao necessaria de Euler. */
    PLANARIDADE_NAO_PLANAR_EULER,
    /* Contem uma subdivisao de K5 (n <= LIMITE_KURATOWSKI). */
    PLANARIDADE_NAO_PLANAR_K5,
    /* Contem uma subdivisao de K3,3 (n <= LIMITE_KURATOWSKI). */
    PLANARIDADE_NAO_PLANAR_K33,
    /* n <= LIMITE_KURATOWSKI, passa em Euler e a busca exaustiva nao
     * encontrou subdivisao de K5 nem de K3,3: pelo Teorema de
     * Kuratowski, o grafo e planar. */
    PLANARIDADE_PLANAR,
    /* n > LIMITE_KURATOWSKI e passa em Euler: nenhuma conclusao, pois
     * Euler e apenas condicao necessaria e Kuratowski nao foi testado. */
    PLANARIDADE_INCONCLUSIVO,
    /* Grafo nulo. */
    PLANARIDADE_ERRO
} ResultadoPlanaridade;

/* Numero de arestas do grafo simples subjacente (lacos sao ignorados,
 * pois nao alteram a planaridade). Retorna -1 se g for nulo. */
int contar_arestas(const GrafoLista *g);

/* Verifica a condicao necessaria de Euler para grafos planares simples:
 * para n >= 3, m <= 3n - 6 (grafos com n < 3 sempre passam).
 * Retorna 1 se o grafo PASSA na condicao (o que NAO prova que ele e
 * planar), 0 se a viola (o que prova que NAO e planar) e -1 se g for
 * nulo. */
int eh_planar_euler(GrafoLista *g);

/* Busca exaustiva (forca bruta) por uma subdivisao de K5 em g, valida
 * apenas para g->n <= LIMITE_KURATOWSKI. Retorna 1 se encontrou, 0 se
 * nao existe e -1 se g for nulo ou grande demais. Se ramos nao for
 * NULL e a subdivisao existir, grava nele os 5 vertices de ramificacao. */
int tem_subdivisao_k5(GrafoLista *g, int ramos[5]);

/* Analoga a tem_subdivisao_k5 para K3,3. Se encontrada e ramos nao for
 * NULL, ramos[0..2] recebe um lado da biparticao e ramos[3..5] o outro. */
int tem_subdivisao_k33(GrafoLista *g, int ramos[6]);

/* Combina Euler e, para n <= LIMITE_KURATOWSKI, a busca por
 * subdivisoes de K5 e K3,3, devolvendo a conclusao mais forte que
 * esses testes permitem. */
ResultadoPlanaridade verificar_planaridade(GrafoLista *g);

#endif
