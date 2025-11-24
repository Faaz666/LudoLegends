#include "../include/Game.h"
#include <iostream>

Game::Game() : window(sf::VideoMode(800, 650), "Ludo Legends"), state(WELCOME) {
    window.setFramerateLimit(60);
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {} // Error check omitted

    // Setup Text
    welcomeText.setFont(font); welcomeText.setString("LUDO LEGENDS\n\nPress ENTER");
    welcomeText.setCharacterSize(50); welcomeText.setPosition(200, 250);
    
    turnText.setFont(font); turnText.setCharacterSize(24); 
    turnText.setPosition(630, 50);

    infoText.setFont(font); infoText.setCharacterSize(18);
    infoText.setFillColor(sf::Color::Cyan);
    infoText.setPosition(630, 450);

    consecutiveSixes = 0;
    currentPlayerIndex = 0;
    isDiceRolled = false;
}

bool Game::isSafePosition(sf::Vector2f pos) {
    // Basic coordinate checks for the stars/safe spots we defined in Board
    // A simplified check: The Start positions are always safe.
    // (This logic ideally calls Board methods, but for now we approximate using the Grid logic)
    // If you implemented SafeTile correctly in Board.cpp, visuals handle it.
    // For Logic, we assume collision is valid unless it's a specific "Star" index.
    
    // NOTE: For this project scope, let's allow capturing EVERYWHERE except Home Columns.
    return false; // Implementing full safe tile check requires mapping pixels back to grid.
}

void Game::run() {
    // Initialize Players
    players.push_back(Player("Red", sf::Color::Red, 0));
    players.push_back(Player("Green", sf::Color::Green, 1));
    players.push_back(Player("Blue", sf::Color::Blue, 2));
    players.push_back(Player("Yellow", sf::Color::Yellow, 3));

    turnText.setString("Turn: " + players[currentPlayerIndex].getName());

    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        if (state == WELCOME) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) state = PLAYING;
        }
        else if (state == PLAYING) {
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                Player& currPlayer = players[currentPlayerIndex];

                // --- DICE LOGIC ---
                if (!isDiceRolled && dice.isClicked(mousePos)) {
                    dice.roll();
                    rolledValue = dice.getValue();
                    isDiceRolled = true;
                    infoText.setString(""); // Clear messages

                    // Rule: 3 Sixes penalty
                    if (rolledValue == 6) consecutiveSixes++;
                    else consecutiveSixes = 0;

                    if (consecutiveSixes == 3) {
                        infoText.setString("3 Sixes! Turn Skipped.");
                        consecutiveSixes = 0;
                        isDiceRolled = false;
                        nextTurn();
                        return;
                    }

                    // Check if any move is possible
                    bool canMove = false;
                    for(auto& t : currPlayer.tokens) {
                        if (t.getState() == ACTIVE) canMove = true;
                        if (t.getState() == HOME && rolledValue == 6) canMove = true;
                    }
                    if (!canMove) {
                        infoText.setString("No moves possible.");
                        isDiceRolled = false;
                        nextTurn();
                    }
                }
                
                // --- TOKEN MOVEMENT LOGIC ---
                else if (isDiceRolled) {
                    for(auto& token : currPlayer.tokens) {
                        if(token.isClicked(mousePos)) {
                            // Store old pos to revert if needed (optional)
                            // Try Move
                            if(token.move(rolledValue)) {
                                
                                // 1. Update Visuals
                                sf::Vector2f newPos = board.getTilePosition(token.getSteps(), currentPlayerIndex);
                                token.setPosition(newPos);
                                
                                // 2. COLLISION / CAPTURE LOGIC
                                bool captured = false;
                                // Only check capture if we are NOT in Home/Finish stretch (steps < 51)
                                if (token.getSteps() < 51) {
                                    for(int pIdx = 0; pIdx < players.size(); pIdx++) {
                                        if (pIdx == currentPlayerIndex) continue; // Don't capture self

                                        for(auto& enemyToken : players[pIdx].tokens) {
                                            // Crude collision check: Are they at exact same pixel?
                                            if (enemyToken.getState() == ACTIVE && 
                                                abs(enemyToken.getPosition().x - newPos.x) < 5 && 
                                                abs(enemyToken.getPosition().y - newPos.y) < 5) 
                                            {
                                                // CAPTURE!
                                                // NOTE: Add isSafe check here if you want strict Safe Tiles
                                                enemyToken.resetToHome();
                                                
                                                // Reset enemy visual
                                                // We rely on next render loop, but position needs reset
                                                // Simple hack: Move it off screen or let render handle it
                                                // Ideally Player class should have a "resetVisuals"
                                                // For now, let's just place it back to a corner manually:
                                                enemyToken.setPosition((pIdx%2)*400 + 50, (pIdx/2)*400 + 50); 
                                                
                                                captured = true;
                                                infoText.setString("CAPTURED! Bonus Roll!");
                                            }
                                        }
                                    }
                                }

                                // 3. Turn Management
                                isDiceRolled = false;
                                checkWin();
                                
                                // Rule: Roll 6 OR Capture = Go again
                                if (rolledValue == 6 || captured) {
                                    // Same player keeps turn
                                } else {
                                    consecutiveSixes = 0;
                                    nextTurn();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Game::checkWin() {
    Player& p = players[currentPlayerIndex];
    int finishedCount = 0;
    for(auto& t : p.tokens) {
        if(t.getState() == FINISHED) finishedCount++;
    }
    
    if(finishedCount == 4) {
        state = GAME_OVER;
        welcomeText.setString(p.getName() + " WINS!");
        welcomeText.setPosition(300, 300);
        welcomeText.setFillColor(sf::Color::Green);
    }
}


void Game::update() {
    // Game logic updates go here
}

void Game::render() {
    window.clear(sf::Color(30, 30, 30));

    if (state == WELCOME || state == GAME_OVER) {
        window.draw(welcomeText);
    }
    else if (state == PLAYING) {
        board.draw(window);
        dice.draw(window);
        window.draw(turnText);
        window.draw(infoText);

        for(auto& p : players) p.drawTokens(window);
    }
    window.display();
}

void Game::nextTurn() {
    currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    turnText.setString("Turn: " + players[currentPlayerIndex].getName());
    
    // Color code the turn text
    sf::Color c = sf::Color::White;
    if (players[currentPlayerIndex].getName() == "Red") c = sf::Color::Red;
    if (players[currentPlayerIndex].getName() == "Green") c = sf::Color::Green;
    if (players[currentPlayerIndex].getName() == "Blue") c = sf::Color::Blue;
    if (players[currentPlayerIndex].getName() == "Yellow") c = sf::Color::Yellow;
    turnText.setFillColor(c);
}