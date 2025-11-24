#pragma once
#include <SFML/Graphics.hpp>

enum TokenState { HOME, ACTIVE, FINISHED };

class Token {
private:
    sf::CircleShape sprite;
    int playerID;
    int stepsTaken; // -1 means home, 0 is start tile, 51+ is home run
    TokenState state;

public:
    Token(int playerID, sf::Color color);
    sf::Vector2f getPosition() const; // Add this
    void setPosition(float x, float y);
    void setPosition(sf::Vector2f pos);
    void draw(sf::RenderWindow& window);
    
    bool isClicked(sf::Vector2i mousePos);
    
    // Logic
    int getSteps() const { return stepsTaken; }
    TokenState getState() const { return state; }
    
    // Returns true if move was valid/successful
    bool move(int diceValue); 
    void resetToHome();
};