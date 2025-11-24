#pragma once
#include <SFML/Graphics.hpp>
#include "Board.h"
#include "Dice.h"
#include "Player.h"

enum GameState { WELCOME, PLAYING, GAME_OVER };

class Game {
private:
    sf::RenderWindow window;
    GameState state;
    int rolledValue;
    bool isDiceRolled;
    int consecutiveSixes; // Rule: 3 sixes = skip
    sf::Text infoText;    // To display messages like "Captured!"
    // Game Objects
    Board board;
    Dice dice;
    std::vector<Player> players;
    int currentPlayerIndex;

    // UI Elements
    sf::Font font;
    sf::Text welcomeText;
    sf::Text turnText;

public:
    void checkWin();
    bool isSafePosition(sf::Vector2f pos);
    Game();
    void run();

private:
    void processEvents();
    void update();
    void render();
    void nextTurn();
};