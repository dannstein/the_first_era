# Paths of the Ancient Archivist
## Estrutura do Mundo, Regiões, Nós e Conexões

---

# Fórmula de Custo

Cada rota do grafo utiliza a seguinte fórmula:

C = D + 4P + 3T

Onde:

- D = Distância
- P = Perigo
- T = Dificuldade do terreno

---

# Escala dos Valores

| Intensidade | Distância | Perigo | Terreno |
|---|---|---|---|
| Baixo | 5–15 | 1–2 | 1–2 |
| Médio | 15–30 | 3–5 | 3–5 |
| Alto | 30–50 | 6–8 | 6–8 |
| Extremo | 50–80 | 9–10 | 9–10 |

---

# Biomas do Mundo

| Bioma | Perigo | Terreno | Descrição |
|---|---|---|---|
| Estradas Reais | 1 | 1 | Rotas seguras do antigo reino |
| Planícies | 1 | 1 | Terreno aberto e rápido |
| Colinas | 1 | 2 | Terreno moderadamente irregular |
| Florestas | 2 | 2 | Vegetação densa |
| Floresta Negra | 5 | 3 | Magia negra e criaturas sombrias |
| Montanhas | 3 | 5 | Escalada difícil |
| Cordilheiras | 5 | 7 | Travessia extremamente lenta |
| Ravinas | 4 | 6 | Caminhos estreitos e perigosos |
| Deserto | 2 | 5 | Consumo intenso de energia |
| Pântano | 4 | 8 | O pior terreno de locomoção |
| Terras Mortas | 7 | 5 | Energia necromântica |
| Lava / Terras Dracônicas | 8 | 7 | Região extremamente hostil |

---

# REGIÃO I — Eryndorheim
## (Picos Congelados do Norte)

Uma região antiga formada por montanhas geladas, ravinas congeladas e ruínas esquecidas pelos reinos humanos.

### Biomas
- Montanhas
- Neve
- Ravinas

### Nodes

| ID | Local |
|---|---|
| 1 | Fortaleza Congelada |
| 2 | Cripta do Rei Branco |
| 3 | Torre dos Ventos |
| 4 | Observatório Celestial |
| 5 | Templo Glacial |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 1-2 | 10 | 2 | 5 | 33 |
| 2-3 | 12 | 3 | 4 | 36 |
| 3-4 | 8 | 2 | 3 | 25 |
| 4-5 | 14 | 2 | 4 | 34 |
| 1-5 | 25 | 4 | 6 | 59 |

---

# REGIÃO II — Nocthyr Vale
## (Vale da Floresta Negra)

Uma floresta amaldiçoada tomada por magia negra ancestral, criaturas corrompidas e ruínas esquecidas.

⚠️ HIGH RISK / HIGH REWARD

### Biomas
- Floresta Negra
- Ruínas Amaldiçoadas

### Nodes

| ID | Local |
|---|---|
| 6 | Santuário Sombrio |
| 7 | Ruínas de Vaelor |
| 8 | Caverna dos Sussurros |
| 9 | Torre da Bruxa |
| 10 | Altar de Sangue |
| 11 | Biblioteca Proibida |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 6-7 | 9 | 5 | 3 | 38 |
| 7-8 | 11 | 4 | 2 | 33 |
| 8-9 | 13 | 5 | 3 | 42 |
| 9-10 | 7 | 6 | 2 | 37 |
| 10-11 | 10 | 7 | 3 | 47 |
| 6-11 | 24 | 8 | 4 | 68 |

---

# REGIÃO III — Aurelion
## (Reino Antigo Central)

O coração do antigo império humano. Região relativamente segura, repleta de estradas antigas, templos e arquivos históricos.

### Biomas
- Planícies
- Estradas Reais
- Colinas

### Nodes

| ID | Local |
|---|---|
| 12 | Capital Arruinada |
| 13 | Grande Arquivo |
| 14 | Catacumbas Reais |
| 15 | Arena Antiga |
| 16 | Templo Solar |
| 17 | Universidade Arcana |
| 18 | Mausoléu dos Reis |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 12-13 | 8 | 1 | 1 | 15 |
| 13-14 | 7 | 1 | 1 | 14 |
| 14-15 | 6 | 1 | 1 | 13 |
| 15-16 | 8 | 1 | 1 | 15 |
| 16-17 | 9 | 1 | 1 | 16 |
| 17-18 | 8 | 1 | 1 | 15 |

---

# REGIÃO IV — Morvhal Mire
## (Pântanos dos Mortos)

Um território afundado pela necromancia antiga, coberto por pântanos tóxicos e cemitérios esquecidos.

### Biomas
- Pântano
- Terras Mortas

