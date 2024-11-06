#include "Player.h"
#include "Constants.h"
#include "Game.h"
#include "Board.h"
#include <iostream>
#include <cmath>

Player::Player(const std::string& playerName, int initialMoney)
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
    onWorldTour(false),
    hasMap(false)
{}

void Player::addRoll(int roll) {
    rollHistory.push_back(roll);
}

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

void Player::printRollHistory() const {
    for (int roll : rollHistory) {
        std::cout << roll << " ";
    }
    std::cout << std::endl;
}

int Player::calculateNewPosition(int steps) const {
    return (position + steps + NUM_TILES) % NUM_TILES; // Bàn cờ có 32 ô
}

std::vector<Tile*> Player::getOwnedProperties() const {
    std::vector<Tile*> ownedProperties;
    for (Tile* tile : properties) { // Duyệt qua danh sách properties của người chơi
        if (tile->getOwnerName() == name) { // Kiểm tra nếu người chơi sở hữu ô đất
            ownedProperties.push_back(tile);
        }
    }
    return ownedProperties;
}

void Player::move(int steps, std::vector<Tile>& board){
    if (canRollDice) {
        // Lấy ô hiện tại và loại bỏ người chơi khỏi ô đó
        Tile& oldTile = board[position];
        oldTile.removePlayer(this);

        // Di chuyển người chơi đến vị trí mới
        int newPosition = (position + steps) % board.size();
        position = newPosition;
        Tile& targetTile = board[newPosition];
        setTargetPosition(targetTile.getX(), targetTile.getY());
        updateTargetPosition();
        isMoving = true;
        canRollDice = false;

        // Thêm người chơi vào ô mới
        Tile& newTile = board[position];
        newTile.addPlayer(this);

        // Kích hoạt sự kiện trên ô mới nếu có
        if (board[position].getOnLand()) {
            //board[position].getOnLand()(*this);
        }
    }
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

void Player::updatePosition(float deltaTime, const std::vector<Player>& otherPlayers) {
    if (isMoving) {
        float dx = targetX - x;
        float dy = targetY - y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < PLAYER_SPEED * deltaTime) {
            x = targetX;
            y = targetY;
            isMoving = false;
            canRollDice = true;

            // Kiểm tra va chạm với người chơi khác và điều chỉnh vị trí nếu cần
            for (const Player& other : otherPlayers) {
                
        // Improved collision handling with margin for error
                if (&other != this && std::abs(other.x - this->x) < TILE_SIZE * 0.1f && std::abs(other.y - this->y) < TILE_SIZE * 0.1f) {
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