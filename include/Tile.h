#pragma once
#include <SFML/Graphics.hpp>

// Base class for all Board Tiles
class Tile {
protected:
    sf::RectangleShape shape;
    bool isSafe;
public:
    Tile(float x, float y, float size, sf::Color color);
    virtual void draw(sf::RenderWindow& window); // Virtual function for Polymorphism
    virtual bool isSafeTile() const;
    sf::Vector2f getPosition() const;
};

// Derived Class: SafeTile (Star tiles)
class SafeTile : public Tile {
public:
    SafeTile(float x, float y, float size, sf::Color color);
    void draw(sf::RenderWindow& window) override; // Overriding behavior
    bool isSafeTile() const override;
};