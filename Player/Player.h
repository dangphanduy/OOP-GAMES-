#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <unordered_set>
#include "Tile.h"
#include "Game.h"

using namespace std;

enum class PlayerState { Normal, Bankrupt };

class Player {
private:
	SDL_Texture* sprite = nullptr;
	float x = 0.0f, y = 0.0f; // Vị trí hiện tại
	float targetX = 0.0f, targetY = 0.0f; // vị trí mục tiêu

	vector<Tile*> ownedProperties;
	string name;
	SDL_Texture* houseTexture;
	vector<int> rollHistory;
	int previousRoll = 0;
	int currentRoll = 0;
	int money;
	unordered_set<Tile*> properties;
	int position = 0;
	PlayerState state = PlayerState::Normal;
	bool isMoving = false;
	bool canRollDice = true; // Kiểm tra xem người chơi có thể lăn xúc xắc không
	int turnsOnLostIsland = 0; // Số lượt trên Lost Island
	bool isOnLostIsland = false;  // Người chơi có đang ở trên Lost Island không
	int worldsUsed = 0;
	bool hasMap = false;

public:
	explicit Player(const string& playerName, int initialMoney);

	// Getters
	SDL_Texture* getSprite() const { return sprite; }
	float getX() const { return x; }
	float getY() const { return y; }
	const string& getName() const { return name; }
	int getMoney() const { return money; }
	int getPosition() const { return position; }
	bool getCanRollDice() const { return canRollDice; }
	int getPreviousRoll() const { return previousRoll; }
	int getCurrentRoll() const { return currentRoll; }
	const unordered_set<Tile*>& getProperties() const { return properties; }
	PlayerState getState() const { return state; }
	bool getIsMoving() const { return isMoving; }
	int getTurnsOnLostIsland() const { return turnsOnLostIsland; }
	bool getIsOnLostIsland() const { return isOnLostIsland; }
	int getWorldsUsed() const { return worldsUsed; }
	bool getHasMap() const { return hasMap; }
	SDL_Texture* getHouseTexture() const { return houseTexture; }
	const vector<Tile*>& getOwnedProperties() const { return ownedProperties; }

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
	void setProperties(const unordered_set<Tile*>& properties) { this->properties = properties; }
	void setState(PlayerState state) { this->state = state; }
	void setIsMoving(bool isMoving) { this->isMoving = isMoving; }
	void setWorldsUsed(int worldsUsed) { this->worldsUsed = worldsUsed; }
	void setTurnsOnLostIsland(int turnsOnLostIsland) { this->turnsOnLostIsland = turnsOnLostIsland; }
	void setIsOnLostIsland(bool isOnLostIsland) { this->isOnLostIsland = isOnLostIsland; }
	void setHasMap(bool hasMap) { this->hasMap = hasMap; }
	void setHouseTexture(SDL_Texture* texture) { this->houseTexture = texture; }

	vector<Tile*> getOwnedProperty() const {
		vector<Tile*> owned;
		for (Tile* tile : properties) {
			if (tile->getOwnerName() == this->name) { // Kiểm tra xem người chơi hiện tại có phải chủ sở hữu không
				owned.push_back(tile);
			}
		}
		return owned;
	}

	// Các phương thức công khai
	void render(SDL_Renderer* renderer);
	int calculateNewPosition(int steps) const;
	void move(int steps, vector<Tile>& board);
	void setTargetPosition(float x, float y);
	void updateTargetPosition();
	void updatePosition(float deltaTime, const vector<Player>& otherPlayers);
	bool hasReachedTarget() const;
	bool canBuyHouse(const Tile& tile) const;
	void addProperty(Tile* tile);
	void clearProperties();
};

#endif