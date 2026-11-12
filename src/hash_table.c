#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

/* ---------- Operações ---------- */

/*
 * Procura o nó com o código (já normalizado) na cadeia do bucket.
 * Retorna o nó ou NULL. Reaproveitada por buscar, atualizar e remover.
 */
static No *encontrar_no(const TabelaHash *t, const char *codigo)
{
    No *s = t->buckets[hash_dobra(codigo, t->m)];

    for (No *atual = s->next; atual != s; atual = atual->next)
    {
        if (strcmp(atual->produto.codigo, codigo) == 0)
            return atual;
    }
    return NULL;
}

Status tabela_inserir(TabelaHash *t, const char *codigo,
                      const char *nome, int quantidade, double preco)
{
    char chave[CODIGO_MAX_DIGITOS + 1];

    if (t == NULL || normalizar_codigo(codigo, chave) != OK)
        return ERRO_CHAVE_INVALIDA;

    if (encontrar_no(t, chave) != NULL)
        return ERRO_DUPLICADA;

    No *novo = malloc(sizeof(No));
    if (novo == NULL)
        return ERRO_MEMORIA;

    strcpy(novo->produto.codigo, chave);
    snprintf(novo->produto.nome, NOME_MAX, "%s", nome ? nome : "");
    novo->produto.quantidade = quantidade;
    novo->produto.preco = preco;

    No *s = t->buckets[hash_dobra(chave, t->m)];

    if (s->next != s) /* bucket já ocupado */
        t->colisoes++;

    /* insere no fim: entre o último nó (s->prev) e o sentinela */
    novo->prev = s->prev;
    novo->next = s;
    s->prev->next = novo;
    s->prev = novo;

    t->n++;
    return OK;
}

const Produto *tabela_buscar(const TabelaHash *t, const char *codigo)
{
    char chave[CODIGO_MAX_DIGITOS + 1];

    if (t == NULL || normalizar_codigo(codigo, chave) != OK)
        return NULL;

    No *no = encontrar_no(t, chave);
    return no ? &no->produto : NULL;
}

Status tabela_remover(TabelaHash *t, const char *codigo)
{
    char chave[CODIGO_MAX_DIGITOS + 1];

    if (t == NULL || normalizar_codigo(codigo, chave) != OK)
        return ERRO_CHAVE_INVALIDA;

    No *no = encontrar_no(t, chave);
    if (no == NULL)
        return NAO_ENCONTRADO;

    /* liga o anterior ao próximo, pulando o nó removido */
    no->prev->next = no->next;
    no->next->prev = no->prev;
    free(no);

    t->n--;
    return OK;
}

Status tabela_atualizar(TabelaHash *t, const char *codigo,
                        const char *nome, int quantidade, double preco)
{
    char chave[CODIGO_MAX_DIGITOS + 1];

    if (t == NULL || normalizar_codigo(codigo, chave) != OK)
        return ERRO_CHAVE_INVALIDA;

    No *no = encontrar_no(t, chave);
    if (no == NULL)
        return NAO_ENCONTRADO;

    /* só os dados mudam; codigo, prev e next ficam intactos */
    snprintf(no->produto.nome, NOME_MAX, "%s", nome ? nome : "");
    no->produto.quantidade = quantidade;
    no->produto.preco = preco;

    return OK;
}