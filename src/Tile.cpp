#include "../include/Tile.h"

Tile::Tile(float x, float y, float size, sf::Color color) {
    shape.setSize(sf::Vector2f(size, size));
    shape.setPosition(x, y);
    shape.setFillColor(color);
    shape.setOutlineThickness(1);
    shape.setOutlineColor(sf::Color::Black);
    isSafe = false;
}

void Tile::draw(sf::RenderWindow& window) {
    window.draw(shape);
}

bool Tile::isSafeTile() const { return isSafe; }

sf::Vector2f Tile::getPosition() const { return shape.getPosition(); }

// SafeTile Implementation
SafeTile::SafeTile(float x, float y, float size, sf::Color color) 
    : Tile(x, y, size, color) {
    isSafe = true;
}

void SafeTile::draw(sf::RenderWindow& window) {
    // Draw base tile
    Tile::draw(window);
    
    // Draw a "Star" or indicator for safe tile
    sf::CircleShape star(shape.getSize().x / 4);
    star.setFillColor(sf::Color::White);
    star.setPosition(shape.getPosition().x + shape.getSize().x/4, 
                     shape.getPosition().y + shape.getSize().y/4);
    window.draw(star);
}
bool SafeTile::isSafeTile() const { return true; }