#include "core/WorldMap.h"

// Map IDs are 1-based; internally nodes are 0-based (ID - 1).
// All edges are bidirectional.
// New node indices per region:
//   I: 0-7   II: 8-15   III: 16-25   IV: 26-33   V: 34-41   VI: 42-49

Graph buildWorldMap() {
    Graph g(50);

    // --- REGION I: Eryndorheim (IDs 1-8, indices 0-7) ---
    // Biomes: Mountains, Snow, Ravines — P: 2-4, T: 3-6
    g.setNode(0, "Frozen Fortress");
    g.setNode(1, "Crypt of the White King");
    g.setNode(2, "Tower of the Winds");
    g.setNode(3, "Celestial Observatory");
    g.setNode(4, "Glacial Temple");
    g.setNode(5, "Frozen Waterfalls");      // new — snow ravine between peaks
    g.setNode(6, "Tomb of the Ice Giants"); // new — buried giant remains under glacier
    g.setNode(7, "Ruined Watchtower");      // new — old military post on cliff edge

    g.addEdge(0, 1, 10, 2, 5); g.addEdge(1, 0, 10, 2, 5); // cost 33
    g.addEdge(1, 2, 12, 3, 4); g.addEdge(2, 1, 12, 3, 4); // cost 36
    g.addEdge(2, 3,  8, 2, 3); g.addEdge(3, 2,  8, 2, 3); // cost 25
    g.addEdge(3, 4, 14, 2, 4); g.addEdge(4, 3, 14, 2, 4); // cost 34
    g.addEdge(0, 4, 25, 4, 6); g.addEdge(4, 0, 25, 4, 6); // cost 59 (shortcut)
    g.addEdge(4, 5, 10, 3, 5); g.addEdge(5, 4, 10, 3, 5); // cost 37
    g.addEdge(5, 6,  8, 3, 4); g.addEdge(6, 5,  8, 3, 4); // cost 32
    g.addEdge(6, 7, 12, 4, 6); g.addEdge(7, 6, 12, 4, 6); // cost 46
    g.addEdge(7, 2,  9, 3, 5); g.addEdge(2, 7,  9, 3, 5); // cost 36 (loop back to Tower of Winds)

    // --- REGION II: Nocthyr Vale (IDs 9-16, indices 8-15) ---
    // Biomes: Black Forest, Cursed Ruins — P: 4-8, T: 2-4
    g.setNode( 8, "Shadow Sanctuary");
    g.setNode( 9, "Ruins of Vaelor");
    g.setNode(10, "Cave of Whispers");
    g.setNode(11, "Witch's Tower");
    g.setNode(12, "Blood Altar");
    g.setNode(13, "Forbidden Library");
    g.setNode(14, "Dark Pilgrim's Shrine"); // new — corrupted holy place in the deep forest
    g.setNode(15, "Moonless Hollow");       // new — perpetually dark valley, no light reaches

    g.addEdge( 8,  9,  9, 5, 3); g.addEdge( 9,  8,  9, 5, 3); // cost 38
    g.addEdge( 9, 10, 11, 4, 2); g.addEdge(10,  9, 11, 4, 2); // cost 33
    g.addEdge(10, 11, 13, 5, 3); g.addEdge(11, 10, 13, 5, 3); // cost 42
    g.addEdge(11, 12,  7, 6, 2); g.addEdge(12, 11,  7, 6, 2); // cost 37
    g.addEdge(12, 13, 10, 7, 3); g.addEdge(13, 12, 10, 7, 3); // cost 47
    g.addEdge( 8, 13, 24, 8, 4); g.addEdge(13,  8, 24, 8, 4); // cost 68 (shortcut)
    g.addEdge(13, 14, 12, 5, 3); g.addEdge(14, 13, 12, 5, 3); // cost 41
    g.addEdge(14, 15,  9, 6, 4); g.addEdge(15, 14,  9, 6, 4); // cost 45
    g.addEdge(15, 10, 11, 5, 4); g.addEdge(10, 15, 11, 5, 4); // cost 43 (loop back to Cave of Whispers)

    // --- REGION III: Aurelion (IDs 17-26, indices 16-25) ---
    // Biomes: Plains, Royal Roads, Hills — P: 1-2, T: 1-2
    g.setNode(16, "Ruined Capital");
    g.setNode(17, "Great Archive");
    g.setNode(18, "Royal Catacombs");
    g.setNode(19, "Ancient Arena");
    g.setNode(20, "Solar Temple");
    g.setNode(21, "Arcane University");
    g.setNode(22, "Mausoleum of Kings");
    g.setNode(23, "Hall of Records");       // new — imperial administrative building
    g.setNode(24, "Old Merchant Quarter");  // new — former trade hub of the empire
    g.setNode(25, "Palace of the Fallen");  // new — collapsed royal palace

    g.addEdge(16, 17,  8, 1, 1); g.addEdge(17, 16,  8, 1, 1); // cost 15
    g.addEdge(17, 18,  7, 1, 1); g.addEdge(18, 17,  7, 1, 1); // cost 14
    g.addEdge(18, 19,  6, 1, 1); g.addEdge(19, 18,  6, 1, 1); // cost 13
    g.addEdge(19, 20,  8, 1, 1); g.addEdge(20, 19,  8, 1, 1); // cost 15
    g.addEdge(20, 21,  9, 1, 1); g.addEdge(21, 20,  9, 1, 1); // cost 16
    g.addEdge(21, 22,  8, 1, 1); g.addEdge(22, 21,  8, 1, 1); // cost 15
    g.addEdge(22, 23,  7, 1, 1); g.addEdge(23, 22,  7, 1, 1); // cost 14
    g.addEdge(23, 24,  6, 1, 1); g.addEdge(24, 23,  6, 1, 1); // cost 13
    g.addEdge(24, 25,  9, 2, 1); g.addEdge(25, 24,  9, 2, 1); // cost 20
    g.addEdge(25, 16,  5, 1, 1); g.addEdge(16, 25,  5, 1, 1); // cost 12 (loop back to Ruined Capital)

    // --- REGION IV: Morvhal Mire (IDs 27-34, indices 26-33) ---
    // Biomes: Swamp, Dead Lands — P: 4-7, T: 5-8
    g.setNode(26, "Sunken Crypt");
    g.setNode(27, "Submerged Village");
    g.setNode(28, "Necromancer's Tower");
    g.setNode(29, "Swamp of Lament");
    g.setNode(30, "Giants' Cemetery");
    g.setNode(31, "Abyssal Well");
    g.setNode(32, "Plague Pit");    // new — mass grave from an ancient epidemic
    g.setNode(33, "Bone Bridge");   // new — crossing built from remains of the fallen

    g.addEdge(26, 27,  8, 4, 7); g.addEdge(27, 26,  8, 4, 7); // cost 45
    g.addEdge(27, 28, 11, 5, 8); g.addEdge(28, 27, 11, 5, 8); // cost 55
    g.addEdge(28, 29,  9, 6, 8); g.addEdge(29, 28,  9, 6, 8); // cost 57
    g.addEdge(29, 30, 13, 5, 7); g.addEdge(30, 29, 13, 5, 7); // cost 54
    g.addEdge(30, 31, 10, 7, 6); g.addEdge(31, 30, 10, 7, 6); // cost 56
    g.addEdge(31, 32,  8, 6, 7); g.addEdge(32, 31,  8, 6, 7); // cost 53
    g.addEdge(32, 33, 11, 5, 6); g.addEdge(33, 32, 11, 5, 6); // cost 49
    g.addEdge(33, 27,  9, 4, 7); g.addEdge(27, 33,  9, 4, 7); // cost 46 (loop back to Submerged Village)

    // --- REGION V: Solkarath (IDs 35-42, indices 34-41) ---
    // Biomes: Desert, Ravines, Arcane Crystals — P: 2-6, T: 4-7
    g.setNode(34, "Golden Pyramid");
    g.setNode(35, "Temple of the Black Sun");
    g.setNode(36, "Buried Ruins");
    g.setNode(37, "Crystal City");
    g.setNode(38, "Bridge of the Condemned");
    g.setNode(39, "Scarlet Abyss");
    g.setNode(40, "Oasis of Mirrors");  // new — deceptive magical oasis deep in the desert
    g.setNode(41, "Sand Fortress");     // new — ancient military outpost half-buried by dunes

    g.addEdge(34, 35, 10, 2, 5); g.addEdge(35, 34, 10, 2, 5); // cost 33
    g.addEdge(35, 36, 12, 3, 5); g.addEdge(36, 35, 12, 3, 5); // cost 39
    g.addEdge(36, 37,  9, 4, 4); g.addEdge(37, 36,  9, 4, 4); // cost 37
    g.addEdge(37, 38,  8, 5, 6); g.addEdge(38, 37,  8, 5, 6); // cost 46
    g.addEdge(38, 39, 11, 6, 7); g.addEdge(39, 38, 11, 6, 7); // cost 56
    g.addEdge(39, 40, 10, 3, 4); g.addEdge(40, 39, 10, 3, 4); // cost 34
    g.addEdge(40, 41,  8, 2, 5); g.addEdge(41, 40,  8, 2, 5); // cost 31
    g.addEdge(41, 35, 12, 3, 5); g.addEdge(35, 41, 12, 3, 5); // cost 39 (loop back to Temple of the Black Sun)

    // --- REGION VI: Drakmord (IDs 43-50, indices 42-49) ---
    // Biomes: Lava, War Fields, Volcanic Mountains — P: 7-9, T: 6-8
    g.setNode(42, "Burned Fortress");
    g.setNode(43, "Dragon's Lair");
    g.setNode(44, "Last War Battlefield");
    g.setNode(45, "Tower of Ashes");
    g.setNode(46, "Titanic Forge");
    g.setNode(47, "Ancient Portal");
    g.setNode(48, "Lava Shrine");   // new — ancient dragon cult worship site
    g.setNode(49, "Molten Crater"); // new — active volcanic crater, extreme heat and gas

    g.addEdge(42, 43, 12, 8, 7); g.addEdge(43, 42, 12, 8, 7); // cost 65
    g.addEdge(43, 44,  8, 7, 6); g.addEdge(44, 43,  8, 7, 6); // cost 54
    g.addEdge(44, 45, 10, 8, 6); g.addEdge(45, 44, 10, 8, 6); // cost 60
    g.addEdge(45, 46,  7, 7, 5); g.addEdge(46, 45,  7, 7, 5); // cost 50
    g.addEdge(46, 47,  9, 9, 6); g.addEdge(47, 46,  9, 9, 6); // cost 69
    g.addEdge(47, 48, 10, 8, 7); g.addEdge(48, 47, 10, 8, 7); // cost 63
    g.addEdge(48, 49,  8, 9, 8); g.addEdge(49, 48,  8, 9, 8); // cost 68
    g.addEdge(49, 43, 11, 8, 7); g.addEdge(43, 49, 11, 8, 7); // cost 64 (loop back to Dragon's Lair)

    // --- INTER-REGION CONNECTIONS ---

    // Bottleneck I — Raven's Pass: Tower of the Winds (2) ↔ Ruined Capital (16)
    g.addEdge( 2, 16, 25, 4, 6); g.addEdge(16,  2, 25, 4, 6); // cost 59

    // Bottleneck II — Black Bridge: Witch's Tower (11) ↔ Ancient Arena (19)
    g.addEdge(11, 19, 18, 6, 4); g.addEdge(19, 11, 18, 6, 4); // cost 54

    // Aurelion ↔ Morvhal Mire: Mausoleum of Kings (22) ↔ Sunken Crypt (26)
    g.addEdge(22, 26, 17, 4, 5); g.addEdge(26, 22, 17, 4, 5); // cost 48

    // Aurelion ↔ Solkarath: Arcane University (21) ↔ Golden Pyramid (34)
    g.addEdge(21, 34, 22, 2, 5); g.addEdge(34, 21, 22, 2, 5); // cost 45

    // Bottleneck III — Scarlet Gorge: Scarlet Abyss (39) ↔ Burned Fortress (42)
    g.addEdge(39, 42, 20, 8, 8); g.addEdge(42, 39, 20, 8, 8); // cost 76

    // Morvhal Mire ↔ Drakmord: Abyssal Well (31) ↔ Last War Battlefield (44)
    g.addEdge(31, 44, 22, 8, 6); g.addEdge(44, 31, 22, 8, 6); // cost 72

    return g;
}
