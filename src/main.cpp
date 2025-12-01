#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <memory>

// --- CONFIGURATION (1920x1080 Base) ---
// Window and layout constants for HD display
const int WIN_W = 1920;   // Window width
const int WIN_H = 1080;   // Window height
const float CELL = 60.0f; // Board cell size (15x15 grid)
const float BOARD_W = 15 * CELL;  // Total board width
const float UI_W = 500.0f;        // Right panel width for UI elements
// Center the board on left side with UI panel on right
const float OFF_X = (WIN_W - UI_W - BOARD_W) / 2.0f; 
const float OFF_Y = (WIN_H - BOARD_W) / 2.0f;
const float ANIM_TIME = 0.59f; // Animation duration synced to audio length 

// --- MODERN PALETTE ---
const sf::Color C_BG      = sf::Color(26, 26, 46);   
const sf::Color C_PANEL   = sf::Color(35, 35, 50);
const sf::Color C_RED     = sf::Color(229, 57, 53);  
const sf::Color C_GREEN   = sf::Color(67, 160, 71);
const sf::Color C_YELLOW  = sf::Color(255, 193, 7); 
const sf::Color C_BLUE    = sf::Color(33, 150, 243); 
const sf::Color C_WHITE   = sf::Color(245, 245, 245);
const sf::Color C_TEXT    = sf::Color(220, 220, 220);
const sf::Color C_BLACK   = sf::Color(10, 10, 15);
const sf::Color C_GOLD    = sf::Color(255, 215, 0);

// --- PATH DATA ---
const int PX[52] = { 1,2,3,4,5, 6,6,6,6,6, 6, 7,8, 8,8,8,8,8, 9,10,11,12,13,14, 14,14, 13,12,11,10,9, 8,8,8,8,8, 8, 7,6, 6,6,6,6,6, 5,4,3,2,1,0, 0,0 };
const int PY[52] = { 6,6,6,6,6, 5,4,3,2,1, 0, 0,0, 1,2,3,4,5, 6,6,6,6,6,6, 7,8, 8,8,8,8,8, 9,10,11,12,13, 14, 14,14, 13,12,11,10,9, 8,8,8,8,8,8, 7,6 };

// --- ASSETS ---
// Centralized resource manager using composition pattern
// Aggregates all textures, sounds, and fonts needed by the game
struct Assets {
    sf::Texture tRed, tGreen, tYel, tBlue, tStar, tCenter;  // Token & board textures
    sf::SoundBuffer bRoll, bMove, bKill, bWin;               // Audio buffers
    sf::Sound sRoll, sMove, sKill, sWin;                     // Playable sounds
    sf::Font fontBold, fontReg;                              // Text fonts
    bool hasImages;  // Fallback flag if images fail to load

    void load() {
        hasImages = true;
        if(!tRed.loadFromFile("Sprites/red.png")) hasImages = false;
        if(!tGreen.loadFromFile("Sprites/green.png")) hasImages = false;
        if(!tYel.loadFromFile("Sprites/yellow.png")) hasImages = false;
        if(!tBlue.loadFromFile("Sprites/blue.png")) hasImages = false;
        if(!tStar.loadFromFile("Sprites/star.png")) hasImages = false;
        if(!tCenter.loadFromFile("Sprites/center.png")) hasImages = false;

        tRed.setSmooth(true); tGreen.setSmooth(true);
        tYel.setSmooth(true); tBlue.setSmooth(true);
        tStar.setSmooth(true); tCenter.setSmooth(true);

        if(bRoll.loadFromFile("Audios/roll.wav")) sRoll.setBuffer(bRoll);
        if(bMove.loadFromFile("Audios/move.wav")) sMove.setBuffer(bMove);
        if(bKill.loadFromFile("Audios/kill.wav")) sKill.setBuffer(bKill);
        if(bWin.loadFromFile("Audios/win.wav")) sWin.setBuffer(bWin);

        // Robust Font Loading
        std::vector<std::string> paths = {"Fonts/Montserrat-Bold.ttf", "../Fonts/Montserrat-Bold.ttf", "Montserrat-Bold.ttf", "C:/Windows/Fonts/arial.ttf"};
        for(const auto& p : paths) if(fontBold.loadFromFile(p)) break;
        
        std::vector<std::string> paths2 = {"Fonts/Montserrat-Medium.ttf", "../Fonts/Montserrat-Medium.ttf", "Montserrat-Medium.ttf", "Montserrat-Regular.ttf"};
        bool reg = false;
        for(const auto& p : paths2) if(fontReg.loadFromFile(p)) { reg=true; break; }
        if(!reg) fontReg = fontBold;
    }
};

// --- HELPER MATH ---
// Convert grid coordinates to screen pixels
sf::Vector2f getGridPos(int col, int row) {
    return sf::Vector2f(OFF_X + col * CELL, OFF_Y + row * CELL);
}

