#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <cmath>
#include <SFML/Audio.hpp>  

using namespace sf;
using namespace std;

const int M = 25;
const int N = 40;

int grid[M][N] = {0};
int ts = 18; //tile size

// MovementPattern
const int LINEAR = 0;
const int ZIGZAG = 1;
const int CIRCULAR = 2;

// GameState
const int MENU = 0;
const int PLAYING = 1;
const int GAMEOVER = 2;
const int SCOREBOARD = 3;
const int MODE_SELECT = 4;

const int SINGLE_PLAYER = 0;
const int TWO_PLAYER = 1;

// Difficulty
const int EASY = 2;        // 2 enemies
const int MEDIUM = 4;      // 4 enemies
const int HARD = 6;        // 6 enemies
const int CONTINUOUS = -1; // DYNAMIC ENEMIES BECAUSE THEY WILL INCREMENTED AFTER 20 SECONDS

// Player IDs
const int PLAYER1 = 0;
const int PLAYER2 = 1;

// Structure to store score information
struct ScoreEntry {
    int score;
    int timeTaken;  // in seconds
};

struct Player {
    int x, y;
    int dx, dy;
    int score;
    int moveCount;
    bool isBuilding;
    int powerUpCount;
    int lastPowerUpScore;
    int rewardBonusCount;
    bool powerUpActive;
    float powerUpTimer;
bool isFrozen;
float frozenTimer;
    Color color;
    int tileValue; 

    Player(int startX, int startY, Color playerColor, int gridValue) {
        x = startX;
        y = startY;
        dx = dy = 0;
        score = 0;
        moveCount = 0;
        isBuilding = false;
        powerUpCount = 0;
        lastPowerUpScore = 0;
        rewardBonusCount = 0;
        powerUpActive = false;
        powerUpTimer = 0.0f;
        isFrozen = false;
frozenTimer = 0.0f;
        color = playerColor;
        tileValue = gridValue;
    }
   
    void reset(int startX, int startY) {
        x = startX;
        y = startY;
        dx = dy = 0;
        score = 0;
        moveCount = 0;
        isBuilding = false;
        powerUpCount = 0;
        lastPowerUpScore = 0;
       
        rewardBonusCount = 0;
        powerUpActive = false;
        powerUpTimer = 0.0f;
        isFrozen = false;
frozenTimer = 0.0f;
    }
};


bool isOuterLayer(int x, int y) {
    // Check if this tile is on the outer layer by checking if it has at least one adjacent non-solid tile
    if (x > 0 && grid[y][x-1] == 0) return true;
    if (x < N-1 && grid[y][x+1] == 0) return true;
    if (y > 0 && grid[y-1][x] == 0) return true;
    if (y < M-1 && grid[y+1][x] == 0) return true;
   
    return false;
}

bool isValidEnemySpawnPoint(int x, int y) {
    
    int gridX = x / ts;
    int gridY = y / ts;
   
    
    if (gridX < 0 || gridX >= N || gridY < 0 || gridY >= M) {
        return false;
    }
   
    // Check if this position is free (no player tiles)
    if (grid[gridY][gridX] != 0) {
        return false;
    }
   
    return true;
}


struct Enemy {
    int x, y;
    int dx, dy;
    int pattern = LINEAR; // Changed from enum to int
    float angle = 0.0f; // For circular
    int zigzagCounter = 0; // For zigzag

    Enemy() {
       // x=y=300
       bool foundValidPoint = false;
     
    for (int attempts = 0; attempts < 100; attempts++) {  
        // Generate random coordinates within the game area
        x = rand() % (N * ts);
        y = rand() % (M * ts);
       
        if (isValidEnemySpawnPoint(x, y)) {
            foundValidPoint = true;
            break;
        }
    }
   
    
    if (!foundValidPoint) {
        x = (N/2) * ts;
        y = (M/2) * ts;
       
        
        int gridX = x / ts;
        int gridY = y / ts;
        if (gridX >= 0 && gridX < N && gridY >= 0 && gridY < M && grid[gridY][gridX] != 0) {
            // Find the nearest empty cell
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < N; j++) {
                    if (grid[i][j] == 0) {
                        x = j * ts;
                        y = i * ts;
                        break;
                    }
                }
            }
        }
    }
   
    
    dx = 4 - rand() % 8;
    dy = 4 - rand() % 8;
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;
    }

    void moveLinear() {
    int nextX = x + dx;
    int nextY = y + dy;
   
   
    if (nextX >= 0 && nextX < N*ts && nextY >= 0 && nextY < M*ts) {
        // Check if the tile at the next position is a solid tile (grid value 1)
        if (grid[nextY / ts][nextX / ts] == 1) {
            // Bounce off regardless of whether it's an outer layer or not
            if (nextX != x) dx = -dx;
            if (nextY != y) dy = -dy;
            return;
        }
    }
   
    x += dx;
    if (grid[y / ts][x / ts] == 1) {
        dx = -dx;
        x += dx;
    }  
   
    y += dy;
    if (grid[y / ts][x / ts] == 1) {
        dy = -dy;
        y += dy;
    }
}

    void moveZigZag() {
    zigzagCounter++; // for zigzag movement
    if (zigzagCounter % 20 == 0) dx = -dx;
   
    int nextX = x + dx;
    int nextY = y + dy;
   
   
    if (nextX >= 0 && nextX < N*ts && nextY >= 0 && nextY < M*ts) {
        // Check if the tile at the next position is a solid tile (grid value 1)
        if (grid[nextY / ts][nextX / ts] == 1) {
            // Bounce off regardless of whether it's an outer layer or not
            if (nextX != x) dx = -dx;
            if (nextY != y) dy = -dy;
            return;
        }
    }
   
    x += dx;
    if (grid[y / ts][x / ts] == 1) {
        dx = -dx;
        x += dx;
    }
   
    y += dy;
    if (grid[y / ts][x / ts] == 1) {
        dy = -dy;
        y += dy;
    }
}

   void moveCircular() {
    angle += 0.05f;
   
    int oldX = x;
    int oldY = y;
   
   
    int nextX = (int)(x + cos(angle) * 3 + dx);
    int nextY = (int)(y + sin(angle) * 3 + dy);
   
    
    if (nextX >= 0 && nextX < N*ts && nextY >= 0 && nextY < M*ts) {
        // Check if the tile at the next position is a solid tile (grid value 1)
        if (grid[nextY / ts][nextX / ts] == 1) {
            // Bounce off regardless of whether it's an outer layer or not
            dx = -dx;
            dy = -dy;
            return;
        }
    }
   
    // for circle pattern
    x += (int)(cos(angle) * 3 + dx);
    y += (int)(sin(angle) * 3 + dy);

    if (x >= 0 && x < N*ts && y >= 0 && y < M*ts) {
        if (grid[y / ts][x / ts] == 1) {
            // On any solid tile - bounce off
            x = oldX;
            y = oldY;
            dx = -dx;
            dy = -dy;
        }
    }
   
    // Bounce off boundaries
    if (x < 0 || x >= N * ts) dx = -dx;
    if (y < 0 || y >= M * ts) dy = -dy;

    // Clamp position to stay in bounds
    if (x < 0) x = 0;
    if (x >= N * ts) x = N * ts - 1;
    if (y < 0) y = 0;
    if (y >= M * ts) y = M * ts - 1;
}
   
    void move() {
        switch (pattern) {
            case LINEAR: moveLinear(); break;
            case ZIGZAG: moveZigZag(); break;
            case CIRCULAR: moveCircular(); break;
        }
    }
};




