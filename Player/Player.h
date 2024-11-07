#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <unordered_set>

#include "Tile.h"

enum class PlayerState { Normal, Bankrupt };

class Player {
private:
	SDL_Texture* sprite = nullptr;
	float x = 0.0f, y = 0.0f; // Vị trí hiện tại
	float targetX = 0.0f, targetY = 0.0f; // vị trí mục tiêu

	std::string name;
	SDL_Texture* houseTexture;
	std::vector<int> rollHistory;
	int previousRoll = 0;
	int currentRoll = 0;
	int money;
	std::unordered_set<Tile*> properties;
	int position = 0;
	PlayerState state = PlayerState::Normal;
	bool isMoving = false;
	bool canRollDice = true; // Kiểm tra xem người chơi có thể lăn xúc xắc không
	int turnsOnLostIsland = 0; // Số lượt trên Lost Island
	bool isOnLostIsland = false;  // Người chơi có đang ở trên Lost Island không
	int worldsUsed = 0;
	bool onWorldTour = false;
	bool hasMap = false;

	bool hasReachedTarget() const;

public:
	explicit Player(const std::string& playerName, int initialMoney);

	// Getters
	SDL_Texture* getSprite() const { return sprite; }
	float getX() const { return x; }
	float getY() const { return y; }
	const std::string& getName() const { return name; }
	int getMoney() const { return money; }
	int getPosition() const { return position; }
	bool getCanRollDice() const { return canRollDice; }
	int getPreviousRoll() const { return previousRoll; }
	int getCurrentRoll() const { return currentRoll; }
	const std::unordered_set<Tile*>& getProperties() const { return properties; }
	PlayerState getState() const { return state; }
	bool getIsMoving() const { return isMoving; }
	int getTurnsOnLostIsland() const { return turnsOnLostIsland; }
	bool getIsOnLostIsland() const { return isOnLostIsland; }
	int getWorldsUsed() const { return worldsUsed; }
	bool getOnWorldTour() const { return onWorldTour; }
	bool getHasMap() const { return hasMap; }
	SDL_Texture* getHouseTexture() const { return houseTexture; }

	// Setters
	void setX(float x) { this->x = x; }
	void setY(float y) { this->y = y; }
	void setTargetX(float targetX) { this->targetX = targetX; }
	void setTargetY(float targetY) { this->targetY = targetY; }
	void setSprite(SDL_Texture* sprite) { this->sprite = sprite; }
	void setMoney(int money) { this->money = money; }
	void setPosition(int position) { this->position = position; }
	void setCanRollDice(bool canRollDice) { this->canRollDice = canRollDice; }
	void setPreviousRoll(int previousRoll) { this->previousRoll = previousRoll; }
	void setCurrentRoll(int currentRoll) { this->currentRoll = currentRoll; }
	void setProperties(const std::unordered_set<Tile*>& properties) { this->properties = properties; }
	void setState(PlayerState state) { this->state = state; }
	void setIsMoving(bool isMoving) { this->isMoving = isMoving; }
	void setOnWorldTour(bool onWorldTour) { this->onWorldTour = onWorldTour; }
	void setWorldsUsed(int worldsUsed) { this->worldsUsed = worldsUsed; }
	void setTurnsOnLostIsland(int turnsOnLostIsland) { this->turnsOnLostIsland = turnsOnLostIsland; }
	void setIsOnLostIsland(bool isOnLostIsland) { this->isOnLostIsland = isOnLostIsland; }
	void setHasMap(bool hasMap) { this->hasMap = hasMap; }
	void setHouseTexture(SDL_Texture* texture) { houseTexture = texture; }

	// Các phương thức công khai
	void addRoll(int roll);
	void printRollHistory() const;
	int calculateNewPosition(int steps) const;
	void move(int steps, std::vector<Tile>& board);
	void setTargetPosition(float x, float y);
	void updateTargetPosition();
	void updatePosition(float deltaTime, const std::vector<Player>& otherPlayers);
	bool canBuyHouse(const Tile& tile) const;
	void addProperty(Tile* tile);
	int countHouses() const;
	std::vector<Tile*> getOwnedProperties() const;
	void clearProperties();
};

#endif