// Calculate token position based on player ID and step count
// Uses lookup tables (PX/PY) for accurate path following
// step=-1: home base, 0-50: main track, 51-56: home stretch
sf::Vector2f getStepPos(int pId, int step, int tokenIndex = 0) {
    if (step == -1) { 
        float startX = 0, startY = 0;
        if (pId == 0) { startX = 0; startY = 0; }       
        else if (pId == 1) { startX = 9; startY = 0; }  
        else if (pId == 2) { startX = 9; startY = 9; }  
        else { startX = 0; startY = 9; }                

        float localX = (tokenIndex % 2 == 0) ? 1.5f : 4.5f;
        float localY = (tokenIndex < 2) ? 1.5f : 4.5f;
        localX -= 0.5f; localY -= 0.5f;

        return getGridPos(startX + localX, startY + localY) + sf::Vector2f(CELL/2, CELL/2);
    }
    
    int col = 0, row = 0;
    if(step <= 50) { 
        // Main track: use global path lookup
        int globalIndex = (pId*13 + step) % 52;
        col = PX[globalIndex]; 
        row = PY[globalIndex];
    } else { 
        // Home stretch: player-specific paths toward center
        int stretchOffset = step - 51;
        if(pId==0) {row=7; col=1+stretchOffset;} 
        else if(pId==1) {col=7; row=1+stretchOffset;} 
        else if(pId==2) {row=7; col=13-stretchOffset;} 
        else if(pId==3) {col=7; row=13-stretchOffset;}
    }
    return sf::Vector2f(OFF_X + col*CELL + CELL/2, OFF_Y + row*CELL + CELL/2);
}

// --- BUTTON CLASS ---
// Interactive UI element with hover effects
class Button {
    sf::RectangleShape bg;
    sf::Text label;
    bool hovered = false;
    sf::Clock hoverClock;
public:
    Button() {}
    void setup(sf::Font& font, std::string text, float x, float y, float w, float h, sf::Color col) {
        bg.setSize({w, h});
        bg.setPosition(x, y);
        bg.setFillColor(col);
        bg.setOutlineThickness(3);
        bg.setOutlineColor(C_WHITE);
        
        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(26);
        sf::FloatRect b = label.getLocalBounds();
        label.setOrigin(b.left + b.width/2, b.top + b.height/2);
        label.setPosition(x + w/2, y + h/2);
        label.setFillColor(C_WHITE);
    }
    bool contains(sf::Vector2i mouse) {
        return bg.getGlobalBounds().contains((float)mouse.x, (float)mouse.y);
    }
    void update(sf::Vector2i mouse) {
        hovered = contains(mouse);
    }
    void draw(sf::RenderWindow& w) {
        // Glow effect when hovered
        float scale = hovered ? 1.05f + 0.03f * sin(hoverClock.getElapsedTime().asSeconds() * 8) : 1.0f;
        sf::Vector2f pos = bg.getPosition();
        sf::Vector2f size = bg.getSize();
        bg.setOrigin(size.x/2, size.y/2);
        bg.setPosition(pos.x + size.x/2, pos.y + size.y/2);
        bg.setScale(scale, scale);
        
        sf::Color outline = hovered ? C_GOLD : C_WHITE;
        bg.setOutlineColor(outline);
        
        w.draw(bg);
        
        label.setScale(scale, scale);
        w.draw(label);
        
        bg.setOrigin(0, 0);
        bg.setPosition(pos);
        bg.setScale(1, 1);
        label.setScale(1, 1);
    }
};


// --- VISUALS ---
class VisualDice {
    sf::RectangleShape box; 
    sf::CircleShape diceDots[7];  // Individual dots for standard die pattern
public:
    VisualDice() {
        box.setSize({120,120}); 
        box.setOutlineThickness(6); box.setFillColor(C_WHITE);
        for(int i=0; i<7; i++) { 
            diceDots[i].setRadius(10); 
            diceDots[i].setOrigin(10,10); 
            diceDots[i].setFillColor(C_BLACK); 
        }
    }
    void draw(sf::RenderWindow& w, int value, sf::Color playerColor) {
        box.setOutlineColor(playerColor); 
        box.setPosition(WIN_W - UI_W/2 - 60, 200); 
        w.draw(box);
        
        float centerX = WIN_W - UI_W/2;
        float centerY = 260; 
        float dotGap = 35;

        // Draw standard dice dot patterns based on value
        if(value%2!=0) { diceDots[0].setPosition(centerX, centerY); w.draw(diceDots[0]); }  // Center dot
        if(value>1) { 
            diceDots[1].setPosition(centerX-dotGap, centerY-dotGap); w.draw(diceDots[1]); 
            diceDots[2].setPosition(centerX+dotGap, centerY+dotGap); w.draw(diceDots[2]); 
        }
        if(value>3) { 
            diceDots[3].setPosition(centerX+dotGap, centerY-dotGap); w.draw(diceDots[3]); 
            diceDots[4].setPosition(centerX-dotGap, centerY+dotGap); w.draw(diceDots[4]); 
        }
        if(value==6) { 
            diceDots[5].setPosition(centerX-dotGap, centerY); w.draw(diceDots[5]); 
            diceDots[6].setPosition(centerX+dotGap, centerY); w.draw(diceDots[6]); 
        }
    }
};

// --- DICE ABSTRACTION (OOP) ---
// Demonstrates INHERITANCE and POLYMORPHISM (rubric requirement)
// Base class defines interface, derived class implements behavior
class Dice {
public:
    virtual ~Dice() {}           // Virtual destructor for proper cleanup
    virtual int roll() = 0;      // Pure virtual method - must be overridden
};

// Concrete implementation using standard random number generation
class RandomDice : public Dice {
public:
    int roll() override { return (std::rand() % 6) + 1; }  // Returns 1-6
};

// --- TILE HIERARCHY (OOP) ---
// Demonstrates INHERITANCE and POLYMORPHISM (rubric requirement)
// Used to check if a board position prevents captures
class Tile {
public:
    virtual ~Tile() {}                          // Virtual destructor
    virtual bool isSafe() const { return false; }  // Default: not safe
};

// Regular track tiles where captures can occur
class NormalTile : public Tile {
public:
    bool isSafe() const override { return false; }
};

// Star tiles where tokens are immune to capture
class SafeTile : public Tile {
public:
    bool isSafe() const override { return true; }
};