void drop(int y, int x)
{
    if (y < 0 || y >= M || x < 0 || x >= N) return; // prevent out-of-bounds access
    if (grid[y][x] != 0) return;

    grid[y][x] = -1;

    drop(y - 1, x);
    drop(y + 1, x);
    drop(y, x - 1);
    drop(y, x + 1);
}


void trackMoves(int x, int y, int grid[M][N], bool& isBuilding, int& moveCount) {
    if (grid[y][x] == 0) {
        if (!isBuilding) {
            isBuilding = true;
        }
    } else if (grid[y][x] == 1) {
        if (isBuilding) {
            moveCount++;
            isBuilding = false;
        }
    }
}
void showModeSelection(RenderWindow& window, Font& font, int& state, int& playerMode) {
    Text title("Select Game Mode", font, 50);
    title.setPosition(200, 100);
    title.setFillColor(Color::Yellow);

    Text opt1("1. Single Player Mode", font, 30);
    opt1.setPosition(220, 200);
    opt1.setFillColor(Color::White);

    Text opt2("2. Two Player Mode", font, 30);
    opt2.setPosition(220, 250);
    opt2.setFillColor(Color::White);

    window.clear(Color::Black);
    window.draw(title);
    window.draw(opt1);
    window.draw(opt2);
    window.display();

    while (window.isOpen() && state == MODE_SELECT) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Num1) {
                    playerMode = SINGLE_PLAYER;
                    state = MENU;
                    return;
                }
                if (e.key.code == Keyboard::Num2) {
                    playerMode = TWO_PLAYER;
                    state = MENU;
                    return;
                }
            }
        }
    }
}

void showDifficultyMenu(RenderWindow& window, Font& font, int& difficulty);
void readScores(ScoreEntry scores[5]);
void saveScores(ScoreEntry scores[5]);
bool updateScoreboard(ScoreEntry scores[5], int newScore, int newTime);
void showScoreboard(RenderWindow& window, Font& font, int& state);

void resetGame(Player& player1, Player& player2, bool& Game, int grid[M][N], Enemy a[], int& enemyCount, int difficulty, float& totalElapsedTime, float& enemySpeedMultiplier, bool& patternsSwitched, float& enemySpawnTimer)
{
    totalElapsedTime = 0;          
    enemySpeedMultiplier = 1.0f;    
    patternsSwitched = false;  
    enemySpawnTimer = 0;

    if (difficulty == CONTINUOUS) {
        enemyCount = 2;
    }

    player1.reset(10, 0);
    player2.reset(N-10, 0);
   
    player1.rewardBonusCount = 0;
    player2.rewardBonusCount = 0;
   
    Game = true;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            grid[i][j] = (i == 0 || j == 0 || i == M - 1 || j == N - 1) ? 1 : 0;

    for (int i = 0; i < enemyCount; i++) {
        a[i] = Enemy(); // reinitialize enemies
        // Reset enemy patterns back to default
        a[i].pattern = LINEAR;
    }
}

void showMenu(RenderWindow& window, Font& font, int& state, int& difficulty, int& playerMode) {
    Text title;
    if (playerMode == SINGLE_PLAYER) {
        title.setString("Xonix Game - Single Player Mode");
    } else {
        title.setString("Xonix Game - 2 Player Mode");
    }
    title.setFont(font);
    title.setCharacterSize(30);
    title.setPosition(150, 100);
    title.setFillColor(Color::Yellow);

    Text prompt("Press ENTER to Start", font, 30);
    prompt.setPosition(250, 200);
    prompt.setFillColor(Color::White);
   
    Text prompt1("Press L to Select Level", font, 30);
    prompt1.setPosition(250, 250);
    prompt1.setFillColor(Color::White);
   
    Text prompt2("Press S for Scoreboard", font, 30);
    prompt2.setPosition(250, 300);
    prompt2.setFillColor(Color::White);

    Text controls;
if (playerMode == SINGLE_PLAYER) {
    controls.setString("Controls: Arrow Keys");
} else {
    controls.setString("Player 1: Arrow Keys | Player 2: WASD");
}
controls.setFont(font);
controls.setCharacterSize(25);
controls.setPosition(200, 350);
controls.setFillColor(Color::Cyan);

    window.clear(Color::Black);
    window.draw(title);
    window.draw(prompt);
    window.draw(prompt1);
    window.draw(prompt2);
    window.draw(controls);
    window.display();

    while (window.isOpen() && state == MENU) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Enter)
                    state = PLAYING;
                if (e.key.code == Keyboard::L)
                    showDifficultyMenu(window, font, difficulty);
                if (e.key.code == Keyboard::S)
                    state = SCOREBOARD;
            }
        }
    }
}

