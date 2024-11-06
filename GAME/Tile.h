#ifndef TILE_H
#define TILE_H

#include <SDL.h>
#include <string>
#include <vector>
#include <functional>
#include <algorithm> 
#include <unordered_set>

class Player;  // Forward declaration

enum class ColorGroup {
    BROWN,
    LIGHT_BLUE,
    PINK,
    ORANGE,
    RED,
    YELLOW,
    GREEN,
    DARK_BLUE,
    NAVAJO_WHITE
};

enum class TileType {
    GO,
    PROPERTY,
    START,
    CHANCE,
    BEACH,
    WORLDS,
    LOST_ISLAND,
    WORLD_TOUR,
    TAX
};

// Class ô cho cờ tỷ phú
class Tile {
public:
    Tile() = default;
    std::string getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }

    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int newX, int newY) { x = newX; y = newY; }

    int getHousePrice() const { return housePrice; }
    void setHousePrice(int price) { housePrice = price; }

    int getNumHouses() const { return numHouses; }
    void setNumHouses(int count) { numHouses = count; }

    TileType getTileType() const { return type; }
    void setTileType(TileType newType) { type = newType; }

    std::string getOwnerName() const { return ownerName; }
    void setOwnerName(const std::string& newOwnerName) { ownerName = newOwnerName; }

    ColorGroup getColorGroup() const { return colorGroup; }
    void setColorGroup(ColorGroup group) { colorGroup = group; }

    int getValueMultiplier() const { return valueMultiplier; }
    void setValueMultiplier(int newValueMultiplier) { valueMultiplier = newValueMultiplier; }

    std::unordered_set<Player*> getPlayersOnTile() const { return playersOnTile; }
    void setPlayersOnTile(std::unordered_set<Player*> players) { playersOnTile = players; }

    std::function<void(Player*)> getOnLand() const { return onLand; }
    void setOnLand(const std::function<void(Player*)>& newOnLand) { onLand = newOnLand; }

    void triggerOnLand(Player* player) {
        if (onLand) {
            onLand(player);
        }
    }

    // Thêm người chơi vào ô
    void addPlayer(Player* player) {
        playersOnTile.insert(player);
    }

    // Xóa người chơi khỏi ô
    void removePlayer(Player* player) {
        playersOnTile.erase(player);
    }
private:
    std::string name;
    std::string ownerName;
    int x, y;
    int housePrice;
    int numHouses;
    int maxHouses;
    int valueMultiplier;
    TileType type;
    ColorGroup colorGroup;
    std::unordered_set<Player*> playersOnTile;
    std::function<void(Player*)> onLand;

    Tile(const std::string& name, int price, int numHouses, TileType t, int maxH,
        std::function<void(Player*)> onLand, int xCoord, int yCoord, ColorGroup group,
        const std::string& ownerName)
        : name(name), ownerName(ownerName), x(xCoord), y(yCoord), housePrice(price), numHouses(numHouses), maxHouses(maxH),
        valueMultiplier(1), type(t), colorGroup(group), onLand(onLand) {}

    friend class TileBuilder;
};

#endif // TILE_H