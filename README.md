# Tabela Hash — Controle de Estoque (Configuração 266)

Trabalho da Segunda Unidade — Tabela Hash
Prof. Kennedy Lopes — UFERSA, Campus Pau dos Ferros

**Autor:**

- Naelson Fernandes da Fonsêca

---

## 1. Configuração atribuída

| Código   | Componente  | Escolha                                                                                          |
| -------- | ----------- | ------------------------------------------------------------------------------------------------ |
| **E2.4** | Estrutura   | Endereçamento fechado: vetor de buckets, cada um com uma **lista circular duplamente encadeada** |
| **H4**   | Função hash | **Dobra** (folding)                                                                              |
| **C1**   | Colisões    | **Encadeamento separado**                                                                        |
| **A2**   | Aplicação   | **Controle de estoque**                                                                          |

**Configuração nº 266 = (E2.4, H4, C1, A2)**

- **Chave:** código do produto (numérico, até 13 dígitos, no estilo EAN-13)
- **Valor:** nome, quantidade e preço

A implementação é toda própria, em C. Não usamos nenhuma biblioteca de tabela hash. As listas, a função hash e o gerenciamento de memória foram escritos pelo autor.

---

## 2. Estrutura de dados

A tabela é um vetor de `m` buckets. Cada bucket aponta para um **nó sentinela** de uma lista circular dupla. Um bucket vazio tem só o sentinela, que aponta para si mesmo.

```
 tabela[]
 ┌─────┐
 │  0  │──► [S] ⟲                                 (bucket vazio)
 ├─────┤
 │  1  │──► [S] ⇄ [P1] ⇄ [P2] ⇄ (volta para S)    (2 produtos: colisão)
 ├─────┤
 │  2  │──► [S] ⇄ [P3] ⇄ (volta para S)
 ├─────┤
 │ ... │
 ├─────┤
 │ m-1 │──► [S] ⟲
 └─────┘

 S = nó sentinela (não guarda produto)
 ⇄ = ponteiros prev/next
```

### Por que usar sentinela

Numa lista circular dupla sem sentinela, inserir e remover exigem tratar vários casos à parte: lista vazia, lista com um único nó e remoção do nó que é a cabeça. Com o sentinela, `prev` e `next` nunca são `NULL`, e as operações ficam com um único caminho de código:

```c
/* inserção no fim da cadeia: O(1) */
novo->prev = s->prev;
novo->next = s;
s->prev->next = novo;
s->prev = novo;

/* remoção de um nó qualquer: O(1) depois de encontrado */
no->prev->next = no->next;
no->next->prev = no->prev;
free(no);
```

O percurso termina quando a iteração volta ao sentinela:

```c
for (No *n = s->next; n != s; n = n->next) { ... }
```

### Registros

```c
typedef struct {
    char   codigo[14];   /* chave normalizada (até 13 dígitos + '\0') */
    char   nome[64];
    int    quantidade;
    double preco;
} Produto;

typedef struct No {
    Produto     produto;
    struct No  *prev;
    struct No  *next;
} No;

typedef struct {
    No           **buckets;    /* vetor de m sentinelas */
    unsigned long  m;          /* número de buckets */
    unsigned long  n;          /* chaves armazenadas */
    unsigned long  colisoes;   /* contador de colisões */
} TabelaHash;
```

---

## 3. Conversão da chave e cálculo do índice

### 3.1 Normalização

1. Todo caractere que não for dígito (espaços, hífens, pontos) é removido. Assim, `789-1000-315507` e `7891000315507` viram a mesma chave.
2. Se nenhum dígito sobrar, ou se sobrarem mais de 13, a chave é **inválida** e a operação é recusada.
3. Zeros à esquerda são mantidos. A comparação de chaves é feita sobre a string normalizada (`strcmp`).

### 3.2 Função de dobra (H4)

1. A chave normalizada é dividida, da esquerda para a direita, em blocos de **3 dígitos**. O último bloco pode ter menos.
2. Cada bloco é lido como inteiro e os blocos são **somados**.
3. O índice final é `soma mod m`, o que garante um resultado em `[0, m − 1]`.

```c
unsigned long hash_dobra(const char *codigo, unsigned long m) {
    unsigned long soma = 0, parte = 0;
    int digitos = 0;
    for (const char *p = codigo; *p; p++) {
        if (*p < '0' || *p > '9') continue;
        parte = parte * 10 + (*p - '0');
        if (++digitos == 3) { soma += parte; parte = 0; digitos = 0; }
    }
    soma += parte;          /* bloco final incompleto */
    return soma % m;
}
```

### 3.3 Exemplo calculado à mão (m = 1009)

Chave: `7891000315507`

```
Blocos:   789 | 100 | 031 | 550 | 7
Soma:     789 + 100 + 31 + 550 + 7 = 1477
Índice:   1477 mod 1009 = 468
```

