#ifndef BOARD_H
#define BOARD_H

#include "C:\Users\anhla\source\repos\GAME\GAME\Player\Player.h"
#include "C:\Users\anhla\source\repos\GAME\GAME\Main\Constants.h"
#include "C:\Users\anhla\source\repos\GAME\GAME\Game\Game.h"
#include "Tile.h"
#include <SDL.h>
#include <vector>
#include <functional>

using namespace std;

class Game;

class Board {
public:
    Board(Game* gameInstance);

    // Tạo ra bảng
    void createBoard();

    void calculateTilePosition(int i, int& x, int& y);

    // Tạo ra các ô đặc biệt trên bàn cờ
    void setupSpecialTiles();

    // Các hàm tạo ra texture
    void renderHouse(SDL_Renderer* renderer, SDL_Texture* houseTexture, int x, int y);
    void renderPlayerAt(Player* player, int x, int y);
    void renderPlayers();
    void renderBoards();

    void handleChanceEvent(Player& player);
    void handleWorldsEvent(Player& player);
    void applyTax(Player& player);

    vector<Tile>& getBoard() {
        return board;
    }

    void setBoard(const vector<Tile>& newBoard) {
        board = newBoard;
    }

    vector<Tile>& getTiles() {
        return tiles;
    }

    void setTiles(const vector<Tile>& newTiles) {
        tiles = newTiles;
    }
    
private:
    const int specialTilePositions[12] = { 0, 8, 16, 24, 12, 20, 28, 4, 14, 18, 25, 30 };
    Game* game;
    vector<Tile> tiles;
    vector<Tile> board;
};

#endif
