#ifndef TILEBUILDER_H
#define TILEBUILDER_H

#include "Tile.h"

class TileBuilder {
public:
    TileBuilder() : tile(Tile()) {}

    TileBuilder& withName(const std::string& name) { tile.name = name; return *this; }
    TileBuilder& withCost(int cost) { tile.housePrice = cost; return *this; }
    TileBuilder& withType(TileType type) { tile.type = type; return *this; }
    TileBuilder& withHouseMax(int houseMax) { tile.maxHouses = houseMax; return *this; }
    TileBuilder& withOnLand(std::function<void(Player*)> onLand) { // Sử dụng con trỏ Player
        tile.onLand = onLand;
        return *this;
    }
    TileBuilder& withColorGroup(ColorGroup colorGroup) { tile.colorGroup = colorGroup; return *this; }
    TileBuilder& withOwnerName(std::string ownerName) { tile.ownerName = ownerName; return *this; } 
    TileBuilder& withPosition(int x, int y) { tile.x = x; tile.y = y; return *this; }

    Tile build() {
        return tile;
    }

private:
    Tile tile;
};

#endif