### 3.4 Colisão intencional

A soma não depende da ordem dos blocos. Então chaves com os mesmos blocos em outra ordem sempre colidem:

```
100200300 → 100 + 200 + 300 = 600 → 600 mod 1009 = 600
300200100 → 300 + 200 + 100 = 600 → 600 mod 1009 = 600
200100300 → 200 + 100 + 300 = 600 → 600 mod 1009 = 600
```

As três chaves caem no bucket 600. Os testes usam esse fato para demonstrar colisões.

---

## 4. Políticas adotadas

| Situação                         | Comportamento                                                                                            |
| -------------------------------- | -------------------------------------------------------------------------------------------------------- |
| Colisão                          | O novo produto é inserido no fim da cadeia do bucket (encadeamento separado)                             |
| Inserção de chave duplicada      | **Recusada**: a função retorna `ERRO_DUPLICADA` e a tabela não muda. Para alterar dados, use `atualizar` |
| Busca de chave inexistente       | Retorna `NULL` / `NAO_ENCONTRADO`                                                                        |
| Atualização de chave inexistente | Retorna `NAO_ENCONTRADO` (não insere)                                                                    |
| Remoção de chave inexistente     | Retorna `NAO_ENCONTRADO`                                                                                 |
| Chave inválida                   | Retorna `ERRO_CHAVE_INVALIDA`                                                                            |
| Atualização                      | Altera nome, quantidade e preço. **O código (chave) nunca muda**                                         |

### Definição operacional de colisão

Uma colisão é contada quando um **novo** produto é inserido num bucket que já tem pelo menos um produto. Atualizações não contam como inserção.

---

## 5. Capacidade e parâmetros

| Parâmetro         | Valor      | Justificativa                                                                                                                                                |
| ----------------- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `m` (buckets)     | **1009**   | Número primo, o que ajuda a espalhar os restos do `mod`. Com 1009 buckets, α vai de ≈ 0,1 a ≈ 9,9 nos experimentos, mostrando bem o efeito do fator de carga |
| Tamanho do bloco  | 3 dígitos  | Com 13 dígitos são 5 blocos. A soma máxima é 4 × 999 + 9 = 4005, que cobre todos os índices de 0 a 1008                                                      |
| Redimensionamento | **Não há** | E2.4 não exige rehashing. Com encadeamento, α pode passar de 1 sem que a tabela "encha"                                                                      |

Fator de carga esperado nos experimentos (α = n / m):

| n      | m    | α      |
| ------ | ---- | ------ |
| 100    | 1009 | ≈ 0,10 |
| 1.000  | 1009 | ≈ 0,99 |
| 10.000 | 1009 | ≈ 9,91 |

---

## 6. Operações implementadas

| Função                    | Descrição                                                | Complexidade média |
| ------------------------- | -------------------------------------------------------- | ------------------ |
| `inserir(chave, valor)`   | Insere um produto novo e recusa duplicata                | O(1 + α)           |
| `buscar(chave)`           | Retorna o produto ou `NULL`                              | O(1 + α)           |
| `atualizar(chave, valor)` | Altera os dados sem mudar a chave                        | O(1 + α)           |
| `remover(chave)`          | Remove o nó da cadeia                                    | O(1 + α)           |
| `listar()`                | Lista todos os produtos                                  | O(m + n)           |
| `exibirEstrutura()`       | Mostra cada bucket e sua cadeia                          | O(m + n)           |
| Estatísticas              | n, m, α, colisões, buckets ocupados, cadeia média/máxima | O(m)               |

Exemplo de saída de `exibirEstrutura()`:

```
[  0] (vazio)
...
[468] -> 7891000315507
...
[600] -> 100200300 <-> 300200100 <-> 200100300
...
n = 4 | m = 1009 | α = 0.0040 | colisões = 2 | ocupados = 2
```

---

## 7. Estrutura do repositório

```
.
├── include/
│   └── hash_table.h       # tipos e protótipos
├── src/
│   ├── hash_table.c       # tabela, listas circulares duplas e função de dobra
│   └── main.c             # aplicação: menu de controle de estoque
├── tests/
│   └── testes.c           # testes de correção
├── bench/
│   └── benchmark.c        # análise experimental
├── data/                  # resultados dos experimentos (CSV)
├── docs/
│   └── slides.pdf         # slides da apresentação
├── Makefile
└── README.md
```

---

## 8. Como compilar e executar

Requisitos: `gcc` e `make`.

```bash
make            # compila a aplicação
./estoque       # executa o menu de estoque

make test       # compila e roda os testes de correção
make bench      # compila e roda a análise experimental

make clean      # remove os binários
```

Menu da aplicação:

```
1 - Inserir produto
2 - Consultar produto
3 - Atualizar produto
4 - Remover produto
5 - Listar produtos
6 - Exibir estrutura da tabela
7 - Estatísticas
0 - Sair
```

