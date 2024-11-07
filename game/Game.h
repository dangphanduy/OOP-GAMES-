#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "Constants.h"
#include "Board.h"
#include "Tile.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <functional>

using namespace std;

class Board;

class Game {
public:
    Game(const vector<string>& playerNames); 
    ~Game();

    void run();
    void initSDL();
    void initPlayers();
    void cleanup();
    void quitGame();

    void handleEvents();
    void handleKeyPress(SDL_Keycode key);
    void update(float deltaTime);

    void render();
    void renderText(const string& message, int x, int y, SDL_Color color);
    void renderPlayerInfo();
    void drawPlayer(SDL_Texture* texture, int x, int y);

    int rollDice();
    void handleRollDice(Player& currentPlayer);
    void nextTurn(); 

    bool hasRolledDoubles(); 
    void setupChanceEvents();

    void auctionProperty(Tile& tile);
    void buyHouse(Tile& tile);
    void buyBeach(Player& player, Tile& tile);

    void sellHouse(Player& player);
    vector<Tile*> getOwnedTilesWithHouses(Player& player);
    void displayOwnedTiles(Player& player, const vector<Tile*>& ownedTiles);
    int getPlayerChoice(size_t numChoices);
    void sellHouseOnTile(Player& player, Tile* tile);
    void handleBankruptcy(Player& player);

    SDL_Texture* loadTexture(const string& path, SDL_Renderer* renderer);

    // Getters
    Player& getPlayer(const string& name);
    vector<Player>& getPlayers() { return players; }
    Board* getBoard() { return board; }
    int getCurrentPlayerIndex() const { return currentPlayerIndex; }
    bool getIsRunning() const { return isRunning; }
    SDL_Renderer* getRenderer() const { return renderer; }
    const vector<function<void(Player&, vector<Player*>&)>> getChanceEvents() const {
        return chanceEvents;
    }

    // Setters
    void setCurrentPlayerIndex(int currentPlayerIndex) { this->currentPlayerIndex = currentPlayerIndex; }
    void setIsRunning(bool isRunning) { this->isRunning = isRunning; }
    void setRenderer(SDL_Renderer* renderer) { this->renderer = renderer; }
    void setChanceEvents(const vector<function<void(Player&, vector<Player*>&)>>& newEvents) {
        this->chanceEvents = newEvents;
    }
    void handleBuyProperty(Player& currentPlayer, Tile& currentTile);
private:
    TTF_Font* font;
    Uint32 turnStartTime; // Biến lưu thời điểm bắt đầu lượt
    bool autoRollDice = false;
    vector<Player> players;
    Board* board;
    int currentPlayerIndex;
    bool isRunning;
    vector<function<void(Player&, vector<Player*>&)>> chanceEvents;
    SDL_Window* window;
    SDL_Renderer* renderer;
};

#endif