#include "Player.h"
#include "Constants.h"
#include "Game.h"
#include "Board.h"
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
    hasMap(false)
{}

void Player::addProperty(Tile* tile) {
    properties.insert(tile);
}

int Player::countHouses() const {
    int totalHouses = 0;
    for (const auto& property : properties) {
        totalHouses += property->getNumHouses();
    }
    return totalHouses;
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

    // Di chuyển người chơi đến vị trí mới
    int newPosition = calculateNewPosition(steps);
    position = newPosition;
    Tile& targetTile = board[newPosition];

    // Cập nhật vị trí đích và trạng thái di chuyển
    setTargetPosition(targetTile.getX(), targetTile.getY());
    updateTargetPosition();
    isMoving = true; // Đặt isMoving = true sau khi cập nhật vị trí đích

    // Thêm người chơi vào ô mới
    Tile& newTile = board[position];
    newTile.addPlayer(this);
}

void Player::setTargetPosition(float x, float y) {
    targetX = x;
    targetY = y;
    isMoving = true;
}

void Player::updateTargetPosition() {
    if (position < 8) {
        targetX = position * TILE_SIZE;
        targetY = 0;
    }
    else if (position < 16) {
        targetX = SCREEN_WIDTH - TILE_SIZE;
        targetY = (position - 8) * TILE_SIZE;
    }
    else if (position < 24) {
        targetX = SCREEN_WIDTH - (position - 16) * TILE_SIZE - TILE_SIZE;
        targetY = SCREEN_HEIGHT - TILE_SIZE;
    }
    else {
        targetX = 0;
        targetY = SCREEN_HEIGHT - (position - 24) * TILE_SIZE - TILE_SIZE;
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