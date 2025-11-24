#pragma once
#include <string>
#include <vector>
#include "Token.h"

class Player {
private:
    std::string name;
    sf::Color color;
public:
    std::vector<Token> tokens;
    Player(std::string name, sf::Color color, int id);
    void drawTokens(sf::RenderWindow& window);
    std::string getName() const;
};