void showDifficultyMenu(RenderWindow& window, Font& font, int& difficulty) {
    Text title("Select Difficulty", font, 40);
    title.setPosition(220, 100);
    title.setFillColor(Color::Yellow);

    Text opt1("1. Easy (2 Enemies)", font, 25);
    opt1.setPosition(220, 180);
    opt1.setFillColor(Color::White);

    Text opt2("2. Medium (4 Enemies)", font, 25);
    opt2.setPosition(220, 220);
    opt2.setFillColor(Color::White);

    Text opt3("3. Hard (6 Enemies)", font, 25);
    opt3.setPosition(220, 260);
    opt3.setFillColor(Color::White);

    Text opt4("4. Continuous Mode", font, 25);
    opt4.setPosition(220, 300);
    opt4.setFillColor(Color::White);

    window.clear(Color::Black);
    window.draw(title);
    window.draw(opt1);
    window.draw(opt2);
    window.draw(opt3);
    window.draw(opt4);
    window.display();

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Num1) { difficulty = EASY; return; }
                if (e.key.code == Keyboard::Num2) { difficulty = MEDIUM; return; }
                if (e.key.code == Keyboard::Num3) { difficulty = HARD; return; }
                if (e.key.code == Keyboard::Num4) { difficulty = CONTINUOUS; return; }
            }
        }
    }
}

// Function to read scores from file
void readScores(ScoreEntry scores[5]) {
    FILE* file = fopen("scores.txt", "r");
   
    
    for (int i = 0; i < 5; i++) {
        scores[i].score = 0;
        scores[i].timeTaken = 0;
    }
   
    if (file) {
        for (int i = 0; i < 5; i++) {
            if (fscanf(file, "%d %d", &scores[i].score, &scores[i].timeTaken) != 2) {
                break; 
            }
        }
        fclose(file);
    }
}


void saveScores(ScoreEntry scores[5]) {
    FILE* file = fopen("scores.txt", "w");
    if (file) {
        for (int i = 0; i < 5; i++) {
            fprintf(file, "%d %d\n", scores[i].score, scores[i].timeTaken);
        }
        fclose(file);
    }
}


bool updateScoreboard(ScoreEntry scores[5], int newScore, int newTime) {
    bool isHighScore = false;
   
   
    for (int i = 0; i < 5; i++) {
        if (newScore > scores[i].score) {
            isHighScore = true;
           
            
            for (int j = 4; j > i; j--) {
                scores[j] = scores[j-1];
            }
           
            scores[i].score = newScore;
            scores[i].timeTaken = newTime;
            break;
        }
    }
   
    if (isHighScore) {
        saveScores(scores);
    }
   
    return isHighScore;
}

void showEndMenu(RenderWindow& window, Font& font, Player& player1, Player& player2, int& highScore, int& state, int totalTimeSeconds) {
    
    ScoreEntry scores[5];
    readScores(scores);
   
   
    int winnerScore = max(player1.score, player2.score);
    string winner = (player1.score > player2.score) ? "Player 1" :
                   (player2.score > player1.score) ? "Player 2" : "Draw";
   
   
    bool isNewHighScoreEntry = updateScoreboard(scores, winnerScore, totalTimeSeconds);
   
    
    bool isNewHighScore = winnerScore > highScore;
    if (isNewHighScore) highScore = winnerScore;

    Text title("Game Over", font, 50);
    title.setPosition(250, 80);
    title.setFillColor(Color::Red);

    Text winnerText;
    if (winner == "Draw") {
        winnerText.setString("It's a Draw!");
    } else {
        winnerText.setString("Winner: " + winner);
    }
    winnerText.setFont(font);
    winnerText.setCharacterSize(30);
    winnerText.setPosition(250, 140);
    winnerText.setFillColor(Color::Green);

    Text score1("Player 1 Score: " + to_string(player1.score), font, 25);
    score1.setPosition(250, 180);
    score1.setFillColor(Color::White);

    Text score2("Player 2 Score: " + to_string(player2.score), font, 25);
    score2.setPosition(250, 210);
    score2.setFillColor(Color::White);
   
    // Display time taken
    int minutes = totalTimeSeconds / 60;
    int seconds = totalTimeSeconds % 60;
    Text timeText("Time: " + to_string(minutes) + "m " + to_string(seconds) + "s", font, 25);
    timeText.setPosition(250, 240);
    timeText.setFillColor(Color::White);

    Text high;
    if (isNewHighScoreEntry) {
        high.setString("New High Score Entry!");
        high.setFillColor(Color::Green);
    } else if (isNewHighScore) {
        high.setString("New High Score: " + to_string(highScore));
        high.setFillColor(Color::Yellow);
    } else {
        high.setString("High Score: " + to_string(highScore));
        high.setFillColor(Color::Yellow);
    }
    high.setFont(font);
    high.setCharacterSize(25);
    high.setPosition(250, 270);

    Text option1("Press R to Restart", font, 25);
    option1.setPosition(250, 310);
    option1.setFillColor(Color::White);

    Text option2("Press M for Main Menu", font, 25);
    option2.setPosition(250, 340);
    option2.setFillColor(Color::White);
   
    Text option3("Press S for Scoreboard", font, 25);
    option3.setPosition(250, 370);
    option3.setFillColor(Color::White);

    Text option4("Press ESC to Exit", font, 25);
    option4.setPosition(250, 400);
    option4.setFillColor(Color::White);

    while (window.isOpen() && state == GAMEOVER) {
        window.clear(Color::Black);
        window.draw(title);
        window.draw(winnerText);
        window.draw(score1);
        window.draw(score2);
        window.draw(timeText);
        window.draw(high);
        window.draw(option1);
        window.draw(option2);
        window.draw(option3);
        window.draw(option4);
        window.display();

        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::R)
                    state = PLAYING;
                else if (e.key.code == Keyboard::M)
                    state = MENU;
                else if (e.key.code == Keyboard::S)
                    state = SCOREBOARD;
                else if (e.key.code == Keyboard::Escape)
                    window.close();
            }
        }
    }
}

