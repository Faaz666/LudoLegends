#pragma once
#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <ctime>

class Dice {
private:
    sf::RectangleShape box;
    sf::Text numberText;
    sf::Font font;
    int currentValue;
public:
    Dice();
    void roll();
    int getValue() const;
    void draw(sf::RenderWindow& window);
    bool isClicked(sf::Vector2i mousePos);
};