#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* Configuração 266: E2.4, H4, C1, A2 */

#define CODIGO_MAX_DIGITOS 13 /* códigos no estilo EAN-13 */
#define TAM_BLOCO_DOBRA 3     /* dígitos por bloco na função de dobra */
#define NOME_MAX 64           /* tamanho máximo do nome do produto */

typedef enum
{
    OK = 0,
    ERRO_CHAVE_INVALIDA
} Status;

/* Registro da aplicação A2 (controle de estoque) */
typedef struct
{
    char codigo[CODIGO_MAX_DIGITOS + 1]; /* chave normalizada */
    char nome[NOME_MAX];
    int quantidade;
    double preco;
} Produto;

/*
 * Nó da lista circular duplamente encadeada (E2.4).
 * Cada bucket tem um nó sentinela, que não guarda produto.
 * Num bucket vazio, sentinela->prev == sentinela->next == sentinela.
 */
typedef struct No
{
    Produto produto;
    struct No *prev;
    struct No *next;
} No;

typedef struct
{
    No **buckets;           /* vetor de m sentinelas */
    unsigned long m;        /* número de buckets */
    unsigned long n;        /* chaves armazenadas */
    unsigned long colisoes; /* contador de colisões */
} TabelaHash;

/* ---------- Função hash (H4) ---------- */

/*
 * Normaliza um código de produto: remove tudo que não for dígito.
 * Ex.: "789-1000-315507" -> "7891000315507".
 * Zeros à esquerda são mantidos.
 *
 * saida deve ter espaço para CODIGO_MAX_DIGITOS + 1 caracteres.
 * Retorna ERRO_CHAVE_INVALIDA se não sobrar nenhum dígito
 * ou se sobrarem mais de CODIGO_MAX_DIGITOS.
 */
Status normalizar_codigo(const char *entrada, char *saida);

/*
 * Função hash por dobra (H4).
 * Divide o código normalizado em blocos de TAM_BLOCO_DOBRA dígitos,
 * da esquerda para a direita, soma os blocos e devolve soma mod m.
 * Ex.: "7891000315507" -> 789+100+31+550+7 = 1477 -> 1477 mod 1009 = 468.
 *
 * Sempre devolve um índice em [0, m-1].
 */
unsigned long hash_dobra(const char *codigo, unsigned long m);

/* ---------- Ciclo de vida da tabela ---------- */

/*
 * Cria uma tabela com m buckets, cada um com seu sentinela.
 * Retorna NULL se m == 0 ou se faltar memória.
 */
TabelaHash *tabela_criar(unsigned long m);

/*
 * Libera todos os nós de todas as cadeias, os sentinelas e a tabela.
 * Aceita NULL.
 */
void tabela_destruir(TabelaHash *t);

#endif