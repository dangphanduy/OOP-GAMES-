#include "Game/Game.h"
#include "Player/Player.h"
#include "Main/Constants.h"
#include "Board/Board.h"
#include <iostream>
#include <cmath>

Player::Player(const string& playerName, int initialMoney)
    : name(playerName),
    money(initialMoney),
    sprite(nullptr),
    x(0.0f),
    y(0.0f),
    targetX(0.0f),
    targetY(0.0f),
    previousRoll(0),
    currentRoll(0),
    position(0),
    state(PlayerState::Normal),
    isMoving(false),
    canRollDice(true),
    turnsOnLostIsland(0),
    isOnLostIsland(false),
    worldsUsed(0),
    hasMap(false),
    houseTexture(houseTexture)
{}

void Player::render(SDL_Renderer* renderer) {
    // 1. Tạo SDL_Rect để xác định vị trí và kích thước của người chơi trên màn hình
    SDL_Rect renderQuad = {
        static_cast<int>(x),
        static_cast<int>(y),
        spriteWidth,
        spriteHeight
    };

    // 2. Sử dụng SDL_RenderCopy để vẽ texture (hình ảnh) của người chơi lên renderer
    SDL_RenderCopy(renderer, sprite, NULL, &renderQuad);
}

void Player::addProperty(Tile* tile) {
    properties.insert(tile);
}

bool Player::canBuyHouse(const Tile& tile) const {
    return tile.getTileType() == TileType::PROPERTY && tile.getNumHouses() < MAX_HOUSE;
}

int Player::calculateNewPosition(int steps) const {
    return (position + steps + NUM_TILES) % NUM_TILES; // Bàn cờ có 32 ô
}

void Player::move(int steps, vector<Tile>& board) {
    // Lấy ô hiện tại và loại bỏ người chơi khỏi ô đó
    Tile& oldTile = board[position];
    oldTile.removePlayer(this);

    // Tính vị trí mới và cập nhật
    position = calculateNewPosition(steps);
    Tile& targetTile = board[position];

    // Cập nhật vị trí đích và trạng thái di chuyển
    setTargetPosition(static_cast<int>(targetTile.getX()), static_cast<int>(targetTile.getY()));
    isMoving = true;

    // Thêm người chơi vào ô mới
    targetTile.addPlayer(this);
}

void Player::setTargetPosition(float x, float y) {
    targetX = x;
    targetY = y;
}

void Player::updateTargetPosition() {
    int side = position / numTilesPerSide; // Xác định cạnh của bàn cờ (0: trên, 1: phải, 2: dưới, 3: trái)
    int tilePos = position % numTilesPerSide; // Vị trí của ô trên cạnh đó

    switch (side) {
    case 0: // Cạnh trên
        targetX = tilePos * TILE_SIZE;
        targetY = 0;
        break;
    case 1: // Cạnh phải
        targetX = numTilesPerSide * TILE_SIZE;
        targetY = tilePos * TILE_SIZE;
        break;
    case 2: // Cạnh dưới
        targetX = (numTilesPerSide - tilePos - 1) * TILE_SIZE;
        targetY = numTilesPerSide * TILE_SIZE;
        break;
    case 3: // Cạnh trái
        targetX = 0;
        targetY = (numTilesPerSide - tilePos - 1) * TILE_SIZE;
        break;
    }
}

void Player::updatePosition(float deltaTime, const vector<Player>& otherPlayers) {
    if (isMoving) {
        float dx = targetX - x;
        float dy = targetY - y;
        float distance = sqrt(dx * dx + dy * dy);

        if (dx * dx + dy * dy < (PLAYER_SPEED * deltaTime) * (PLAYER_SPEED * deltaTime)) {
            x = targetX;
            y = targetY;
            isMoving = false;

            // Kiểm tra va chạm với người chơi khác và điều chỉnh vị trí nếu cần
            for (const Player& other : otherPlayers) {
                if (&other != this && abs(other.x - this->x) < TILE_SIZE * 0.1f && abs(other.y - this->y) < TILE_SIZE * 0.1f) {
                    this->x += TILE_SIZE * 0.2f; // Tăng khoảng cách x 
                    this->y += TILE_SIZE * 0.2f; // Tăng khoảng cách y
                }
            }
        }
        else {
            x += dx / distance * PLAYER_SPEED * deltaTime;
            y += dy / distance * PLAYER_SPEED * deltaTime;
        }
    }
}

bool Player::hasReachedTarget() const {
    return (x == targetX && y == targetY);
}