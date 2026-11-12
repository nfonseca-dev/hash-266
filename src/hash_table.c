#include <stddef.h>
#include "hash_table.h"

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