/*
 * Estrutura de Dados - 2o Trabalho Pratico (Pilha e Fila) - Profa. Andreia de Jesus
 * Integrantes:
 *   1) [OSMAR NUNES JÚNIOR]
 *   2) [CEZAR ANTHONIO LIMA LHEIO]
 */

/*
 * Trabalho 2 - Parte 1: Aplicacao de PILHA (alocacao encadeada)
 * Compilar: gcc pilha.c -o pilha -lm
 *
 * Operadores: + - * / ^ (^ = exponenciacao) e parenteses ( )
 * Aceita numeros inteiros e reais (ex.: 12.5). Use ponto como separador.
 * Menos unario: "-3" valido no inicio, depois de "(" ou de outro operador.
 *               "-(...)" vira "-1 * (...)".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 256       /* tamanho maximo de uma expressao / vetor de tokens */
#define MAX_EXPR 100  /* quantidade maxima de expressoes na opcao 4 */

/* tipo: 'n' = numero (valor em val), '+','-','*','/','^','(' ou ')' */
typedef struct {
    char tipo;
    double val;
} Token;

/* No da pilha encadeada */
typedef struct No {
    Token t;
    struct No *prox;
} No;

/* ---------- Pilha ---------- */

void empilha(No **topo, Token t) {
    No *n = (No *) malloc(sizeof(No));
    if (n == NULL) {
        printf("Sem memoria!\n");
        exit(1);
    }
    n->t = t;
    n->prox = *topo;
    *topo = n;
}

Token desempilha(No **topo) {
    No *n = *topo;
    Token t = n->t;
    *topo = n->prox;
    free(n);
    return t;
}

int vazia(No *topo) {
    return topo == NULL;
}

void esvazia(No **topo) {
    while (!vazia(*topo))
        desempilha(topo);
}

/* ---------- Auxiliares ---------- */

int ehOperador(char c) {
    return c != '\0' && strchr("+-*/^", c) != NULL;
}

int prec(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    if (op == '^') return 3;
    return 0;
}

/* Copia a expressao trocando simbolos alternativos pelos usados no programa
   (os do enunciado, em UTF-8):  ÷ -> /   · e × -> *   – — − -> -
   e chaves/colchetes tratados como parenteses. */
void normaliza(const char *src, char *dst) {
    int i = 0, j = 0;
    while (src[i] != '\0') {
        unsigned char c = (unsigned char) src[i];
        unsigned char c1 = (unsigned char) src[i + 1];
        unsigned char c2 = (c1 != 0) ? (unsigned char) src[i + 2] : 0;

        if (c == 0xC3 && c1 == 0xB7) {
            dst[j++] = '/';
            i += 2;
        } else if ((c == 0xC2 && c1 == 0xB7) || (c == 0xC3 && c1 == 0x97)) {
            dst[j++] = '*';
            i += 2;
        } else if ((c == 0xE2 && c1 == 0x80 && (c2 == 0x93 || c2 == 0x94)) ||
                   (c == 0xE2 && c1 == 0x88 && c2 == 0x92)) {
            dst[j++] = '-';
            i += 3;
        } else if (c == '{' || c == '[') {
            dst[j++] = '(';
            i++;
        } else if (c == '}' || c == ']') {
            dst[j++] = ')';
            i++;
        } else {
            dst[j++] = src[i];
            i++;
        }
    }
    dst[j] = '\0';
}

/* Transforma o texto em vetor de tokens. Retorna 1 se ok, 0 se erro. */
int tokeniza(const char *orig, Token *t, int *n) {
    char s[MAX];
    int i = 0;
    *n = 0;
    if (strlen(orig) >= MAX)
        return 0;
    normaliza(orig, s);
    while (s[i] != '\0') {
        char c = s[i];
        int neg = 0;

        if (isspace((unsigned char) c)) {
            i++;
            continue;
        }
        if (*n >= MAX - 3)
            return 0;

        /* '-' e unario se for o primeiro token, ou vier apos '(' ou operador */
        if (c == '-' && (*n == 0 || t[*n - 1].tipo == '(' || ehOperador(t[*n - 1].tipo))) {
            int j = i + 1;
            while (isspace((unsigned char) s[j]))
                j++;
            if (isdigit((unsigned char) s[j]) || s[j] == '.') {
                neg = 1;           /* numero negativo */
                i = j;
                c = s[i];
            } else if (s[j] == '(') {
                t[*n].tipo = 'n';  /* -(...) vira -1 * (...) */
                t[*n].val = -1;
                (*n)++;
                t[*n].tipo = '*';
                t[*n].val = 0;
                (*n)++;
                i = j;
                continue;
            } else {
                return 0;
            }
        }

        if (isdigit((unsigned char) c) || c == '.') {
            char *fim;
            double v;
            if (c == '0' && (s[i + 1] == 'x' || s[i + 1] == 'X'))
                return 0;  /* nao aceita hexadecimal */
            v = strtod(s + i, &fim);
            if (fim == s + i)
                return 0;
            t[*n].tipo = 'n';
            t[*n].val = neg ? -v : v;
            (*n)++;
            i = (int) (fim - s);
        } else if (ehOperador(c) || c == '(' || c == ')') {
            t[*n].tipo = c;
            t[*n].val = 0;
            (*n)++;
            i++;
        } else {
            return 0;  /* caractere invalido */
        }
    }
    return 1;
}

