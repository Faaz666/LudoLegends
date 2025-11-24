#pragma once
#include <vector>
#include "Tile.h"
#include <SFML/Graphics.hpp>

class Board {
private:
    std::vector<Tile*> tiles; // Pointers for Polymorphism
public:
    Board();
    ~Board(); // Destructor to clean up pointers
    void initializeBoard(); // procedural generation of the grid
    sf::Vector2f getTilePosition(int stepIndex, int playerID);
    void draw(sf::RenderWindow& window);
};