void showGameOver(RenderWindow& window, Sprite& sGameover, int& state) {
    window.draw(sGameover);
    window.display();

    while (window.isOpen() && state == GAMEOVER) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::R)
                state = MENU;
        }
    }
}


void showScoreboard(RenderWindow& window, Font& font, int& state) {
    ScoreEntry scores[5];
    readScores(scores);
   
    Text title("High Scores", font, 40);
    title.setPosition(250, 80);
    title.setFillColor(Color::Yellow);
   
    Text scoreTexts[5];
    for (int i = 0; i < 5; i++) {
        string scoreStr;
        if (scores[i].score > 0) {
            int minutes = scores[i].timeTaken / 60;
            int seconds = scores[i].timeTaken % 60;
            scoreStr = to_string(i+1) + ". Score: " + to_string(scores[i].score) +
                      " | Time: " + to_string(minutes) + "m " + to_string(seconds) + "s";
        } else {
            scoreStr = to_string(i+1) + ". ---";
        }
       
        scoreTexts[i].setFont(font);
        scoreTexts[i].setString(scoreStr);
        scoreTexts[i].setCharacterSize(25);
        scoreTexts[i].setFillColor(Color::White);
        scoreTexts[i].setPosition(150, 160 + i * 40);
    }
   
    Text backPrompt("Press B to return to menu", font, 20);
    backPrompt.setPosition(220, 400);
    backPrompt.setFillColor(Color::White);
   
    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::B) {
                    state = MENU;
                    return;
                }
            }
        }
       
        window.clear(Color::Black);
        window.draw(title);
        for (int i = 0; i < 5; i++) {
            window.draw(scoreTexts[i]);
        }
        window.draw(backPrompt);
        window.display();
    }
}


