#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* Configuração 266: E2.4, H4, C1, A2 */

#define CODIGO_MAX_DIGITOS 13 /* códigos no estilo EAN-13 */
#define TAM_BLOCO_DOBRA 3     /* dígitos por bloco na função de dobra */
#define NOME_MAX 64           /* tamanho máximo do nome do produto */

typedef enum
{
    OK = 0,
    ERRO_CHAVE_INVALIDA, /* código vazio, sem dígitos ou longo demais */
    ERRO_DUPLICADA,      /* inserção de código que já existe */
    NAO_ENCONTRADO,      /* código não está na tabela */
    ERRO_MEMORIA         /* malloc falhou */
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

/* Métricas da tabela, usadas no menu, nos testes e no benchmark */
typedef struct
{
    unsigned long n;          /* chaves armazenadas */
    unsigned long m;          /* número de buckets */
    double alfa;              /* fator de carga n / m */
    unsigned long colisoes;   /* inserções que encontraram bucket ocupado */
    unsigned long ocupados;   /* buckets com pelo menos 1 produto */
    double cadeia_media;      /* n / ocupados (tamanho médio das cadeias não vazias) */
    unsigned long cadeia_max; /* maior cadeia */
} Estatisticas;

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

/* ---------- Operações ---------- */

/*
 * Insere um produto novo no fim da cadeia do seu bucket.
 * O código é normalizado antes do cálculo do índice.
 *
 * Conta uma colisão quando o bucket já tem pelo menos um produto.
 *
 * Retorna OK, ERRO_CHAVE_INVALIDA, ERRO_DUPLICADA (tabela não muda)
 * ou ERRO_MEMORIA.
 */
Status tabela_inserir(TabelaHash *t, const char *codigo,
                      const char *nome, int quantidade, double preco);

/*
 * Busca um produto pelo código (normalizado antes da busca).
 * Retorna ponteiro para o produto ou NULL se não existir
 * ou se o código for inválido.
 */
const Produto *tabela_buscar(const TabelaHash *t, const char *codigo);

/*
 * Remove o produto com o código (normalizado antes da busca).
 * O nó é desligado da lista circular dupla e liberado em O(1)
 * depois de encontrado; os demais nós da cadeia continuam acessíveis.
 *
 * O contador de colisões não é decrementado: ele registra quantas
 * inserções encontraram o bucket ocupado, e isso já aconteceu.
 *
 * Retorna OK, ERRO_CHAVE_INVALIDA ou NAO_ENCONTRADO.
 */
Status tabela_remover(TabelaHash *t, const char *codigo);

/*
 * Atualiza nome, quantidade e preço de um produto existente.
 * O código (chave) nunca muda, então o produto continua no mesmo
 * bucket e na mesma posição da cadeia.
 *
 * Atualização não é inserção: não altera n nem o contador de colisões,
 * e não insere o produto se ele não existir.
 *
 * Retorna OK, ERRO_CHAVE_INVALIDA ou NAO_ENCONTRADO.
 */
Status tabela_atualizar(TabelaHash *t, const char *codigo,
                        const char *nome, int quantidade, double preco);

/* ---------- Depuração e métricas ---------- */

/*
 * exibirEstrutura(): mostra cada bucket e sua cadeia, na ordem da lista.
 * Ex.: [600] -> 100200300 <-> 300200100 <-> 200100300
 * Com mostrar_vazios = 0, só os buckets ocupados são impressos
 * (útil com m grande). Ao final, imprime as estatísticas.
 */
void tabela_exibir_estrutura(const TabelaHash *t, int mostrar_vazios);

/* Lista todos os produtos: código, nome, quantidade e preço. */
void tabela_listar(const TabelaHash *t);

/* Calcula n, m, alfa, colisões, buckets ocupados e tamanho das cadeias. */
Estatisticas tabela_estatisticas(const TabelaHash *t);

/* Imprime as estatísticas numa linha. */
void imprimir_estatisticas(const Estatisticas *e);

#endif