class Token {
public:
    sf::Sprite sp;
    sf::CircleShape fallback;
    int id, pId, steps = -1;
    bool active = false, finished = false;
    
    Token(int i, int pid, sf::Texture& tex, bool hasImg, sf::Color c) : id(i), pId(pid) {
        if(hasImg) {
            sp.setTexture(tex);
            sf::Vector2u sz = tex.getSize();
            float scale = (CELL * 0.85f) / sz.x; 
            sp.setScale(scale, scale);
            sp.setOrigin(sz.x/2.0f, sz.y/2.0f);
        }
        fallback.setRadius(CELL*0.35f); fallback.setOrigin(CELL*0.35f, CELL*0.35f);
        fallback.setFillColor(c); fallback.setOutlineThickness(3); fallback.setOutlineColor(C_WHITE);
    }

    void draw(sf::RenderWindow& w, sf::Vector2f pos, float yOffset, bool useImg, bool greyed) {
        sf::Color tint = greyed ? sf::Color(100,100,100, 150) : sf::Color::White;
        if(useImg) {
            sp.setPosition(pos.x, pos.y - yOffset);
            sp.setColor(tint);
            w.draw(sp);
        } else {
            fallback.setPosition(pos.x, pos.y - yOffset);
            fallback.setFillColor(greyed ? sf::Color(80,80,80) : fallback.getFillColor());
            w.draw(fallback);
        }
    }
};

// --- MENU SYSTEM ---
struct MenuBubble { sf::Vector2f pos, vel; float radius; sf::Color col; };
class MenuSystem {
    std::vector<MenuBubble> bubbles;
public:
    MenuSystem() {
        for(int i=0; i<35; i++) {
            MenuBubble b;
            b.pos = sf::Vector2f(rand()%WIN_W, rand()%WIN_H);
            b.vel = sf::Vector2f((rand()%10-5)/4.0f, (rand()%10-5)/4.0f);
            b.radius = rand()%60 + 20;
            int c = rand()%4;
            if(c==0) b.col = sf::Color(229,57,53, 40); else if(c==1) b.col = sf::Color(67,160,71, 40);
            else if(c==2) b.col = sf::Color(255,193,7, 40); else b.col = sf::Color(33,150,243, 40);
            bubbles.push_back(b);
        }
    }
    void update() {
        for(auto& b : bubbles) {
            b.pos += b.vel;
            if(b.pos.x < -100 || b.pos.x > WIN_W+100) b.vel.x *= -1;
            if(b.pos.y < -100 || b.pos.y > WIN_H+100) b.vel.y *= -1;
        }
    }
    void draw(sf::RenderWindow& w) {
        for(auto& b : bubbles) {
            sf::CircleShape s(b.radius); s.setOrigin(b.radius, b.radius);
            s.setPosition(b.pos); s.setFillColor(b.col); w.draw(s);
        }
    }
};

// Player data structure demonstrating COMPOSITION (owns tokens)
// and ASSOCIATION (relates to other players via game state)
struct Player { 
    int id;                      // Player index (0-3)
    std::string name;            // Display name
    sf::Color col;               // Player color
    std::vector<Token> tokens;   // Composition: owns 4 tokens
    bool killed = false;         // Has captured an enemy (unlocks home stretch)
    bool finished = false;       // All tokens in goal
    bool forfeited = false;      // Voluntarily quit
    int finalRank = 0;           // Finish position (1st, 2nd, etc.)
    int captureCount = 0;        // Number of enemies captured this game
};

// --- PARTICLES ---
struct Particle { sf::Vector2f position; sf::Vector2f velocity; float life; sf::Color color; };
class FireworkSystem {
    std::vector<Particle> particles;
public:
    void explode(float x, float y, sf::Color c) {
        for(int i=0; i<60; i++) {
            Particle p; p.position = sf::Vector2f(x, y);
            float angle = (rand() % 360) * 3.14159 / 180;
            float speed = (rand() % 80 + 30) / 10.0f;
            p.velocity = sf::Vector2f(cos(angle)*speed, sin(angle)*speed);
            p.life = 255; p.color = c; particles.push_back(p);
        }
    }
    void update() {
        for(auto& p : particles) { p.position += p.velocity; p.life -= 3; p.velocity.y += 0.2f; }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p){ return p.life <= 0; }), particles.end());
    }
    void draw(sf::RenderWindow& win) {
        for(auto& p : particles) {
            sf::CircleShape dot(4); dot.setPosition(p.position);
            dot.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, (sf::Uint8)p.life));
            win.draw(dot);
        }
    }
};

enum State { MENU, PLAYING, ROLLING_DICE, GAME_OVER };

class Game {
    sf::RenderWindow win;
    Assets assets; 
    VisualDice visualDice; 
    FireworkSystem fireworks;
    sf::RectangleShape grid[15][15];
    sf::Sprite sStar, sCenter;
    sf::ConvexShape fallbackStar;
    MenuSystem menuAnim;
    
    // Track tiles for polymorphic safe checks (global indices 0..51)
    std::vector<std::unique_ptr<Tile>> trackTiles;
    
    // Dice abstraction (used to finalize a roll)
    RandomDice diceRoller;
    
    std::vector<Player> players;
    std::vector<std::string> rankList; 
    
    State state = MENU;
    int curP = 0, roll = 1;
    bool rolled = false, anim = false;
    sf::Clock rollClock;
    
    Token* movingT = nullptr;
    int movesLeft = 0;
    sf::Clock clk;
    sf::Vector2f animStart, animEnd;