/* Verifica se a sequencia de tokens forma uma expressao valida */
int valida(Token *t, int n) {
    int esperaOperando = 1, nivel = 0, i;
    if (n == 0)
        return 0;
    for (i = 0; i < n; i++) {
        if (t[i].tipo == 'n') {
            if (!esperaOperando) return 0;
            esperaOperando = 0;
        } else if (t[i].tipo == '(') {
            if (!esperaOperando) return 0;
            nivel++;
        } else if (t[i].tipo == ')') {
            if (esperaOperando) return 0;
            nivel--;
            if (nivel < 0) return 0;
        } else {  /* operador */
            if (esperaOperando) return 0;
            esperaOperando = 1;
        }
    }
    return !esperaOperando && nivel == 0;
}

/* Decide se o operador do topo deve sair antes de empilhar o atual.
   modo 0 = posfixa, modo 1 = prefixa (usada sobre a expressao invertida) */
int deveDesempilhar(char topo, char atual, int modo) {
    int pt = prec(topo), pa = prec(atual);
    int igual = (modo == 0 && atual != '^') || (modo == 1 && atual == '^');
    return igual ? pt >= pa : pt > pa;
}

/* Algoritmo com pilha de operadores: infixa -> posfixa (modo 0) */
void converte(Token *in, int n, Token *out, int *m, int modo) {
    No *pilha = NULL;
    int i;
    *m = 0;
    for (i = 0; i < n; i++) {
        Token x = in[i];
        if (x.tipo == 'n') {
            out[(*m)++] = x;
        } else if (x.tipo == '(') {
            empilha(&pilha, x);
        } else if (x.tipo == ')') {
            while (!vazia(pilha) && pilha->t.tipo != '(')
                out[(*m)++] = desempilha(&pilha);
            if (!vazia(pilha))
                desempilha(&pilha);  /* descarta o '(' */
        } else {
            while (!vazia(pilha) && pilha->t.tipo != '(' &&
                   deveDesempilhar(pilha->t.tipo, x.tipo, modo))
                out[(*m)++] = desempilha(&pilha);
            empilha(&pilha, x);
        }
    }
    while (!vazia(pilha))
        out[(*m)++] = desempilha(&pilha);
}

void infixaParaPosfixa(Token *in, int n, Token *out, int *m) {
    converte(in, n, out, m, 0);
}

/* Prefixa: inverte a expressao (trocando parenteses), converte e inverte o resultado */
void infixaParaPrefixa(Token *in, int n, Token *out, int *m) {
    Token rev[MAX], tmp[MAX];
    int i;
    for (i = 0; i < n; i++) {
        rev[i] = in[n - 1 - i];
        if (rev[i].tipo == '(') rev[i].tipo = ')';
        else if (rev[i].tipo == ')') rev[i].tipo = '(';
    }
    converte(rev, n, tmp, m, 1);
    for (i = 0; i < *m; i++)
        out[i] = tmp[*m - 1 - i];
}

/* Escreve os tokens em texto separados por espaco */
void formata(Token *t, int n, char *buf) {
    int i, pos = 0;
    buf[0] = '\0';
    for (i = 0; i < n; i++) {
        if (t[i].tipo == 'n')
            pos += sprintf(buf + pos, "%.10g ", t[i].val);
        else
            pos += sprintf(buf + pos, "%c ", t[i].tipo);
    }
}

