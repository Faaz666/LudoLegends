#include "../include/Player.h"

Player::Player(std::string name, sf::Color color, int id) {
    this->name = name;
    this->color = color;
    
    // Create 4 tokens for this player
    // In a real game, we place them in the "Home" box coordinates
    // For Demo: Placing them randomly near corners to show they exist
    float startX = (id % 2 == 0) ? 50 : 450; 
    float startY = (id < 2) ? 50 : 450;

    for(int i=0; i<4; i++) {
        Token t(id, color);
        t.setPosition(startX + (i*35), startY); // Visual offset
        tokens.push_back(t);
    }
}

void Player::drawTokens(sf::RenderWindow& window) {
    for(auto& token : tokens) {
        token.draw(window);
    }
}

std::string Player::getName() const { return name; }