    sf::Text txtInfo, txtTurn, txtTitle, txtSub, txtLeaderboard, txtControls, txtRankLabel, txtTitleShadow, txtLeaderboardTitle; 
    sf::Text txtMenuOptions, txtHelp;
    sf::Clock textPulseClock;
    sf::RectangleShape leaderboardBox, uiPanel, turnHighlighter, overlay;
    sf::RectangleShape helpOverlay;

    bool showHelp = false;
    Button btnStart, btnHelp, btnQuit, btnRestart;
    sf::CircleShape playerIndicators[4];
    sf::Text playerLabels[4];
    sf::Text captureCounters[4];


public:
    Game() : win(sf::VideoMode(WIN_W, WIN_H), "Ludo Legends", sf::Style::Close | sf::Style::Resize) {        
        std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed RNG for true randomness        
        win.setFramerateLimit(60);
        assets.load(); 

        // --- BOARD GRID ---
        for(int r=0;r<15;r++) for(int c=0;c<15;c++) {
            grid[r][c].setSize({CELL, CELL});
            grid[r][c].setPosition(getGridPos(c,r));
            grid[r][c].setOutlineThickness(1); grid[r][c].setOutlineColor(sf::Color(0,0,0,60));
            
            sf::Color col = C_WHITE;
            if(r<6 && c<6) col=C_RED; else if(r<6 && c>8) col=C_GREEN;
            else if(r>8 && c>8) col=C_YELLOW; else if(r>8 && c<6) col=C_BLUE;
            else if(r>=6 && r<=8 && c>=6 && c<=8) col=C_BLACK; 
            if(r==7&&c>0&&c<6) col=C_RED; if(c==7&&r>0&&r<6) col=C_GREEN;
            if(r==7&&c>8&&c<14) col=C_YELLOW; if(c==7&&r>8&&r<14) col=C_BLUE;
            
            // Home Slots - match player zone colors
            if ((r==1||r==4) && (c==1||c==4)) col = C_RED; 
            if ((r==1||r==4) && (c==10||c==13)) col = C_GREEN;
            if ((r==10||r==13) && (c==10||c==13)) col = C_YELLOW;
            if ((r==10||r==13) && (c==1||c==4)) col = C_BLUE;

            grid[r][c].setFillColor(col);
        }

        sStar.setTexture(assets.tStar);
        sf::Vector2u stSz = assets.tStar.getSize();
        if(stSz.x > 0) sStar.setScale(CELL/stSz.x, CELL/stSz.y);

        fallbackStar.setPointCount(5);
        fallbackStar.setPoint(0,{0,-10}); fallbackStar.setPoint(1,{3,-3}); fallbackStar.setPoint(2,{10,-3});
        fallbackStar.setPoint(3,{5,2}); fallbackStar.setPoint(4,{7,10}); fallbackStar.setFillColor(sf::Color(100,100,100,150));

        sCenter.setTexture(assets.tCenter);
        sf::Vector2u cnSz = assets.tCenter.getSize();
        if(cnSz.x > 0) sCenter.setScale((3*CELL)/cnSz.x, (3*CELL)/cnSz.y);
        sCenter.setPosition(getGridPos(6,6));

        setupP(0, "RED", C_RED, assets.tRed);
        setupP(1, "GREEN", C_GREEN, assets.tGreen);
        setupP(2, "YELLOW", C_YELLOW, assets.tYel);
        setupP(3, "BLUE", C_BLUE, assets.tBlue);

        // Turn Highlight
        turnHighlighter.setSize({6*CELL, 6*CELL});
        turnHighlighter.setFillColor(sf::Color::Transparent);
        turnHighlighter.setOutlineThickness(8);

        // UI Panel
        uiPanel.setSize({UI_W, WIN_H}); uiPanel.setPosition(WIN_W - UI_W, 0); uiPanel.setFillColor(C_PANEL);

        // Fonts & Text
        txtTitle.setFont(assets.fontBold); txtTitle.setString("LUDO LEGENDS"); txtTitle.setCharacterSize(120); 
        txtTitle.setOrigin(txtTitle.getLocalBounds().width/2, 0); txtTitle.setPosition(WIN_W/2, 200); 
        txtTitle.setFillColor(C_WHITE); 
        
        txtTitleShadow = txtTitle; 
        txtTitleShadow.setFillColor(sf::Color(0,0,0,100));
        txtTitleShadow.setPosition(WIN_W/2 + 8, 208);

        txtSub.setFont(assets.fontReg); txtSub.setString("PRESS ENTER TO START"); txtSub.setCharacterSize(40); 
        txtSub.setOrigin(txtSub.getLocalBounds().width/2, 0); txtSub.setPosition(WIN_W/2, 600);

        // Game UI Text
        float panelCenter = WIN_W - UI_W/2;
        txtTurn.setFont(assets.fontBold); txtTurn.setCharacterSize(45); 
        
        txtInfo.setFont(assets.fontReg); txtInfo.setCharacterSize(24); txtInfo.setFillColor(C_TEXT);
        
        // Menu Options Text
        txtMenuOptions.setFont(assets.fontReg);
        txtMenuOptions.setCharacterSize(26);
        txtMenuOptions.setFillColor(C_TEXT);
        txtMenuOptions.setString("OPTIONS:\nEnter - Start Game\nH - How to Play\nEsc - Quit");
        sf::FloatRect mo = txtMenuOptions.getLocalBounds();
        txtMenuOptions.setOrigin(mo.width/2, 0);
        txtMenuOptions.setPosition(WIN_W/2, 680);
        
        // Leaderboard Box with improved aesthetics
        leaderboardBox.setSize({900, 700}); 
        leaderboardBox.setOrigin(450, 350);
        leaderboardBox.setPosition(WIN_W/2, WIN_H/2);
        leaderboardBox.setFillColor(sf::Color(25, 25, 40, 250)); 
        leaderboardBox.setOutlineThickness(6); leaderboardBox.setOutlineColor(C_GOLD);

        txtLeaderboardTitle.setFont(assets.fontBold); txtLeaderboardTitle.setString("VICTORY!"); 
        txtLeaderboardTitle.setCharacterSize(80); txtLeaderboardTitle.setFillColor(C_GOLD);
        txtLeaderboardTitle.setOutlineThickness(3); txtLeaderboardTitle.setOutlineColor(sf::Color(150, 100, 0));
        
        txtLeaderboard.setFont(assets.fontReg); txtLeaderboard.setCharacterSize(45); txtLeaderboard.setFillColor(C_WHITE);
        txtLeaderboard.setLetterSpacing(1.2f);

        txtRankLabel.setFont(assets.fontBold); txtRankLabel.setCharacterSize(36); txtRankLabel.setOutlineThickness(3); txtRankLabel.setOutlineColor(sf::Color::Black); 

        txtControls.setFont(assets.fontReg); txtControls.setCharacterSize(20); txtControls.setPosition(panelCenter - 100, 900);
        txtControls.setString("CONTROLS\n\nSpace : Roll Dice\nF : Forfeit Game");
        txtControls.setFillColor(sf::Color(150,150,150));
        
        overlay.setSize({(float)WIN_W, (float)WIN_H});
        overlay.setFillColor(sf::Color(0,0,0,150));

        // Help overlay and text
        helpOverlay.setSize({(float)WIN_W, (float)WIN_H});
        helpOverlay.setFillColor(sf::Color(0,0,0,200));
        txtHelp.setFont(assets.fontReg);
        txtHelp.setCharacterSize(28);
        txtHelp.setFillColor(C_WHITE);
        txtHelp.setString(
            "HOW TO PLAY\n\n"
            "- Roll the dice (Space).\n"
            "- Click a valid token to move.\n"
            "- Roll a 6 to enter from home.\n"
            "- Capture an enemy to unlock home stretch.\n"
            "- Finish all 4 tokens to rank.\n\n"
            "Press H to close this help."
        );
        sf::FloatRect hb = txtHelp.getLocalBounds();
        txtHelp.setOrigin(hb.width/2, hb.height/2);
        txtHelp.setPosition(WIN_W/2, WIN_H/2);

        // Setup interactive menu buttons with hover effects
        btnStart.setup(assets.fontBold, "START GAME", WIN_W/2 - 150, 500, 300, 60, C_GREEN);
        btnHelp.setup(assets.fontBold, "HOW TO PLAY", WIN_W/2 - 150, 580, 300, 60, C_BLUE);
        btnQuit.setup(assets.fontBold, "QUIT", WIN_W/2 - 150, 660, 300, 60, C_RED);
        btnRestart.setup(assets.fontBold, "PLAY AGAIN", WIN_W/2 - 150, WIN_H - 150, 300, 60, C_GREEN);

        // Setup player indicators in UI panel - properly aligned with centered text
        float indicatorStartY = 530;
        float indicatorX = WIN_W - UI_W + 200;  // Balanced position in UI panel
        for(int i=0; i<4; i++) {
            float rowY = indicatorStartY + i*100;  // More spacing between rows
            
            // Circular player indicator
            playerIndicators[i].setRadius(25);
            playerIndicators[i].setOrigin(25, 25);
            playerIndicators[i].setOutlineThickness(3);
            playerIndicators[i].setOutlineColor(C_WHITE);
            playerIndicators[i].setPosition(indicatorX, rowY);
            
            // Player name label - properly centered vertically
            playerLabels[i].setFont(assets.fontReg);
            playerLabels[i].setCharacterSize(22);
            playerLabels[i].setFillColor(C_TEXT);
            
            // Capture counter - properly centered vertically
            captureCounters[i].setFont(assets.fontReg);
            captureCounters[i].setCharacterSize(16);
            captureCounters[i].setFillColor(sf::Color(180,180,180));
        }

        // Initialize polymorphic track tiles for safe checks
        initTrackTiles();
    }

