#ifndef BOARD_H
#define BOARD_H

#include "Player.h"
#include "Constants.h"
#include "Game.h"
#include "Tile.h"
#include <SDL.h>
#include <vector>
#include <functional>

class Game;

class Board {
public:
    Board(Game* gameInstance); // Constructor 

    // Tạo ra bảng
    void createBoard();

    void calculateTilePosition(int i, int& x, int& y);

    // Tạo ra các ô đặc biệt trên bàn cờ
    void setupSpecialTiles();

    void applyTax(Player& player);

    // Các hàm tạo ra texture
    void renderHouse(SDL_Renderer* renderer, SDL_Texture* houseTexture, int x, int y);
    void renderPlayerAt(Player* player, int x, int y);
    void renderPlayers();
    void renderBoards();

    void handleChanceEvent(Player& player);
    void handleWorldsEvent(Player& player);

    std::vector<Tile> board;
private:
    static const int NUM_SPECIAL_TILES = 4;
    int specialTilePositions[NUM_SPECIAL_TILES] = { 0, 8, 16, 24 };
    Game* gameInstance;
    std::vector<Tile> tiles;
    std::vector<std::function<void(Player&)>> chanceEvents;
};

#endif