### Nodes

| ID | Local |
|---|---|
| 19 | Cripta Afundada |
| 20 | Vila Submersa |
| 21 | Torre do Necromante |
| 22 | Pântano do Lamento |
| 23 | Cemitério dos Gigantes |
| 24 | Poço Abissal |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 19-20 | 8 | 4 | 7 | 45 |
| 20-21 | 11 | 5 | 8 | 55 |
| 21-22 | 9 | 6 | 8 | 57 |
| 22-23 | 13 | 5 | 7 | 54 |
| 23-24 | 10 | 7 | 6 | 56 |

---

# REGIÃO V — Solkarath
## (Expansão Escarlate)

Um vasto deserto de ruínas soterradas, ravinas e formações arcanas cristalinas.

### Biomas
- Deserto
- Ravinas
- Cristais Arcanos

### Nodes

| ID | Local |
|---|---|
| 25 | Pirâmide Dourada |
| 26 | Templo do Sol Negro |
| 27 | Ruínas Enterradas |
| 28 | Cidade de Cristal |
| 29 | Ponte dos Condenados |
| 30 | Abismo Escarlate |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 25-26 | 10 | 2 | 5 | 33 |
| 26-27 | 12 | 3 | 5 | 39 |
| 27-28 | 9 | 4 | 4 | 37 |
| 28-29 | 8 | 5 | 6 | 46 |
| 29-30 | 11 | 6 | 7 | 56 |

---

# REGIÃO VI — Drakmord
## (Terras Dracônicas)

Uma região devastada por antigas guerras dracônicas, rios de lava e fortalezas destruídas.

⚠️ HIGH RISK / HIGH REWARD

### Biomas
- Lava
- Campos de Guerra
- Montanhas Vulcânicas

### Nodes

| ID | Local |
|---|---|
| 31 | Fortaleza Queimada |
| 32 | Covil do Dragão |
| 33 | Campo da Última Guerra |
| 34 | Torre de Cinzas |
| 35 | Forja Titânica |
| 36 | Portal Antigo |

---

## Conexões Internas

| Edge | D | P | T | Custo |
|---|---|---|---|---|
| 31-32 | 12 | 8 | 7 | 65 |
| 32-33 | 8 | 7 | 6 | 54 |
| 33-34 | 10 | 8 | 6 | 60 |
| 34-35 | 7 | 7 | 5 | 50 |
| 35-36 | 9 | 9 | 6 | 69 |

---

# GARGALOS PRINCIPAIS

---

# Gargalo I — Passagem do Corvo

Conecta:
- Eryndorheim ↔ Aurelion

### Edge
3 ↔ 12

### Bioma
- Ravina Congelada

| D | P | T | Custo |
|---|---|---|---|
| 25 | 4 | 6 | 59 |

---

# Gargalo II — Ponte Negra

Conecta:
- Nocthyr Vale ↔ Aurelion

### Edge
9 ↔ 15

### Bioma
- Floresta Negra

| D | P | T | Custo |
|---|---|---|---|
| 18 | 6 | 4 | 54 |

---

# Gargalo III — Desfiladeiro Escarlate

Conecta:
- Solkarath ↔ Drakmord

### Edge
30 ↔ 31

### Bioma
- Lava / Ravina Vulcânica

| D | P | T | Custo |
|---|---|---|---|
| 20 | 8 | 8 | 76 |

---

# CONEXÕES ENTRE REGIÕES

| Edge | Regiões | Custo |
|---|---|---|
| 3-12 | Eryndorheim ↔ Aurelion | 59 |
| 9-15 | Nocthyr Vale ↔ Aurelion | 54 |
| 18-19 | Aurelion ↔ Morvhal Mire | 48 |
| 17-25 | Aurelion ↔ Solkarath | 45 |
| 30-31 | Solkarath ↔ Drakmord | 76 |
| 24-33 | Morvhal Mire ↔ Drakmord | 72 |

---

# Estrutura Estratégica do Mundo

O mapa foi projetado para gerar:

- Clusters regionais
- Gargalos naturais
- Rotas seguras e rotas perigosas
- Regiões de alto risco e alta recompensa
- Ótimos locais para Hill Climbing
- Exploração eficiente para Simulated Annealing
- Estratégias globais para Algoritmos Genéticos

---

# Comportamento Esperado dos Algoritmos

## Hill Climbing
- Forte otimização local
- Tendência a ficar preso em clusters

## Simulated Annealing
- Exploração mais agressiva
- Maior capacidade de escapar de mínimos locais

## Algoritmo Genético
- Melhor equilíbrio global
- Rotas mais refinadas
- Melhor adaptação a regiões extremas

---