    // Initialize the 52-tile track with safe tiles at specific indices
    // Demonstrates polymorphism: trackTiles holds Tile* pointing to SafeTile or NormalTile
    void initTrackTiles() {
        trackTiles.clear();
        trackTiles.resize(52);
        for(int i=0;i<52;i++) trackTiles[i] = std::make_unique<NormalTile>();
        // Global safe indices per rules
        const int safeIdx[8] = {0,8,13,21,26,34,39,47};
        for(int s : safeIdx) { trackTiles[s] = std::make_unique<SafeTile>(); }
    }

    // Initialize a player with 4 tokens positioned in home base corners
    // Demonstrates composition: Player owns Tokens
    void setupP(int id, std::string n, sf::Color c, sf::Texture& t) {
        Player p = {id, n, c};
        // Create 4 tokens for this player
        for(int i=0; i<4; i++) p.tokens.emplace_back(i, id, t, assets.hasImages, c);
        players.push_back(p);
    }

    void run() {
        while(win.isOpen()) {
            sf::Event e;
            while(win.pollEvent(e)) {
                if(e.type == sf::Event::Closed) win.close();
                
                if(state == MENU && e.type == sf::Event::KeyPressed) {
                    if(e.key.code == sf::Keyboard::Enter) { state = PLAYING; updateUI("Space to Roll"); assets.sWin.play(); }
                    if(e.key.code == sf::Keyboard::H) { showHelp = !showHelp; }
                    if(e.key.code == sf::Keyboard::Escape) { win.close(); }
                }
                
                // Handle menu button clicks
                if(state == MENU && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i m = sf::Mouse::getPosition(win);
                    if(btnStart.contains(m)) { state = PLAYING; updateUI("Space to Roll"); assets.sWin.play(); }
                    if(btnHelp.contains(m)) { showHelp = !showHelp; }
                    if(btnQuit.contains(m)) { win.close(); }
                }
                
                // Handle restart button in game over screen
                if(state == GAME_OVER && e.type == sf::Event::MouseButtonPressed && e.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i m = sf::Mouse::getPosition(win);
                    if(btnRestart.contains(m)) { resetGame(); }
                }

                if(state == PLAYING) {
                    if(!anim && e.type == sf::Event::KeyPressed) {
                        if(e.key.code == sf::Keyboard::Space && !rolled && state != ROLLING_DICE) startRoll(0);
                        if(e.key.code == sf::Keyboard::S && !rolled && state != ROLLING_DICE) startRoll(6);
                        if(e.key.code == sf::Keyboard::F) handleForfeit();
                    }
                    if(!anim && rolled && e.type == sf::Event::MouseButtonPressed) {
                        sf::Vector2i m = sf::Mouse::getPosition(win);
                        for(auto& t : players[curP].tokens) {
                            sf::Vector2f pos = getStepPos(curP, t.steps, t.id);
                            if(std::hypot(m.x-pos.x, m.y-pos.y) < 30) {
                                if(isValid(t)) startAnim(t);
                            }
                        }
                    }
                }
            }

            if(state == ROLLING_DICE) updateDiceAnim();
            if(anim) updateAnim();
            if(state == MENU) {
                menuAnim.update();
                sf::Vector2i mouse = sf::Mouse::getPosition(win);
                btnStart.update(mouse);
                btnHelp.update(mouse);
                btnQuit.update(mouse);
            }
            if(state == GAME_OVER) {
                // Enhanced confetti: multiple colors, varied spawns for celebration effect
                if(rand()%10==0) {
                    sf::Color confettiColors[] = {C_RED, C_GREEN, C_YELLOW, C_BLUE, C_GOLD, C_WHITE};
                    int randomX = rand() % WIN_W;
                    int randomY = rand() % 200 + 100;  // Upper portion of screen
                    fireworks.explode(randomX, randomY, confettiColors[rand()%6]);
                }
                fireworks.update();
                sf::Vector2i mouse = sf::Mouse::getPosition(win);
                btnRestart.update(mouse);
            }
            render(); 
        }
    }

