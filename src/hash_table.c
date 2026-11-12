#include <stddef.h>
#include <stdlib.h>
#include "hash_table.h"

/* ---------- Função hash (H4) ---------- */

Status normalizar_codigo(const char *entrada, char *saida)
{
    int tam = 0;

    if (entrada == NULL || saida == NULL)
        return ERRO_CHAVE_INVALIDA;

    for (const char *p = entrada; *p != '\0'; p++)
    {
        if (*p < '0' || *p > '9')
            continue; /* ignora espaços, hífens etc. */
        if (tam == CODIGO_MAX_DIGITOS)
        {
            saida[0] = '\0';
            return ERRO_CHAVE_INVALIDA; /* dígitos demais */
        }
        saida[tam++] = *p;
    }
    saida[tam] = '\0';

    return (tam == 0) ? ERRO_CHAVE_INVALIDA : OK;
}

unsigned long hash_dobra(const char *codigo, unsigned long m)
{
    unsigned long soma = 0, parte = 0;
    int digitos = 0;

    for (const char *p = codigo; *p != '\0'; p++)
    {
        parte = parte * 10 + (unsigned long)(*p - '0');
        if (++digitos == TAM_BLOCO_DOBRA)
        {
            soma += parte;
            parte = 0;
            digitos = 0;
        }
    }
    soma += parte; /* bloco final incompleto */

    return soma % m;
}

/* ---------- Ciclo de vida da tabela ---------- */

/* Cria um sentinela apontando para si mesmo (lista circular vazia). */
static No *criar_sentinela(void)
{
    No *s = malloc(sizeof(No));
    if (s == NULL)
        return NULL;
    s->prev = s;
    s->next = s;
    return s;
}

/* Libera todos os nós de uma cadeia, exceto o sentinela. */
static void liberar_cadeia(No *s)
{
    No *atual = s->next;
    while (atual != s)
    { /* para ao voltar ao sentinela */
        No *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
    s->prev = s;
    s->next = s;
}

TabelaHash *tabela_criar(unsigned long m)
{
    if (m == 0)
        return NULL;

    TabelaHash *t = malloc(sizeof(TabelaHash));
    if (t == NULL)
        return NULL;

    t->buckets = malloc(m * sizeof(No *));
    if (t->buckets == NULL)
    {
        free(t);
        return NULL;
    }

    for (unsigned long i = 0; i < m; i++)
    {
        t->buckets[i] = criar_sentinela();
        if (t->buckets[i] == NULL)
        {
            /* desfaz o que já foi alocado */
            for (unsigned long j = 0; j < i; j++)
                free(t->buckets[j]);
            free(t->buckets);
            free(t);
            return NULL;
        }
    }

    t->m = m;
    t->n = 0;
    t->colisoes = 0;
    return t;
}

void tabela_destruir(TabelaHash *t)
{
    if (t == NULL)
        return;

    for (unsigned long i = 0; i < t->m; i++)
    {
        liberar_cadeia(t->buckets[i]);
        free(t->buckets[i]); /* o próprio sentinela */
    }
    free(t->buckets);
    free(t);
}