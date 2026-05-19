#include "core/WorldMap.h"

// Map IDs in map.md are 1-based; internally nodes are 0-based (ID - 1).
// All edges are bidirectional.
// Inter-region connections without explicit D/P/T have values derived from cost formula: C = D + 4P + 3T

Graph buildWorldMap() {
    Graph g(36);

    // --- REGION I: Eryndorheim (IDs 1-5 → indices 0-4) ---
    g.setNode(0, "Fortaleza Congelada");
    g.setNode(1, "Cripta do Rei Branco");
    g.setNode(2, "Torre dos Ventos");
    g.setNode(3, "Observatorio Celestial");
    g.setNode(4, "Templo Glacial");

    g.addEdge(0, 1, 10, 2, 5); g.addEdge(1, 0, 10, 2, 5); // 1-2: cost 33
    g.addEdge(1, 2, 12, 3, 4); g.addEdge(2, 1, 12, 3, 4); // 2-3: cost 36
    g.addEdge(2, 3,  8, 2, 3); g.addEdge(3, 2,  8, 2, 3); // 3-4: cost 25
    g.addEdge(3, 4, 14, 2, 4); g.addEdge(4, 3, 14, 2, 4); // 4-5: cost 34
    g.addEdge(0, 4, 25, 4, 6); g.addEdge(4, 0, 25, 4, 6); // 1-5: cost 59

    // --- REGION II: Nocthyr Vale (IDs 6-11 → indices 5-10) ---
    g.setNode(5,  "Santuario Sombrio");
    g.setNode(6,  "Ruinas de Vaelor");
    g.setNode(7,  "Caverna dos Sussurros");
    g.setNode(8,  "Torre da Bruxa");
    g.setNode(9,  "Altar de Sangue");
    g.setNode(10, "Biblioteca Proibida");

    g.addEdge(5,  6,  9, 5, 3); g.addEdge(6,  5,  9, 5, 3); // 6-7:  cost 38
    g.addEdge(6,  7, 11, 4, 2); g.addEdge(7,  6, 11, 4, 2); // 7-8:  cost 33
    g.addEdge(7,  8, 13, 5, 3); g.addEdge(8,  7, 13, 5, 3); // 8-9:  cost 42
    g.addEdge(8,  9,  7, 6, 2); g.addEdge(9,  8,  7, 6, 2); // 9-10: cost 37
    g.addEdge(9, 10, 10, 7, 3); g.addEdge(10, 9, 10, 7, 3); // 10-11: cost 47
    g.addEdge(5, 10, 24, 8, 4); g.addEdge(10, 5, 24, 8, 4); // 6-11: cost 68

    // --- REGION III: Aurelion (IDs 12-18 → indices 11-17) ---
    g.setNode(11, "Capital Arruinada");
    g.setNode(12, "Grande Arquivo");
    g.setNode(13, "Catacumbas Reais");
    g.setNode(14, "Arena Antiga");
    g.setNode(15, "Templo Solar");
    g.setNode(16, "Universidade Arcana");
    g.setNode(17, "Mausoleu dos Reis");

    g.addEdge(11, 12, 8, 1, 1); g.addEdge(12, 11, 8, 1, 1); // 12-13: cost 15
    g.addEdge(12, 13, 7, 1, 1); g.addEdge(13, 12, 7, 1, 1); // 13-14: cost 14
    g.addEdge(13, 14, 6, 1, 1); g.addEdge(14, 13, 6, 1, 1); // 14-15: cost 13
    g.addEdge(14, 15, 8, 1, 1); g.addEdge(15, 14, 8, 1, 1); // 15-16: cost 15
    g.addEdge(15, 16, 9, 1, 1); g.addEdge(16, 15, 9, 1, 1); // 16-17: cost 16
    g.addEdge(16, 17, 8, 1, 1); g.addEdge(17, 16, 8, 1, 1); // 17-18: cost 15

    // --- REGION IV: Morvhal Mire (IDs 19-24 → indices 18-23) ---
    g.setNode(18, "Cripta Afundada");
    g.setNode(19, "Vila Submersa");
    g.setNode(20, "Torre do Necromante");
    g.setNode(21, "Pantano do Lamento");
    g.setNode(22, "Cemiterio dos Gigantes");
    g.setNode(23, "Poco Abissal");

    g.addEdge(18, 19,  8, 4, 7); g.addEdge(19, 18,  8, 4, 7); // 19-20: cost 45
    g.addEdge(19, 20, 11, 5, 8); g.addEdge(20, 19, 11, 5, 8); // 20-21: cost 55
    g.addEdge(20, 21,  9, 6, 8); g.addEdge(21, 20,  9, 6, 8); // 21-22: cost 57
    g.addEdge(21, 22, 13, 5, 7); g.addEdge(22, 21, 13, 5, 7); // 22-23: cost 54
    g.addEdge(22, 23, 10, 7, 6); g.addEdge(23, 22, 10, 7, 6); // 23-24: cost 56

    // --- REGION V: Solkarath (IDs 25-30 → indices 24-29) ---
    g.setNode(24, "Piramide Dourada");
    g.setNode(25, "Templo do Sol Negro");
    g.setNode(26, "Ruinas Enterradas");
    g.setNode(27, "Cidade de Cristal");
    g.setNode(28, "Ponte dos Condenados");
    g.setNode(29, "Abismo Escarlate");

    g.addEdge(24, 25, 10, 2, 5); g.addEdge(25, 24, 10, 2, 5); // 25-26: cost 33
    g.addEdge(25, 26, 12, 3, 5); g.addEdge(26, 25, 12, 3, 5); // 26-27: cost 39
    g.addEdge(26, 27,  9, 4, 4); g.addEdge(27, 26,  9, 4, 4); // 27-28: cost 37
    g.addEdge(27, 28,  8, 5, 6); g.addEdge(28, 27,  8, 5, 6); // 28-29: cost 46
    g.addEdge(28, 29, 11, 6, 7); g.addEdge(29, 28, 11, 6, 7); // 29-30: cost 56

    // --- REGION VI: Drakmord (IDs 31-36 → indices 30-35) ---
    g.setNode(30, "Fortaleza Queimada");
    g.setNode(31, "Covil do Dragao");
    g.setNode(32, "Campo da Ultima Guerra");
    g.setNode(33, "Torre de Cinzas");
    g.setNode(34, "Forja Titanica");
    g.setNode(35, "Portal Antigo");

    g.addEdge(30, 31, 12, 8, 7); g.addEdge(31, 30, 12, 8, 7); // 31-32: cost 65
    g.addEdge(31, 32,  8, 7, 6); g.addEdge(32, 31,  8, 7, 6); // 32-33: cost 54
    g.addEdge(32, 33, 10, 8, 6); g.addEdge(33, 32, 10, 8, 6); // 33-34: cost 60
    g.addEdge(33, 34,  7, 7, 5); g.addEdge(34, 33,  7, 7, 5); // 34-35: cost 50
    g.addEdge(34, 35,  9, 9, 6); g.addEdge(35, 34,  9, 9, 6); // 35-36: cost 69

    // --- INTER-REGION CONNECTIONS ---

    // Gargalo I — Passagem do Corvo: 3 ↔ 12 (indices 2 ↔ 11)
    g.addEdge(2, 11, 25, 4, 6); g.addEdge(11, 2, 25, 4, 6); // cost 59

    // Gargalo II — Ponte Negra: 9 ↔ 15 (indices 8 ↔ 14)
    g.addEdge(8, 14, 18, 6, 4); g.addEdge(14, 8, 18, 6, 4); // cost 54

    // 18 ↔ 19 (indices 17 ↔ 18): Aurelion ↔ Morvhal Mire — D=17,P=4,T=5
    g.addEdge(17, 18, 17, 4, 5); g.addEdge(18, 17, 17, 4, 5); // cost 48

    // 17 ↔ 25 (indices 16 ↔ 24): Aurelion ↔ Solkarath — D=22,P=2,T=5
    g.addEdge(16, 24, 22, 2, 5); g.addEdge(24, 16, 22, 2, 5); // cost 45

    // Gargalo III — Desfiladeiro Escarlate: 30 ↔ 31 (indices 29 ↔ 30)
    g.addEdge(29, 30, 20, 8, 8); g.addEdge(30, 29, 20, 8, 8); // cost 76

    // 24 ↔ 33 (indices 23 ↔ 32): Morvhal Mire ↔ Drakmord — D=22,P=8,T=6
    g.addEdge(23, 32, 22, 8, 6); g.addEdge(32, 23, 22, 8, 6); // cost 72

    return g;
}
