#include "Game/Game.h"
#include "Player/Player.h"
#include "Main/Constants.h"
#include "Board/Board.h"
#include "Board/Tile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

using namespace std;

Game::Game(const vector<string>& playerNames) : currentPlayerIndex(0) {
    initSDL();
    board = new Board(this); // Khởi tạo Board và truyền con trỏ đến Game
    board->createBoard(); // Gọi phương thức tạo bảng
    for (const auto& name : playerNames) {
        players.emplace_back(name, 1500);
    }
    initPlayers();
    currentPlayerIndex = 0;
    setupChanceEvents();
}

Game::~Game() {
    delete board;
    cleanup();
}

void Game::initPlayers() {
    vector<string> playerTextures = { "Player1.png", "Player2.png", "Player3.png", "Player4.png" };
    vector<string> houseTextures = { HOUSE1_TEXTURE_PATH, HOUSE2_TEXTURE_PATH, HOUSE3_TEXTURE_PATH, HOUSE4_TEXTURE_PATH };
    Tile& startTile = board->getBoard()[0];
    int startX = startTile.getX() + TILE_SIZE / 2;
    int startY = startTile.getY() + TILE_SIZE / 2;

    // Tính toán bán kính vòng tròn để người chơi đứng xung quanh
    int radius = TILE_SIZE / 2 - 20;

    // Chia đều các góc trên vòng tròn cho số lượng người chơi
    float angleIncrement = 2 * M_PI / players.size();

    for (int i = 0; i < players.size(); ++i) {
        // Tải hình ảnh người chơi
        SDL_Texture* playerTexture = loadTexture(playerTextures[i], renderer);
        players[i].setSprite(playerTexture);
        // Tải hình ảnh 4 ngôi nhà cho 4 người chơi
        SDL_Texture* playerHouseTexture = loadTexture(houseTextures[i], renderer);
        players[i].setHouseTexture(playerHouseTexture);

        // Tính toán góc hiện tại
        float angle = i * angleIncrement;

        // Tính toán tọa độ (x, y) của người chơi dựa trên góc và bán kính
        int x = static_cast<int>(startX + radius * cos(angle) - 35 / 2);
        int y = static_cast<int>(startY + radius * sin(angle) - 35 / 2);

        // Đặt vị trí và cập nhật target cho người chơi
        players[i].setX(x);
        players[i].setY(y);
        players[i].setTargetPosition(x, y);

        // Thêm người chơi vào ô bắt đầu
        startTile.addPlayer(&players[i]);
    }
}

Player& Game::getPlayer(const string& name) {
    for (Player& player : players) {
        if (player.getName() == name) {
            return player;
        }
    }
}