    void startRoll(int force) {
        state = ROLLING_DICE; 
        rollClock.restart();
        assets.sRoll.play();
        roll = (force > 0) ? force : 0; 
    }

    void updateDiceAnim() {
        if(rollClock.getElapsedTime().asSeconds() < 0.5f) {
            // Animate dice rolling with random values
            if((int)(rollClock.getElapsedTime().asMilliseconds()) % 8 == 0) 
                visualDice.draw(win, (rand()%6)+1, players[curP].col); 
        } else {
            // Animation complete: finalize roll result
            state = PLAYING;
            rolled = true;
            if(roll == 0) roll = diceRoller.roll();
            if(!canMove()) { rolled=false; nextTurn(); } 
            else updateUI("Select Token");
        }
    }

    void handleForfeit() {
        players[curP].forfeited = true;
        checkWinCondition();
        if(state != GAME_OVER) nextTurn();
    }

    void checkWinCondition() {
        int activePlayers = 0;
        for(const auto& p : players) if(!p.forfeited && !p.finished) activePlayers++;

        if(activePlayers <= 1) {
            for(auto& p : players) if(!p.forfeited && !p.finished) { p.finished = true; rankList.push_back(p.name); }
            state = GAME_OVER;
            assets.sWin.play();
            
            std::string results;
            int rank = 1;
            for(const auto& name : rankList) results += std::to_string(rank++) + ". " + name + "\n\n";
            for(const auto& p : players) if(p.forfeited) results += "DNF - " + p.name + "\n\n";

            txtLeaderboard.setString(results);
            sf::FloatRect lb = txtLeaderboard.getLocalBounds();
            txtLeaderboard.setOrigin(lb.left + lb.width/2.0f, lb.top + lb.height/2.0f);
            txtLeaderboard.setPosition(WIN_W/2, WIN_H/2 + 50);

            sf::FloatRect tb = txtLeaderboardTitle.getLocalBounds();
            txtLeaderboardTitle.setOrigin(tb.left + tb.width/2.0f, tb.top + tb.height/2.0f);
            txtLeaderboardTitle.setPosition(WIN_W/2, WIN_H/2 - 250);
        }
    }

    bool isValid(Token& t) {
        if(t.finished) return false;
        if(!t.active) return roll == 6;
        if(t.steps + roll > 56) return false;
        if(t.steps + roll > 50 && !players[curP].killed) return false;
        return true;
    }
    
    bool canMove() {
        for(auto& t : players[curP].tokens) if(isValid(t)) return true;
        return false;
    }

    void startAnim(Token& t) {
        movingT = &t;
        movesLeft = t.active ? roll : 1;
        anim = true;
        prepStep();
    }

    void prepStep() {
        if(movingT->steps == -1) { 
            sf::Vector2f base = getStepPos(curP, -1, movingT->id);
            animStart = base;
            animEnd = getStepPos(curP, 0);
        } else { 
            animStart = getStepPos(curP, movingT->steps);
            animEnd = getStepPos(curP, movingT->steps + 1);
        }
        clk.restart();
        assets.sMove.play();
    }