int main()
{
    srand(time(0));
    int state = MODE_SELECT;
    int difficulty = EASY;
    int playerMode = TWO_PLAYER;

    Font font;
    font.loadFromFile("images/arial.ttf");
    

sf::SoundBuffer captureSoundBuffer, bonusSoundBuffer, powerUpSoundBuffer, freezeSoundBuffer;
sf::Sound captureSound, bonusSound, powerUpSound, freezeSound;

captureSoundBuffer.loadFromFile("sounds/coin.wav");
bonusSoundBuffer.loadFromFile("sounds/bonus.wav");
powerUpSoundBuffer.loadFromFile("sounds/powerup.wav");
freezeSoundBuffer.loadFromFile("sounds/freeze.wav");


captureSound.setBuffer(captureSoundBuffer);
bonusSound.setBuffer(bonusSoundBuffer);
powerUpSound.setBuffer(powerUpSoundBuffer);
freezeSound.setBuffer(freezeSoundBuffer);

RenderWindow window(VideoMode(N*ts, M*ts), "Xonix Game - 2 Player Mode!");

    
    window.setFramerateLimit(60);

    Texture t1, t2, t3, t4;
    t1.loadFromFile("images/tiles.png");
    t2.loadFromFile("images/gameover.png");
    t3.loadFromFile("images/enemy.png");
    // You might need to create an additional tile image for player 2
    // t4.loadFromFile("images/tiles_p2.png");

    Sprite sTile(t1), sGameover(t2), sEnemy(t3), sTileP2(t1);
    sGameover.setPosition(100, 100);
    sEnemy.setOrigin(20, 20);

    int highScore = 0;
   
   
    Player player1(10, 0, Color::Blue, 2);   // Player 1 starts from left
    Player player2(N-10, 0, Color::Green, 3); // Player 2 starts from right

    Enemy a[10];
    int currentEnemyCount = 2;

    bool Game = true;
    float timer = 0, delay = 0.07;
    float enemySpawnTimer = 0; // Track enemy increase timing
    float totalElapsedTime = 0;  // Total elapsed time since game started
    float enemySpeedMultiplier = 1.0f;  // Speed multiplier for enemy movement
    Clock clock;
    static bool patternsSwitched = false;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (i == 0 || j == 0 || i == M - 1 || j == N - 1)  grid[i][j] = 1;

    while (window.isOpen())
    {
    if (state == MODE_SELECT) {
        showModeSelection(window, font, state, playerMode);
        continue;
       }
        if (state == MENU) {
            showMenu(window, font, state, difficulty,playerMode);
       
            if (state == SCOREBOARD) {
                showScoreboard(window, font, state);
                continue;
            }

            // Set enemy count based on difficulty
            switch (difficulty) {
                case EASY:
                    currentEnemyCount = 2;
                    break;
                case MEDIUM:
                    currentEnemyCount = 4;
                    break;
                case HARD:
                    currentEnemyCount = 6;
                    break;
                case CONTINUOUS:
                    currentEnemyCount = 2;
                    break;
            }
       
            resetGame(player1, player2, Game, grid, a, currentEnemyCount, difficulty, totalElapsedTime, enemySpeedMultiplier, patternsSwitched, enemySpawnTimer);
            clock.restart();
            continue;
        }

        float time = clock.getElapsedTime().asSeconds();
        clock.restart();
        timer += time;
       
        if (state == PLAYING) {
            totalElapsedTime += time;  // Track total game time
        }

        // Handle power-up timers for both players
        if (player1.powerUpActive) {
            player1.powerUpTimer += time;
            if (player1.powerUpTimer >= 3.0f) {
                player1.powerUpActive = false;  // Unfreeze after 3 seconds
            }
        }

        if (player2.powerUpActive) {
            player2.powerUpTimer += time;
            if (player2.powerUpTimer >= 3.0f) {
                player2.powerUpActive = false;  // Unfreeze after 3 seconds
            }
        }
       
        
        if (!patternsSwitched && totalElapsedTime >= 30.0f) {
            for (int i = 0; i < currentEnemyCount / 2; i++) {
                if (i % 2 == 0)
                    a[i].pattern = ZIGZAG;
                else
                    a[i].pattern = CIRCULAR;
            }
            patternsSwitched = true;
        }

      
        if ((int)totalElapsedTime % 20 == 0 && totalElapsedTime > 0 && (int)(totalElapsedTime - time) % 20 != 0) {
            enemySpeedMultiplier += 0.4f;
        }

        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();
        }

    
       if (!player1.isFrozen) {  // Only allow movement if not frozen
    if (Keyboard::isKeyPressed(Keyboard::Left)) { player1.dx = -1; player1.dy = 0; }
    if (Keyboard::isKeyPressed(Keyboard::Right)) { player1.dx = 1; player1.dy = 0; }
    if (Keyboard::isKeyPressed(Keyboard::Up)) { player1.dx = 0; player1.dy = -1; }
    if (Keyboard::isKeyPressed(Keyboard::Down)) { player1.dx = 0; player1.dy = 1; }
}
if (Keyboard::isKeyPressed(Keyboard::RControl) && player1.powerUpCount > 0 && !player1.powerUpActive) {
    player1.powerUpActive = true;
    player1.powerUpTimer = 0.0f;
    player1.powerUpCount--;
   
 
    powerUpSound.play();
   
   
    if (playerMode == TWO_PLAYER) {
        player2.isFrozen = true;
        player2.frozenTimer = 0.0f;
        
      
        freezeSound.play();
    }
}
     
        if (!player2.isFrozen) {  // Only allow movement if not frozen
    if (Keyboard::isKeyPressed(Keyboard::A)) { player2.dx = -1; player2.dy = 0; }
    if (Keyboard::isKeyPressed(Keyboard::D)) { player2.dx = 1; player2.dy = 0; }
    if (Keyboard::isKeyPressed(Keyboard::W)) { player2.dx = 0; player2.dy = -1; }
    if (Keyboard::isKeyPressed(Keyboard::S)) { player2.dx = 0; player2.dy = 1; }
}
if (Keyboard::isKeyPressed(Keyboard::LControl) && player2.powerUpCount > 0 && !player2.powerUpActive) {
    player2.powerUpActive = true;
    player2.powerUpTimer = 0.0f;
    player2.powerUpCount--;
    
  
    powerUpSound.play();
   

    player1.isFrozen = true;
    player1.frozenTimer = 0.0f;
    
  
    freezeSound.play();
}
       
        if (!Game) {
            state = GAMEOVER;
            showEndMenu(window, font, player1, player2, highScore, state, (int)totalElapsedTime);

            if (state == PLAYING) {
                resetGame(player1, player2, Game, grid, a, currentEnemyCount, difficulty, totalElapsedTime, enemySpeedMultiplier, patternsSwitched, enemySpawnTimer);
                clock.restart();
                continue;
            }

            if (state == MENU) {
                continue; // Main menu will handle the reset
            }
           
            if (state == SCOREBOARD) {
                showScoreboard(window, font, state);
                continue; // Will show the scoreboard
            }

            continue;
        }

     if (timer > delay)
{
   
    if (!player1.isFrozen) {
        player1.x += player1.dx;
        player1.y += player1.dy;

        if (player1.x < 0) player1.x = 0; if (player1.x > N-1) player1.x = N-1;
        if (player1.y < 0) player1.y = 0; if (player1.y > M-1) player1.y = M-1;
           
        trackMoves(player1.x, player1.y, grid, player1.isBuilding, player1.moveCount);

        if (grid[player1.y][player1.x] == 2 || grid[player1.y][player1.x] == 3) Game = false;
        if (grid[player1.y][player1.x] == 0) grid[player1.y][player1.x] = player1.tileValue;
    }
   
    if (grid[player1.y][player1.x] == player1.tileValue && (player1.dx != 0 || player1.dy != 0)) {
    player1.score += 1; // Add 1 point for each tile in the trail
}


    if (playerMode == TWO_PLAYER && !player2.isFrozen) {
        // Update Player 2
        player2.x += player2.dx;
        player2.y += player2.dy;

        if (player2.x < 0) player2.x = 0; if (player2.x > N-1) player2.x = N-1;
        if (player2.y < 0) player2.y = 0; if (player2.y > M-1) player2.y = M-1;
           
        trackMoves(player2.x, player2.y, grid, player2.isBuilding, player2.moveCount);

        if (grid[player2.y][player2.x] == 2 || grid[player2.y][player2.x] == 3) Game = false;
        if (grid[player2.y][player2.x] == 0) grid[player2.y][player2.x] = player2.tileValue;
    }
    if (playerMode == TWO_PLAYER && grid[player2.y][player2.x] == player2.tileValue && (player2.dx != 0 || player2.dy != 0)) {
    player2.score += 1; // Add 1 point for each tile in the trail
}
   
    timer = 0;
}

        
        if (difficulty == CONTINUOUS) {
            enemySpawnTimer += time;
            if (enemySpawnTimer >= 20.0f && currentEnemyCount < 10) {
                int toAdd = min(2, 10 - currentEnemyCount);
                for (int i = currentEnemyCount; i < currentEnemyCount + toAdd; i++) {
                    a[i] = Enemy(); // Add new enemy
                }
                currentEnemyCount += toAdd;
                enemySpawnTimer = 0;
            }
        }

    
        if (!player1.powerUpActive && !player2.powerUpActive) {
            for (int i = 0; i < currentEnemyCount; i++) {
                for (int s = 0; s < (int)enemySpeedMultiplier; s++)
                    a[i].move();
            }
        }
        if (player1.isFrozen) {
    player1.frozenTimer += time;
    if (player1.frozenTimer >= 3.0f) {
        player1.isFrozen = false;  // Unfreeze after 3 seconds
    }
}

if (player2.isFrozen) {
    player2.frozenTimer += time;
    if (player2.frozenTimer >= 3.0f) {
        player2.isFrozen = false;  // Unfreeze after 3 seconds
    }
}
       
       
      
     if (grid[player1.y][player1.x] == 1)
{
    player1.dx = player1.dy = 0;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j] == 2) // Player 1 trail
                grid[i][j] = 1;
               
    for (int i = 0; i < currentEnemyCount; i++) {
        int gx = a[i].x / ts;
        int gy = a[i].y / ts;

        if (gx >= 0 && gx < N && gy >= 0 && gy < M)
            drop(gy, gx);  // Only call drop if within bounds
    }

    int enclosedTiles = 0;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == -1)
                grid[i][j] = 0;
            else {
                if (grid[i][j] == 0) {  
                    enclosedTiles++;   
                    grid[i][j] = 1;
                }
            }
        }

   
    int bonusMultiplier = 1;
   
    
    if (enclosedTiles > 10) {
        
        if (player1.rewardBonusCount >= 5) {
            bonusMultiplier = 4;
        }
        // After 3 occurrences, the threshold reduces to 5 tiles for double points
        else if (player1.rewardBonusCount >= 3) {
            bonusMultiplier = 2;
        }
        
        else {
            bonusMultiplier = 2;
        }
       
        // Increment the reward counter when a bonus is earned
        player1.rewardBonusCount++;
    }
    // Check for reduced threshold after 3 bonus occurrences
    else if (enclosedTiles > 5 && player1.rewardBonusCount >= 3) {
        // After 5 occurrences, capturing more than 5 tiles yields ×4 points
        if (player1.rewardBonusCount >= 5) {
            bonusMultiplier = 4;
        } else {
            bonusMultiplier = 2;
        }
        player1.rewardBonusCount++;
    }
   
    
    player1.score += enclosedTiles * bonusMultiplier;
    
  
