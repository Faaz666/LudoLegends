#include "../include/Token.h"
#include <iostream>

Token::Token(int playerID, sf::Color color) {
    this->playerID = playerID;
    sprite.setRadius(15);
    sprite.setFillColor(color);
    sprite.setOutlineThickness(2);
    sprite.setOutlineColor(sf::Color::Black);
    
    state = HOME;
    stepsTaken = -1; 
}

void Token::setPosition(float x, float y) { sprite.setPosition(x, y); }
void Token::setPosition(sf::Vector2f pos) { sprite.setPosition(pos); }
sf::Vector2f Token::getPosition() const { return sprite.getPosition(); }

void Token::draw(sf::RenderWindow& window) { window.draw(sprite); }

bool Token::isClicked(sf::Vector2i mousePos) {
    return sprite.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}

void Token::resetToHome() {
    state = HOME;
    stepsTaken = -1;
}

bool Token::move(int diceValue) {
    // RULE: If at Home, need a 6 to start
    if (state == HOME) {
        if (diceValue == 6) {
            state = ACTIVE;
            stepsTaken = 0; // Move to start tile
            return true;
        }
        return false; // Can't move
    }
    
    // RULE: If Active, move forward
    if (state == ACTIVE) {
        if (stepsTaken + diceValue <= 56) { // 56 is the center goal
            stepsTaken += diceValue;
            if (stepsTaken == 56) state = FINISHED;
            return true;
        }
    }
    return false;
}