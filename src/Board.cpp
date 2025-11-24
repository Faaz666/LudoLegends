#include "../include/Board.h"

// Helper to identify specific Grid coordinates that are SAFE
bool isSafeGridPos(int r, int c) {
    // 1. The Starting Squares (Colored ones with arrows)
    if (r == 6 && c == 1) return true;  // Red Start
    if (r == 1 && c == 8) return true;  // Green Start
    if (r == 8 && c == 13) return true; // Blue Start -> Actually Yellow based on your logic
    if (r == 13 && c == 6) return true; // Yellow Start -> Actually Blue based on your logic
    
    // 2. The Star Squares (Standard Ludo locations)
    if (r == 8 && c == 2) return true;
    if (r == 6 && c == 12) return true;
    if (r == 2 && c == 6) return true;
    if (r == 12 && c == 8) return true;
    
    // 3. The Center Grid (Home) is safe
    if (r >= 6 && r <= 8 && c >= 6 && c <= 8) return true;

    return false;
}

Board::Board() { initializeBoard(); }
Board::~Board() { for(Tile* t : tiles) delete t; tiles.clear(); }

void Board::initializeBoard() {
    float cellSize = 40.0f;
    float offsetX = 20.0f;
    float offsetY = 20.0f;

    for (int row = 0; row < 15; ++row) {
        for (int col = 0; col < 15; ++col) {
            float x = offsetX + col * cellSize;
            float y = offsetY + row * cellSize;
            
            sf::Color color = sf::Color::White;

            // -- Logic to color the board like Ludo --
            if (row < 6 && col < 6) color = sf::Color::Red;        // Top Left
            else if (row < 6 && col > 8) color = sf::Color::Green; // Top Right
            else if (row > 8 && col < 6) color = sf::Color::Blue;  // Bottom Left
            else if (row > 8 && col > 8) color = sf::Color::Yellow;// Bottom Right
            
            // Colored Paths (Home Columns)
            else if (row == 7 && col > 0 && col < 6) color = sf::Color::Red;
            else if (col == 7 && row > 0 && row < 6) color = sf::Color::Green;
            else if (row == 7 && col > 8 && col < 14) color = sf::Color::Yellow;
            else if (col == 7 && row > 8 && row < 14) color = sf::Color::Blue;
            
            // Center
            else if (row >= 6 && row <= 8 && col >= 6 && col <= 8) color = sf::Color::Cyan;

            // -- Create Tile --
            if (isSafeGridPos(row, col)) {
                tiles.push_back(new SafeTile(x, y, cellSize, color));
            } else {
                tiles.push_back(new Tile(x, y, cellSize, color));
            }
        }
    }
}

void Board::draw(sf::RenderWindow& window) {
    for(Tile* t : tiles) t->draw(window);
}

sf::Vector2f Board::getTilePosition(int step, int playerID) {
    // 1. Calculate the generic path index (0-51) based on Player Color
    // Red starts at 0, Green at 13, Blue at 26, Yellow at 39
    int offset = playerID * 13;
    
    // The visual grid coordinates (Row, Col) for the 52 main steps
    // Starting from Red's start position and going clockwise
    static const int pathX[] = {
        1,2,3,4,5,  6,6,6,6,6,6,  8,8,8,8,8,8,  9,10,11,12,13,14, // Red side -> Green side
        14,14, 13,12,11,10,9,  8,8,8,8,8,8,  6,6,6,6,6,6,  5,4,3,2,1, // Blue side -> Yellow side
        0,0,0,0,0,0 // Closing the loop
    };
    
    static const int pathY[] = {
        6,6,6,6,6,  5,4,3,2,1,0,  0,1,2,3,4,5,  6,6,6,6,6,6, // Top half
        8,  8,8,8,8,8,  9,10,11,12,13,14, 14,13,12,11,10,9,  8,8,8,8,8,8, // Bottom half
        8,7,6 // Closing
    };

    float cellSize = 40.0f;
    float offsetX = 20.0f;
    float offsetY = 20.0f;

    // LOGIC:
    // If step < 0, it means "At Home" (Base)
    // If step >= 51 (Main track done), enter Home Column
    
    int finalCol = 0;
    int finalRow = 0;

    if (step == -1) {
        // Base Positions (Just generic corners for now)
        if(playerID == 0) { finalCol = 1; finalRow = 1; } // Red
        if(playerID == 1) { finalCol = 12; finalRow = 1; } // Green
        if(playerID == 2) { finalCol = 12; finalRow = 12; } // Blue
        if(playerID == 3) { finalCol = 1; finalRow = 12; } // Yellow
    }
    else if (step < 51) {
        // Walking the main track
        int globalIndex = (offset + step) % 52;
        finalCol = pathX[globalIndex];
        finalRow = pathY[globalIndex];
    }
    else {
        // INSIDE HOME RUN (Victory Road)
        int homeStep = step - 51; // 0 to 5
        if(playerID == 0) { finalRow = 7; finalCol = 1 + homeStep; } // Red goes Right
        if(playerID == 1) { finalCol = 7; finalRow = 1 + homeStep; } // Green goes Down (Typo fix logic if needed) -> Actually Green goes Down from top? 
        // FIX: Green enters from Top (7,0) is wrong. Green enters at (8,0). 
        // Let's use simple logic:
        if(playerID == 0) { finalRow = 7; finalCol = 1 + homeStep; } // Red: Row 7, Col 1->6
        if(playerID == 1) { finalCol = 7; finalRow = 1 + homeStep; } // Green: Col 7, Row 1->6
        if(playerID == 2) { finalRow = 7; finalCol = 13 - homeStep; } // Blue: Row 7, Col 13->8
        if(playerID == 3) { finalCol = 7; finalRow = 13 - homeStep; } // Yellow: Col 7, Row 13->8
    }

    return sf::Vector2f(offsetX + finalCol * cellSize, offsetY + finalRow * cellSize);
}