if (enclosedTiles > 0) {
    captureSound.play();
    
   
    if (bonusMultiplier > 1) {
        bonusSound.play();
    }
}
   
 
   // Power-up granting for player 1
int powerUpThresholds[] = {50, 70, 100, 130};
int numFixedThresholds = 4;
   
// Check fixed thresholds (50, 70, 100, 130)
for (int i = 0; i < numFixedThresholds; i++) {
    if (player1.score >= powerUpThresholds[i] && player1.lastPowerUpScore < powerUpThresholds[i]) {
        player1.powerUpCount++;
    }
}
   
// Check for additional thresholds (every 30 points after 130)
int lastDynamicThreshold = 130;
while (lastDynamicThreshold + 30 <= player1.score) {
    lastDynamicThreshold += 30;
    if (player1.lastPowerUpScore < lastDynamicThreshold) {
        player1.powerUpCount++;
    }
}
   
player1.lastPowerUpScore = player1.score;
}


if (playerMode == TWO_PLAYER && grid[player2.y][player2.x] == 1)
{
    player2.dx = player2.dy = 0;
   
    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++)
            if (grid[i][j] == 3) // Player 2 trail
                grid[i][j] = 1;

    for (int i = 0; i < currentEnemyCount; i++) {
        int gx = a[i].x / ts;
        int gy = a[i].y / ts;

        if (gx >= 0 && gx < N && gy >= 0 && gy < M)
            drop(gy, gx);  // Only call drop if within bounds
    }

    int enclosedTiles = 0;

    for (int i = 0; i < M; i++)
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == -1)
                grid[i][j] = 0;
            else {
                if (grid[i][j] == 0) {  // if it was 0, now becomes 1
                    enclosedTiles++;   // count this as part of the enclosed region
                    grid[i][j] = 1;
                }
            }
        }

   
    int bonusMultiplier = 1;
   
    
    if (enclosedTiles > 10) {
        
        if (player2.rewardBonusCount >= 5) {
            bonusMultiplier = 4;
        }
        
        else if (player2.rewardBonusCount >= 3) {
            bonusMultiplier = 2;
        }
        
        else {
            bonusMultiplier = 2;
        }
       
        player2.rewardBonusCount++;
    }
   
    else if (enclosedTiles > 5 && player2.rewardBonusCount >= 3) {
        // After 5 occurrences, capturing more than 5 tiles yields ×4 points
        if (player2.rewardBonusCount >= 5) {
            bonusMultiplier = 4;
        } else {
            bonusMultiplier = 2;
        }
        player2.rewardBonusCount++;
    }
   
   
    player2.score += enclosedTiles * bonusMultiplier;
    
    