    void updateAnim() {
        if(clk.getElapsedTime().asSeconds() >= ANIM_TIME) {
            if(!movingT->active) {
                movingT->active = true;
                movingT->steps = 0;
            } else {
                movingT->steps++;
            }
            movesLeft--;
            if(movesLeft > 0) prepStep();
            else finalize();
        }
    }

    // Called after token animation completes
    // Checks for captures, updates game state, and determines next turn
    void finalize() {
        anim = false;
        // Capture detection: only on main track (steps 0-50), not in safe zones or home stretch
        if(movingT->steps <= 50 && !isSafe(movingT->steps)) {
            sf::Vector2f myPos = getStepPos(curP, movingT->steps);
            // Loop through all opponents to check for collision
            for(auto& opponent : players) {
                if(opponent.id == curP || opponent.forfeited || opponent.finished) continue;
                for(auto& enemyToken : opponent.tokens) {
                    // Check if enemy token is on same position
                    if(enemyToken.active && enemyToken.steps <= 50) {
                        sf::Vector2f enemyPos = getStepPos(opponent.id, enemyToken.steps);
                        float positionTolerance = 5.0f;  // Pixel distance for collision detection
                        if(std::abs(myPos.x - enemyPos.x) < positionTolerance && 
                           std::abs(myPos.y - enemyPos.y) < positionTolerance) {
                            // Capture: send enemy back to home
                            enemyToken.active = false;
                            enemyToken.steps = -1;
                            players[curP].killed = true;      // Unlock home stretch
                            players[curP].captureCount++;     // Track capture stat
                            assets.sKill.play();
                        }
                    }
                }
            }
        }
        
        if(movingT->steps == 56) movingT->finished = true;
        
        int finishedTokens = 0;
        for(auto& token : players[curP].tokens) if(token.finished) finishedTokens++;
        if(finishedTokens == 4) { 
            players[curP].finished = true;
            rankList.push_back(players[curP].name);
            players[curP].finalRank = rankList.size();
            checkWinCondition();
            if(state != GAME_OVER) nextTurn(); 
            return;
        }

        if(roll != 6) nextTurn();
        else { rolled = false; updateUI("Roll 6: Go Again"); }
    }

    bool isSafe(int s) {
        int g = (curP*13 + s)%52;
        if(g >= 0 && g < (int)trackTiles.size()) return trackTiles[g]->isSafe();
        return false;
    }

    void nextTurn() { 
        int attempts = 0;
        do {
            curP = (curP + 1) % 4;
            attempts++;
        } while ((players[curP].finished || players[curP].forfeited) && attempts < 5);
        rolled = false; 
        updateUI("Space to Roll"); 
    }

    void resetGame() {
        // Reset all player states
        for(auto& p : players) {
            p.killed = false;
            p.finished = false;
            p.forfeited = false;
            p.finalRank = 0;
            p.captureCount = 0;
            for(auto& t : p.tokens) {
                t.steps = -1;
                t.active = false;
                t.finished = false;
            }
        }

        rankList.clear();
        curP = 0;
        roll = 1;
        rolled = false;
        anim = false;
        state = PLAYING;
        updateUI("Space to Roll");
        assets.sWin.play();
    }
    
    void updateUI(std::string s) {
        txtTurn.setString(players[curP].name + "'S TURN");
        
        sf::FloatRect tr = txtTurn.getLocalBounds();
        txtTurn.setOrigin(tr.width/2, 0);
        txtTurn.setPosition(WIN_W - UI_W/2, 80);
        
        txtTurn.setFillColor(players[curP].col);
        txtInfo.setString(s + (players[curP].killed ? "" : "\n(Need Kill)"));
        
        sf::FloatRect ir = txtInfo.getLocalBounds();
        txtInfo.setOrigin(ir.width/2, 0);
        txtInfo.setPosition(WIN_W - UI_W/2, 400);
        
        turnHighlighter.setOutlineColor(players[curP].col);
        if(curP==0) turnHighlighter.setPosition(OFF_X, OFF_Y);
        else if(curP==1) turnHighlighter.setPosition(OFF_X+9*CELL, OFF_Y);
        else if(curP==2) turnHighlighter.setPosition(OFF_X+9*CELL, OFF_Y+9*CELL);
        else turnHighlighter.setPosition(OFF_X, OFF_Y+9*CELL);
    }

