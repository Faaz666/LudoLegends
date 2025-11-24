#include "../include/Dice.h"
#include <string>

Dice::Dice() {
    srand(time(0));
    currentValue = 1;

    // Better positioning (Right side, centered vertically)
    box.setSize(sf::Vector2f(100, 100));
    box.setPosition(640, 275); 
    box.setFillColor(sf::Color::White);
    box.setOutlineThickness(4);
    box.setOutlineColor(sf::Color::Black);

    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        // Handle error
    }
    
    numberText.setFont(font);
    numberText.setCharacterSize(60);
    numberText.setFillColor(sf::Color::Black);
    numberText.setString("1");
    
    // Center text logic
    sf::FloatRect textRect = numberText.getLocalBounds();
    numberText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    numberText.setPosition(box.getPosition().x + box.getSize().x/2.0f, box.getPosition().y + box.getSize().y/2.0f);
}

void Dice::roll() {
    currentValue = (rand() % 6) + 1;
    numberText.setString(std::to_string(currentValue));
    
    // Re-center text every roll (because "1" is thinner than "6")
    sf::FloatRect textRect = numberText.getLocalBounds();
    numberText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    numberText.setPosition(box.getPosition().x + box.getSize().x/2.0f, box.getPosition().y + box.getSize().y/2.0f);
}

int Dice::getValue() const { return currentValue; }

void Dice::draw(sf::RenderWindow& window) {
    window.draw(box);
    window.draw(numberText);
}

bool Dice::isClicked(sf::Vector2i mousePos) {
    return box.getGlobalBounds().contains(static_cast<sf::Vector2f>(mousePos));
}