/* Calcula expressao posfixa com pilha. Retorna 0 ok, 1 divisao por zero, 2 resultado invalido */
int calcula(Token *p, int n, double *res) {
    No *pilha = NULL;
    int i, erro = 0;
    for (i = 0; i < n && !erro; i++) {
        if (p[i].tipo == 'n') {
            empilha(&pilha, p[i]);
        } else {
            Token b = desempilha(&pilha);
            Token a = desempilha(&pilha);
            Token r;
            r.tipo = 'n';
            r.val = 0;
            switch (p[i].tipo) {
                case '+': r.val = a.val + b.val; break;
                case '-': r.val = a.val - b.val; break;
                case '*': r.val = a.val * b.val; break;
                case '/':
                    if (b.val == 0) erro = 1;
                    else r.val = a.val / b.val;
                    break;
                case '^': r.val = pow(a.val, b.val); break;
            }
            /* x - x != 0 so acontece quando x e infinito ou NaN */
            if (!erro && (r.val - r.val != 0))
                erro = 2;
            if (!erro)
                empilha(&pilha, r);
        }
    }
    if (!erro)
        *res = desempilha(&pilha).val;
    esvazia(&pilha);
    return erro;
}

/* ---------- Entrada e opcoes do menu ---------- */

void leLinha(char *buf, int tam) {
    if (fgets(buf, tam, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\r\n")] = '\0';
}

/* Le e prepara uma expressao; retorna 1 se valida */
int preparaExpressao(const char *s, Token *t, int *n) {
    return tokeniza(s, t, n) && valida(t, *n);
}

void opcaoPosfixa(void) {
    char s[MAX], buf[MAX * 24];
    Token t[MAX], out[MAX];
    int n, m;
    printf("Expressao infixa: ");
    leLinha(s, MAX);
    if (!preparaExpressao(s, t, &n)) {
        printf("Expressao invalida!\n");
        return;
    }
    infixaParaPosfixa(t, n, out, &m);
    formata(out, m, buf);
    printf("Posfixa: %s\n", buf);
}

void opcaoPrefixa(void) {
    char s[MAX], buf[MAX * 24];
    Token t[MAX], out[MAX];
    int n, m;
    printf("Expressao infixa: ");
    leLinha(s, MAX);
    if (!preparaExpressao(s, t, &n)) {
        printf("Expressao invalida!\n");
        return;
    }
    infixaParaPrefixa(t, n, out, &m);
    formata(out, m, buf);
    printf("Prefixa: %s\n", buf);
}

void opcaoValida(void) {
    char s[MAX];
    Token t[MAX];
    int n;
    printf("Expressao: ");
    leLinha(s, MAX);
    if (preparaExpressao(s, t, &n))
        printf("Expressao VALIDA.\n");
    else
        printf("Expressao INVALIDA.\n");
}

void opcaoResultados(void) {
    static char exprs[MAX_EXPR][MAX];
    int qtd = 0, i;

    printf("Digite as expressoes, uma por linha (linha vazia para terminar):\n");
    while (qtd < MAX_EXPR) {
        leLinha(exprs[qtd], MAX);
        if (exprs[qtd][0] == '\0')
            break;
        qtd++;
    }

    printf("\n--- Resultados ---\n");
    for (i = 0; i < qtd; i++) {
        Token t[MAX], pos[MAX];
        int n, m, erro;
        double res;

        if (!preparaExpressao(exprs[i], t, &n)) {
            printf("%s = expressao invalida\n", exprs[i]);
            continue;
        }
        infixaParaPosfixa(t, n, pos, &m);
        erro = calcula(pos, m, &res);
        if (erro == 0)
            printf("%s = %.10g\n", exprs[i], res);
        else if (erro == 1)
            printf("%s = erro: divisao por zero\n", exprs[i]);
        else
            printf("%s = erro: resultado invalido\n", exprs[i]);
    }
}

void mostraMenu(void) {
    printf("\n===== MENU =====\n");
    printf("1. Transformar expressao Infixa em Posfixa\n");
    printf("2. Transformar expressao Infixa em Prefixa\n");
    printf("3. Ler uma expressao e informar se e valida\n");
    printf("4. Ler expressoes e informar o resultado de cada uma\n");
    printf("5. Sair\n");
    printf("Opcao: ");
}

int main(void) {
    char linha[MAX];
    int op;
    do {
        mostraMenu();
        leLinha(linha, MAX);
        op = atoi(linha);
        if (linha[0] == '\0' && feof(stdin))
            op = 5;
        switch (op) {
            case 1: opcaoPosfixa(); break;
            case 2: opcaoPrefixa(); break;
            case 3: opcaoValida(); break;
            case 4: opcaoResultados(); break;
            case 5: printf("Saindo...\n"); break;
            default: printf("Opcao invalida!\n");
        }
    } while (op != 5);
    return 0;
}