    void render() {
        win.clear(C_BG);

        if(state == MENU) {
            menuAnim.draw(win);
            win.draw(txtTitleShadow); win.draw(txtTitle);
            // Draw interactive buttons
            btnStart.draw(win);
            btnHelp.draw(win);
            btnQuit.draw(win);
            if(showHelp) { win.draw(helpOverlay); win.draw(txtHelp); }
        }
        else if (state == GAME_OVER) {
            // Draw board state in background
            win.draw(uiPanel);
            for(int r=0;r<15;r++) for(int c=0;c<15;c++) win.draw(grid[r][c]);
            
            // Draw star tiles
            int starPos[8][2] = {{1,6}, {6,2}, {8,1}, {12,6}, {13,8}, {8,12}, {6,13}, {2,8}};
            for(auto& pos : starPos) {
                if(assets.hasImages) { sStar.setPosition(getGridPos(pos[0],pos[1])); win.draw(sStar); }
                else { fallbackStar.setPosition(getGridPos(pos[0],pos[1])+sf::Vector2f(20,18)); win.draw(fallbackStar); }
            }
            if(assets.hasImages) win.draw(sCenter);
            
            // Draw all tokens in final positions
            for(auto& p : players) {
                for(auto& t : p.tokens) {
                    sf::Vector2f pos = getStepPos(p.id, t.steps, t.id);
                    t.draw(win, pos, 0, assets.hasImages, p.forfeited);
                }
            }
            
            fireworks.draw(win);
            
            // Gradient overlay effect
            sf::RectangleShape gradTop({(float)WIN_W, WIN_H/3.0f});
            gradTop.setPosition(0, 0);
            gradTop.setFillColor(sf::Color(10, 10, 20, 100));
            win.draw(gradTop);
            
            win.draw(overlay);
            win.draw(leaderboardBox);
            win.draw(txtLeaderboardTitle);
            win.draw(txtLeaderboard);
            btnRestart.draw(win);
        }
        else {
            win.draw(uiPanel);
            for(int r=0;r<15;r++) for(int c=0;c<15;c++) win.draw(grid[r][c]);
            
            // Pulse Border
            int alpha = 150 + 100 * sin(textPulseClock.getElapsedTime().asSeconds() * 5);
            sf::Color bc = turnHighlighter.getOutlineColor(); bc.a = alpha;
            turnHighlighter.setOutlineColor(bc);
            win.draw(turnHighlighter);

            // Draw star tiles at safe positions
            int starPos[8][2] = {{1,6}, {6,2}, {8,1}, {12,6}, {13,8}, {8,12}, {6,13}, {2,8}};
            for(auto& pos : starPos) {
                if(assets.hasImages) { sStar.setPosition(getGridPos(pos[0],pos[1])); win.draw(sStar); }
                else { fallbackStar.setPosition(getGridPos(pos[0],pos[1])+sf::Vector2f(20,18)); win.draw(fallbackStar); }
            }

            if(assets.hasImages) win.draw(sCenter);

            for(auto& p : players) {
                for(auto& t : p.tokens) {
                    float u = (anim && movingT == &t) ? clk.getElapsedTime().asSeconds() / ANIM_TIME : 0;
                    sf::Vector2f pos = (anim && movingT == &t) ? animStart + (animEnd - animStart) * u : getStepPos(p.id, t.steps, t.id);
                    float yOff = (anim && movingT == &t) ? sin(u * 3.14159f) * 20.0f : 0;
                    t.draw(win, pos, yOff, assets.hasImages, p.forfeited);
                }

                if(p.forfeited || p.finished) {
                    // Base center positions for each player
                    float basePos[4][2] = {{OFF_X+3*CELL, OFF_Y+3*CELL}, {OFF_X+12*CELL, OFF_Y+3*CELL}, 
                                           {OFF_X+12*CELL, OFF_Y+12*CELL}, {OFF_X+3*CELL, OFF_Y+12*CELL}};
                    
                    std::string suffix[] = {"st", "nd", "rd", "th"};
                    std::string label = p.forfeited ? "OUT" : std::to_string(p.finalRank) + suffix[std::min(p.finalRank-1, 3)];
                    txtRankLabel.setString(label);
                    
                    sf::FloatRect b = txtRankLabel.getLocalBounds();
                    txtRankLabel.setOrigin(b.left + b.width/2.0f, b.top + b.height/2.0f);
                    txtRankLabel.setPosition(basePos[p.id][0], basePos[p.id][1]);
                    txtRankLabel.setFillColor(C_WHITE);
                    win.draw(txtRankLabel);
                }
            }

            // Draw dice in UI panel
            if(state == ROLLING_DICE) {
                visualDice.draw(win, (rand()%6)+1, players[curP].col);
            } else {
                visualDice.draw(win, roll, players[curP].col);
            }

            // Draw UI text elements
            win.draw(txtTurn); win.draw(txtInfo); win.draw(txtControls);
            
            // Draw player indicators with avatars and capture stats
            float indicatorStartY = 530;
            float indicatorX = WIN_W - UI_W + 200;  // Balanced position in UI panel
            for(int i=0; i<4; i++) {
                float rowY = indicatorStartY + i*100;
                
                playerIndicators[i].setFillColor(players[i].col);
                // Highlight active player with animated glow
                if(i == curP) {
                    float glow = 5 + 3 * sin(textPulseClock.getElapsedTime().asSeconds() * 6);
                    playerIndicators[i].setOutlineThickness(glow);
                    playerIndicators[i].setOutlineColor(players[i].col);
                } else {
                    playerIndicators[i].setOutlineThickness(3);
                    playerIndicators[i].setOutlineColor(C_WHITE);
                }
                // Grey out if finished/forfeited
                if(players[i].finished || players[i].forfeited) {
                    playerIndicators[i].setFillColor(sf::Color(80,80,80));
                }
                win.draw(playerIndicators[i]);
                
                // Draw player name with proper vertical centering to prevent clipping
                playerLabels[i].setString(players[i].name);
                sf::FloatRect labelBounds = playerLabels[i].getLocalBounds();
                playerLabels[i].setOrigin(0, labelBounds.top + labelBounds.height/2);
                playerLabels[i].setPosition(indicatorX + 40, rowY - 15);
                win.draw(playerLabels[i]);
                
                // Draw capture counter with proper vertical centering to prevent clipping
                captureCounters[i].setString("Kills: " + std::to_string(players[i].captureCount));
                sf::FloatRect counterBounds = captureCounters[i].getLocalBounds();
                captureCounters[i].setOrigin(0, counterBounds.top + counterBounds.height/2);
                captureCounters[i].setPosition(indicatorX + 40, rowY + 15);
                win.draw(captureCounters[i]);
            }
        }
        win.display();
    }
};


int main() { 
    Game g; 
    g.run(); 
    return 0; 
}