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

    std::vector<Tile>& getBoard() {
        return board;
    }

    void setBoard(const std::vector<Tile>& newBoard) {
        board = newBoard;
    }
    
private:
    const int specialTilePositions[12] = { 0, 8, 16, 24, 12, 20, 28, 4, 14, 18, 25, 30 };
    Game* game;
    std::vector<Tile> tiles;
    std::vector<Tile> board;
};

#endif
