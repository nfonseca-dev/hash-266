#ifndef HASH_TABLE_H
#define HASH_TABLE_H

/* Configuração 266: E2.4, H4, C1, A2 */

#define CODIGO_MAX_DIGITOS 13 /* códigos no estilo EAN-13 */
#define TAM_BLOCO_DOBRA 3     /* dígitos por bloco na função de dobra */

typedef enum
{
    OK = 0,
    ERRO_CHAVE_INVALIDA
} Status;

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

#endif