if (enclosedTiles > 0) {
    captureSound.play();
    
   
    if (bonusMultiplier > 1) {
        bonusSound.play();
    }
}
   
int powerUpThresholds[] = {50, 70, 100, 130};
int numFixedThresholds = 4;
   
for (int i = 0; i < numFixedThresholds; i++) {
    if (player2.score >= powerUpThresholds[i] && player2.lastPowerUpScore < powerUpThresholds[i]) {
        player2.powerUpCount++;
    }
}
   

int lastDynamicThreshold = 130;
while (lastDynamicThreshold + 30 <= player2.score) {
    lastDynamicThreshold += 30;
    if (player2.lastPowerUpScore < lastDynamicThreshold) {
        player2.powerUpCount++;
    }
}
   
player2.lastPowerUpScore = player2.score;
}

        
     
for (int i = 0; i < currentEnemyCount; i++) {
    int ey = a[i].y / ts;
    int ex = a[i].x / ts;
   
    if (ey >= 0 && ey < M && ex >= 0 && ex < N) {
        // Only kill player 1 if they're moving (building a trail)
        if (grid[ey][ex] == 2 && (player1.dx != 0 || player1.dy != 0)) Game = false;  
       
        // Only kill player 2 if they're moving (building a trail) - and only in two-player mode
        if (playerMode == TWO_PLAYER && grid[ey][ex] == 3 && (player2.dx != 0 || player2.dy != 0)) Game = false;
    }
}

        
if (player1.powerUpActive) {
    window.clear(sf::Color(25, 0, 51)); 
} else if (player2.powerUpActive) {
    window.clear(sf::Color(0, 51, 25)); 
} else {
    window.clear(sf::Color::Black); 
}

        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; j++)
            {
                if (grid[i][j] == 0) continue;
               
                if (grid[i][j] == 1) sTile.setTextureRect(IntRect(0, 0, ts, ts));
                if (grid[i][j] == 2) {  // Player 1 trail
                    sTile.setTextureRect(IntRect(54, 0, ts, ts));
                    sTile.setColor(player1.color);
                }
                if (grid[i][j] == 3) {  // Player 2 trail
                    sTile.setTextureRect(IntRect(54, 0, ts, ts));
                    sTile.setColor(player2.color);
                }
               
                sTile.setPosition(j * ts, i * ts);
                window.draw(sTile);
               
               //RESET COLOR TO DEFAULT
                sTile.setColor(Color::White);
            }

sTile.setTextureRect(IntRect(36, 0, ts, ts));
sTile.setColor(player1.color);
sTile.setPosition(player1.x * ts, player1.y * ts);
window.draw(sTile);
sTile.setColor(Color::White);

if (player1.isFrozen) {
    CircleShape freezeEffect(ts/2);
    freezeEffect.setFillColor(Color(0, 191, 255, 120));  // Light blue, semi-transparent
    freezeEffect.setPosition(player1.x * ts, player1.y * ts);
    window.draw(freezeEffect);
   
    Text frozenText;
    frozenText.setFont(font);
    frozenText.setCharacterSize(14);
    frozenText.setFillColor(Color::Cyan);
    frozenText.setString("FROZEN!");
    frozenText.setPosition(10, 130);
    window.draw(frozenText);
}


if (playerMode == TWO_PLAYER) {
    sTile.setTextureRect(IntRect(36, 0, ts, ts));
    sTile.setColor(player2.color);
    sTile.setPosition(player2.x * ts, player2.y * ts);
    window.draw(sTile);
    sTile.setColor(Color::White);
}
if (playerMode == TWO_PLAYER && player2.isFrozen) {
    CircleShape freezeEffect(ts/2);
    freezeEffect.setFillColor(Color(0, 191, 255, 120));  // Light blue, semi-transparent
    freezeEffect.setPosition(player2.x * ts, player2.y * ts);
    window.draw(freezeEffect);
   
    Text frozenText;
    frozenText.setFont(font);
    frozenText.setCharacterSize(14);
    frozenText.setFillColor(Color::Cyan);
    frozenText.setString("FROZEN!");
    frozenText.setPosition(N*ts - 150, 130);
    window.draw(frozenText);
}


Text p1Title;
p1Title.setFont(font);
p1Title.setCharacterSize(16);
p1Title.setFillColor(player1.color);
p1Title.setString("Player 1");
p1Title.setPosition(10, 10);
window.draw(p1Title);

Text p1ScoreText;
p1ScoreText.setFont(font);
p1ScoreText.setCharacterSize(16);
p1ScoreText.setFillColor(Color::White);
if (playerMode == SINGLE_PLAYER) {
    p1ScoreText.setString("Score: " + to_string(player1.score));}
    else {
    p1ScoreText.setString("Score: " + to_string(player2.score));}
   

p1ScoreText.setPosition(10, 30);
window.draw(p1ScoreText);

Text p1MoveText;
p1MoveText.setFont(font);
p1MoveText.setCharacterSize(16);
p1MoveText.setFillColor(Color::White);
p1MoveText.setString("Moves: " + to_string(player1.moveCount));
p1MoveText.setPosition(10, 50);
window.draw(p1MoveText);

Text p1PowerUpText;
p1PowerUpText.setFont(font);
p1PowerUpText.setCharacterSize(16);
p1PowerUpText.setFillColor(Color::Cyan);
if (playerMode == SINGLE_PLAYER) {
   
    p1PowerUpText.setString("Power-Ups: " + to_string(player1.powerUpCount));
   
} else {
   
    p1PowerUpText.setString("Power-Ups: " + to_string(player2.powerUpCount));
   
}

