/*
 * Estrutura de Dados - 2o Trabalho Pratico (Pilha e Fila) - Profa. Andreia de Jesus
 * Integrantes:
 *   1) [OSMAR NUNES JÚNIOR]
 *   2) [CEZAR ANTHONIO LIMA LHEIO]
 */

/*
 * Trabalho 2 - Parte 2: Aplicacao de FILA (alocacao encadeada)
 * Compilar: gcc fila.c -o fila
 * Executar: ./fila            (le "solicitacoes.txt")
 *           ./fila arquivo.txt (le outro arquivo)
 *
 * Formato do arquivo: prioridade, identificacao; prioridade, identificacao; ...
 *   Prioridades: 1 = maxima (Fila 0), 2 = normal (Fila 1), 3 = baixa (Fila 2)
 */
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#define ESPERA() Sleep(500)       /* 0,5 segundo */
#else
#include <unistd.h>
#define ESPERA() usleep(500000)   /* 0,5 segundo */
#endif

#define POR_RODADA 5

/* No da fila: prioridade, identificacao (IP) e ponteiro para o proximo */
typedef struct No {
    int prioridade;
    char id[32];
    struct No *prox;
} No;

typedef struct {
    No *ini;
    No *fim;
} Fila;

void inicializa(Fila *f) {
    f->ini = NULL;
    f->fim = NULL;
}

int vazia(Fila *f) {
    return f->ini == NULL;
}

void enfileira(Fila *f, int prio, const char *id) {
    No *n = (No *) malloc(sizeof(No));
    if (n == NULL) {
        printf("Sem memoria!\n");
        exit(1);
    }
    n->prioridade = prio;
    strcpy(n->id, id);
    n->prox = NULL;
    if (vazia(f))
        f->ini = n;
    else
        f->fim->prox = n;
    f->fim = n;
}

/* Remove o primeiro da fila. Retorna 1 se removeu, 0 se a fila estava vazia */
int desenfileira(Fila *f, int *prio, char *id) {
    No *n = f->ini;
    if (n == NULL)
        return 0;
    *prio = n->prioridade;
    strcpy(id, n->id);
    f->ini = n->prox;
    if (f->ini == NULL)
        f->fim = NULL;
    free(n);
    return 1;
}

void esvazia(Fila *f) {
    int p;
    char id[32];
    while (desenfileira(f, &p, id))
        ;
}

void imprime(const char *nome, Fila *f) {
    No *n;
    int qtd = 0;
    for (n = f->ini; n != NULL; n = n->prox)
        qtd++;
    printf("%-8s (%2d): ", nome, qtd);
    if (qtd == 0)
        printf("vazia");
    for (n = f->ini; n != NULL; n = n->prox)
        printf("[%d|%s] ", n->prioridade, n->id);
    printf("\n");
}

/* Remove espacos e quebras de linha no fim do texto */
void aparaFim(char *s) {
    int i = (int) strlen(s) - 1;
    while (i >= 0 && isspace((unsigned char) s[i]))
        s[i--] = '\0';
}

/* Le o arquivo e carrega a fila de entrada. Retorna a quantidade lida, ou -1 se nao abriu */
int carregaArquivo(const char *nomeArq, Fila *entrada) {
    FILE *arq = fopen(nomeArq, "r");
    int prio, qtd = 0;
    char id[32];

    if (arq == NULL)
        return -1;
    while (fscanf(arq, " %d , %31[^;] ;", &prio, id) == 2) {
        aparaFim(id);
        if (prio < 1 || prio > 3) {
            printf("Prioridade invalida (%d) ignorada: %s\n", prio, id);
            continue;
        }
        enfileira(entrada, prio, id);
        qtd++;
    }
    fclose(arq);
    return qtd;
}

void mostraSituacao(Fila *entrada, Fila filas[3]) {
    imprime("Entrada", entrada);
    imprime("Fila 0", &filas[0]);
    imprime("Fila 1", &filas[1]);
    imprime("Fila 2", &filas[2]);
    printf("\n");
}

/* Descarrega ate POR_RODADA solicitacoes da entrada para as filas de prioridade */
void distribui(Fila *entrada, Fila filas[3]) {
    int k, prio;
    char id[32];
    for (k = 0; k < POR_RODADA && desenfileira(entrada, &prio, id); k++)
        enfileira(&filas[prio - 1], prio, id);
}

int main(int argc, char *argv[]) {
    const char *nomeArq = (argc > 1) ? argv[1] : "solicitacoes.txt";
    Fila entrada, filas[3];
    int total, rodada = 0, i;

    inicializa(&entrada);
    for (i = 0; i < 3; i++)
        inicializa(&filas[i]);

    total = carregaArquivo(nomeArq, &entrada);
    if (total < 0) {
        printf("Nao foi possivel abrir o arquivo '%s'.\n", nomeArq);
        return 1;
    }
    printf("%d solicitacoes carregadas na fila de entrada.\n\n", total);
    printf("Situacao inicial:\n");
    mostraSituacao(&entrada, filas);

    while (!vazia(&entrada)) {
        rodada++;
        distribui(&entrada, filas);
        printf("=== Rodada %d ===\n", rodada);
        mostraSituacao(&entrada, filas);
        fflush(stdout);
        ESPERA();
    }

    printf("Distribuicao concluida.\n");
    esvazia(&entrada);
    for (i = 0; i < 3; i++)
        esvazia(&filas[i]);
    return 0;
}