---

## 9. Testes de correção

Os testes ficam em `tests/testes.c`. O teste 8 (redimensionamento) não se aplica, porque é exclusivo de E1.2.

| #   | Caso                                    | Entrada                                         | Esperado                                | Obtido | Situação |
| --- | --------------------------------------- | ----------------------------------------------- | --------------------------------------- | ------ | -------- |
| 1   | Inserção e busca de chave existente     | inserir `7891000315507`; buscar `7891000315507` | produto encontrado                      |        |          |
| 2   | Busca de chave inexistente              | buscar `1111111111111`                          | `NAO_ENCONTRADO`                        |        |          |
| 3   | Inserção duplicada                      | inserir `7891000315507` de novo                 | `ERRO_DUPLICADA`, n não muda            |        |          |
| 4   | Colisão demonstrada                     | inserir `100200300`, `300200100`, `200100300`   | os três no bucket 600, colisões = 2     |        |          |
| 5   | Remoção em colisão                      | remover `300200100`; buscar os outros dois      | ambos encontrados                       |        |          |
| 6   | Atualização sem alterar a chave         | atualizar a quantidade de `100200300`           | dados novos, mesma chave e mesmo bucket |        |          |
| 7a  | Estrutura vazia                         | buscar/remover na tabela vazia                  | `NAO_ENCONTRADO`, sem falha             |        |          |
| 7b  | Remover o único nó da cadeia            | remover `7891000315507`                         | bucket 468 volta a ficar vazio          |        |          |
| 7c  | Remover o primeiro e o último da cadeia | remover as pontas da cadeia do bucket 600       | cadeia continua íntegra                 |        |          |
| 7d  | Chave inválida                          | inserir `abc`                                   | `ERRO_CHAVE_INVALIDA`                   |        |          |
| 7e  | Normalização                            | inserir `789-1000-315507`                       | tratada como `7891000315507`            |        |          |

> Preencher as colunas **Obtido** e **Situação** com a saída de `make test`.

---

## 10. Análise experimental

### Metodologia

- Cenários com **100, 1.000 e 10.000** produtos.
- Códigos de 13 dígitos gerados aleatoriamente, com **semente fixa** para que os conjuntos sejam reproduzíveis e comparáveis.
- **5 repetições** por cenário. A tabela mostra as médias.
- Tempo medido com `clock_gettime(CLOCK_MONOTONIC)`.
- **Busca bem-sucedida:** procura de todas as chaves inseridas.
- **Busca malsucedida:** procura de um mesmo número de chaves garantidamente ausentes.
- Mesmo `m` e mesma regra de medição em todas as repetições.

### Resultados

| Registros | Cap. (m) | α   | Colisões | Cadeia média | Cadeia máx. | Ins. (ms) | Busca ok (ms) | Busca aus. (ms) |
| --------- | -------- | --- | -------- | ------------ | ----------- | --------- | ------------- | --------------- |
| 100       | 1009     |     |          |              |             |           |               |                 |
| 1.000     | 1009     |     |          |              |             |           |               |                 |
| 10.000    | 1009     |     |          |              |             |           |               |                 |

> Preencher com a saída de `make bench`. Os dados brutos ficam em `data/`.

### Interpretação

_Preencher depois dos experimentos. Pontos a discutir:_

- Como o número de colisões e o tamanho médio das cadeias crescem com α.
- Por que a busca malsucedida tende a custar mais que a bem-sucedida: ela precisa percorrer a cadeia inteira até voltar ao sentinela.
- Se o tempo de busca acompanha o tamanho médio da cadeia, como prevê O(1 + α).

---

## 11. Discussão e limitações

- **Dobra por soma não depende da ordem:** chaves com os mesmos blocos em outra ordem sempre colidem. Isso foi útil para os testes, mas é uma fraqueza em dados reais.
- **Concentração dos valores:** a soma de 5 blocos se concentra no meio da faixa possível (de 0 a 4005), como uma soma de variáveis aleatórias. Alguns buckets tendem a receber mais produtos que outros. `exibirEstrutura()` e a métrica de cadeia máxima permitem observar isso.
- **Sem redimensionamento:** com `m` fixo, o desempenho piora linearmente com α. Uma melhoria possível seria fazer rehashing quando α passar de um limite.
- **Melhorias possíveis:** dobra por deslocamento (_shift folding_), que pesa blocos diferentes de formas diferentes, ou dobra com inversão alternada dos blocos (_boundary folding_), para reduzir colisões entre permutações.

---

## 12. Referências

- CORMEN, T. H. et al. _Algoritmos: Teoria e Prática_. 3. ed. Elsevier, 2012. Cap. 11 (Tabelas de Espalhamento).
- Slides da atividade "Trabalho da Segunda Unidade — Tabela Hash", Prof. Kennedy Lopes.