p1PowerUpText.setPosition(10, 70);
window.draw(p1PowerUpText);

Text p1BonusText;
p1BonusText.setFont(font);
p1BonusText.setCharacterSize(16);
p1BonusText.setFillColor(Color::Yellow);
if (playerMode == SINGLE_PLAYER) {
 
    p1BonusText.setString("Bonus Count: " + to_string(player1.rewardBonusCount));
} else {
   
    p1BonusText.setString("Bonus Count: " + to_string(player2.rewardBonusCount));}
p1BonusText.setPosition(10, 90);
window.draw(p1BonusText);




if (playerMode == TWO_PLAYER) {
    // Display game info - right column (Player 2)
    Text p2Title;
    p2Title.setFont(font);
    p2Title.setCharacterSize(16);
    p2Title.setFillColor(player2.color);
    p2Title.setString("Player 2");
    p2Title.setPosition(N*ts - 100, 10);
    window.draw(p2Title);
   
    Text p2ScoreText;
    p2ScoreText.setFont(font);
    p2ScoreText.setCharacterSize(16);
    p2ScoreText.setFillColor(Color::White);
    p2ScoreText.setString("Score: " + to_string(player1.score));
    p2ScoreText.setPosition(N*ts - 100, 30);
    window.draw(p2ScoreText);

    Text p2MoveText;
    p2MoveText.setFont(font);
    p2MoveText.setCharacterSize(16);
    p2MoveText.setFillColor(Color::White);
    p2MoveText.setString("Moves: " + to_string(player2.moveCount));
    p2MoveText.setPosition(N*ts - 100, 50);
    window.draw(p2MoveText);

    Text p2PowerUpText;
    p2PowerUpText.setFont(font);
    p2PowerUpText.setCharacterSize(16);
    p2PowerUpText.setFillColor(Color::Cyan);
    p2PowerUpText.setString("Power-Ups: " + to_string(player1.powerUpCount));
    p2PowerUpText.setPosition(N*ts - 100, 70);
    window.draw(p2PowerUpText);
   
    Text p2BonusText;
    p2BonusText.setFont(font);
    p2BonusText.setCharacterSize(16);
    p2BonusText.setFillColor(Color::Yellow);
    p2BonusText.setString("Bonus Count: " + to_string(player1.rewardBonusCount));
    p2BonusText.setPosition(N*ts - 110, 90);
    window.draw(p2BonusText);
}

        sEnemy.rotate(10);
        for (int i = 0; i < currentEnemyCount; i++) {
            // Color enemies red if player 1's power-up is active
            if (player1.powerUpActive) {
                sEnemy.setColor(Color(255, 0, 0, 128)); // Semi-transparent red
            }
            // Color enemies yellow if player 2's power-up is active
            else if (player2.powerUpActive) {
                sEnemy.setColor(Color(255, 255, 0, 128)); // Semi-transparent yellow
            }
            else {
                sEnemy.setColor(Color::White);
            }
           
            sEnemy.setPosition(a[i].x, a[i].y);
            window.draw(sEnemy);
        }
       
       
        sEnemy.setColor(Color::White);

       

        
       Text timeText;
timeText.setFont(font);
timeText.setCharacterSize(16);
timeText.setFillColor(Color::White);
int minutes = (int)totalElapsedTime / 60;
int seconds = (int)totalElapsedTime % 60;
timeText.setString("Time: " + to_string(minutes) + "m " + to_string(seconds) + "s");
timeText.setPosition(N*ts/2 - 60, 10);
window.draw(timeText);

Text modeText;
modeText.setFont(font);
modeText.setCharacterSize(16);
modeText.setFillColor(Color::White);
string mode = (difficulty == EASY) ? "Easy" :
             (difficulty == MEDIUM) ? "Medium" :
             (difficulty == HARD) ? "Hard" : "Continuous";
modeText.setString("Mode: " + mode);
modeText.setPosition(N*ts/2 - 60, 30);
window.draw(modeText);

Text enemyText;
enemyText.setFont(font);
enemyText.setCharacterSize(16);
enemyText.setFillColor(Color::Red);
enemyText.setString("Enemies: " + to_string(currentEnemyCount));
enemyText.setPosition(N*ts/2 - 60, 50);
window.draw(enemyText);


if (player1.powerUpActive) {
    Text activeText;
    activeText.setFont(font);
    activeText.setCharacterSize(16);
    activeText.setFillColor(Color::Yellow);
    activeText.setString("POWER-UP ACTIVE!");
    activeText.setPosition(10, 110);
    window.draw(activeText);
}

if (playerMode == TWO_PLAYER && player2.powerUpActive) {
    Text activeText;
    activeText.setFont(font);
    activeText.setCharacterSize(16);
    activeText.setFillColor(Color::Yellow);
    activeText.setString("POWER-UP ACTIVE!");
    activeText.setPosition(N*ts - 150, 110);
    window.draw(activeText);
}

        window.display();
    }

    return 0;
}












































































































































































































































































//guuuuuuuuuuuuukoirhhhhhhhhhhhhtjgnv
//fjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjkds
//uhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhewgf
//uuuuuuuuuuuuuuuuuuuuuuuuuugtwe
//guwwwwwwwwwwwwwwwwwwwwwwwwwwwwi





















































































































































































//jgaaaaaaaaaaaaaaaaaaaaaaaaaaar
//rhaioooooooooooooooooooooooooooooool
//hjiooooooooooooooooooooooooooooooooo
//hgarkdkgkhgkuetsssss
//jhtjjjjjjjjjjjjjs














































































































































































































//iraaaaaaaaaaaaaaaaaaaaaaaaag
//raaaaaaaaaaaaaaaaaaag
//roihaeeeeee
//vcxlllllllllll
//wreeewrr
