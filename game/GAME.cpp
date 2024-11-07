#include "Game.h"
#include "Player.h"
#include "Constants.h"
#include "Board.h"
#include "Tile.h"
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
    int startingPositions = 0; // Vị trí bắt đầu

    for (int i = 0; i < players.size(); ++i) {
        // Tải hình ảnh người chơi
        SDL_Texture* playerTexture = loadTexture(playerTextures[i], renderer);
        players[i].setSprite(playerTexture);

        SDL_Texture* playerHouseTexture = loadTexture(houseTextures[i], renderer);
        players[i].setHouseTexture(playerHouseTexture);

        // Thêm người chơi vào ô khởi đầu tương ứng
        board->getBoard()[startingPositions].addPlayer(&players[i]);

        // Cập nhật vị trí của người chơi
        players[i].setPosition(startingPositions);
    }
}

Player& Game::getPlayer(const string& name) {
    for (Player& player : players) {
        if (player.getName() == name) {
            return player;
        }
    }
}

void Game::quitGame() {
    SDL_Quit();
}