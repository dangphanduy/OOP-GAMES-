#include <iostream>
#include <string>
#include <algorithm>  
#include <random> 
#include "Player.h"
#include "Board.h"
#include "Constants.h"
#include "Game.h"
#include "TileBuilder.h"

Board::Board(Game* gameInstance) : gameInstance(gameInstance) {}

void Board::handleChanceEvent(Player& player) {
    std::random_device rd;
    std::mt19937 gen{ rd() };
    std::uniform_int_distribution<> dis(0, gameInstance->getChanceEvents().size() - 1);
    int eventIndex = dis(gen);
    gameInstance->getChanceEvents()[eventIndex](player);  // Execute random Chance event
}

void Board::handleWorldsEvent(Player& player) {
    std::cout << player.getName() << " landed on Worlds!" << std::endl;

    // Lấy danh sách các ô đất mà người chơi sở hữu
    const auto ownedProperties = player.getOwnedProperties(); // Sử dụng auto

    // Nếu người chơi không sở hữu ô đất nào
    if (ownedProperties.empty()) {
        std::cout << "You don't own any properties yet." << std::endl;
        return;
    }

    // Hiển thị danh sách các ô đất
    std::cout << "Choose a property to increase its value:" << std::endl;
    for (size_t i = 0; i < ownedProperties.size(); ++i) {
        std::cout << i + 1 << ". " << ownedProperties[i]->getName() << std::endl;
    }

    // Nhận lựa chọn của người chơi
    int choice;
    std::cin >> choice;

    // Xử lý lựa chọn
    if (choice > 0 && choice <= ownedProperties.size()) {
        Tile* selectedTile = ownedProperties[choice - 1];
        selectedTile->setValueMultiplier(selectedTile->getValueMultiplier() * 2); // Tăng hệ số nhân lên gấp đôi
        player.setWorldsUsed(player.getWorldsUsed() + 1); // Tăng số lần sử dụng "Worlds"

        std::cout << "The value of " << selectedTile->getName() << " has been doubled!" << std::endl;
        std::cout << "Multiplier: " << selectedTile->getValueMultiplier() << std::endl;
    }
    else {
        std::cout << "Invalid choice." << std::endl;
    }
}

void Board::applyTax(Player& player) {
    // Tìm ô Tax trên bàn cờ
    Tile* taxTile = nullptr;
    for (Tile& t : board) {
        if (t.getTileType() == TileType::TAX) {
            taxTile = &t;
            break;
        }
    }

    if (taxTile != nullptr) {
        int totalHouseValue = 0;

        // Tính tổng giá trị nhà của người chơi
        for (const Tile* p : player.getProperties()) {
            totalHouseValue += p->getNumHouses() * p->getHousePrice();
        }

        // Tính tiền thuế (10% tổng giá trị nhà)
        int taxAmount = 0.1 * totalHouseValue;
        player.setMoney(player.getMoney() - taxAmount);

        std::cout << player.getName() << " paid $" << taxAmount << " in taxes at "
            << taxTile->getName() << std::endl;
    }
}

void Board::calculateTilePosition(int i, int& x, int& y) {
    switch (i) {
    case 0:  // Góc trên bên trái (START)
        x = 0;
        y = 0;
        break;
    case 8:  // Góc trên bên phải
        x = TILE_SIZE * numTilesPerSide;
        y = 0;
        break;
    case 16: // Góc dưới bên phải
        x = TILE_SIZE * numTilesPerSide;
        y = TILE_SIZE * numTilesPerSide;
        break;
    case 24: // Góc dưới bên trái
        x = 0;
        y = TILE_SIZE * numTilesPerSide;
        break;
    default:
        if (i > 0 && i < 8) { // Các ô trên đường trên (từ ô 1 đến 6)
            x = TILE_SIZE * i;
            y = 0;
        }
        else if (i > 8 && i < 16) { // Các ô trên đường bên phải (từ ô 8 đến 14)
            x = TILE_SIZE * numTilesPerSide;
            y = TILE_SIZE * (i - 8);
        }
        else if (i > 16 && i < 24) { // Các ô trên đường dưới (từ ô 16 đến 22)
            x = TILE_SIZE * (24 - i);
            y = TILE_SIZE * numTilesPerSide;
        }
        else if (i > 24 && i < 32) { // Các ô trên đường bên trái (từ ô 24 đến 31)
            x = 0;
            y = TILE_SIZE * (32 - i);
        }
        break;
    }
}

void Board::setupSpecialTiles() {
    // Tạo danh sách các ô đặc biệt
    struct SpecialTileInfo {
        std::string name;
        TileType type;
        std::function<void(Player*)> onLand; // Sử dụng Player*
        int cost = 0;
        int houseMax = 4; // Giá trị mặc định cho houseMax
    };
    std::vector<SpecialTileInfo> specialTiles = {
        {"Start", TileType::START, [](Player* player) { player->setMoney(player->getMoney() + 200); }},
        {"Lost Island", TileType::LOST_ISLAND, [](Player* player) {
            player->setIsOnLostIsland(true);
            std::cout << player->getName() << " is stranded on Lost Island!" << std::endl;
        }},
        {"Worlds", TileType::WORLDS, [this](Player* player) { this->handleWorldsEvent(*player); }},
        {"World Tour", TileType::WORLD_TOUR, [](Player* player) {
            player->setOnWorldTour(true);
            std::cout << player->getName() << " activated World Tour!" << std::endl;
        }},
        {"Chance", TileType::CHANCE, [this](Player* player) { this->handleChanceEvent(*player); }},
        {"Chance", TileType::CHANCE, [this](Player* player) { this->handleChanceEvent(*player); }},
        {"Chance", TileType::CHANCE, [this](Player* player) { this->handleChanceEvent(*player); }},
        {"Beach", TileType::BEACH, nullptr, 200, 1},
        {"Beach", TileType::BEACH, nullptr, 200, 1},
        {"Beach", TileType::BEACH, nullptr, 200, 1},
        {"Beach", TileType::BEACH, nullptr, 200, 1},
        {"Tax", TileType::TAX, [this](Player* player) { this->applyTax(*player); }} // Giả sử applyTax chỉ nhận Player*
    };

    // Tạo danh sách các ô Property
    struct PropertyInfo {
        std::string name;
        int cost;
        ColorGroup colorGroup;
    };
    std::vector<PropertyInfo> properties = {
        {"GRANADA", 120, ColorGroup::BROWN},
            {"SEVILLE", 120, ColorGroup::BROWN},
            {"MADRID", 150, ColorGroup::BROWN},
            {"MADRID", 150, ColorGroup::BROWN},
            {"MA CAO", 170, ColorGroup::PINK},
            {"BEIJING", 200, ColorGroup::PINK},
            {"SHANGHAI", 220, ColorGroup::PINK},
            {"VENICE", 250, ColorGroup::DARK_BLUE},
            {"MILAN", 270, ColorGroup::DARK_BLUE},
            {"ROME", 300, ColorGroup::DARK_BLUE},
            {"HAMBURG", 330,  ColorGroup::ORANGE},
            {"BERLIN", 360,  ColorGroup::ORANGE},
            {"LONDON", 400,  ColorGroup::YELLOW},
            {"SYDNEY", 420,  ColorGroup::YELLOW},
            {"CHICAGO", 450,ColorGroup::NONE},
            {"LAS VEGAS", 470, ColorGroup::NONE},
            {"NEW YORK", 500, ColorGroup::NONE},
            {"LYON", 520,  ColorGroup::GREEN},
            {"PARIS", 560, ColorGroup::GREEN},
            {"OSAKA", 580, ColorGroup::LIGHT_BLUE},
            {"TOKYO", 650, ColorGroup::LIGHT_BLUE}
    };

    // Khởi tạo các ô đặc biệt
    int specialTileIndex = 0;
    for (int i = 0; i < NUM_TILES; ++i) {
        if (specialTileIndex < specialTiles.size() &&
            i == specialTilePositions[specialTileIndex]) { // Kiểm tra vị trí của ô đặc biệt
            SpecialTileInfo info = specialTiles[specialTileIndex];
            board[i] = TileBuilder()
                .withName(info.name)
                .withType(info.type)
                .withOnLand(info.onLand)
                .withCost(info.cost)
                .withHouseMax(info.houseMax)
                .build();
            specialTileIndex++;
        }
        else if (i - 1 < properties.size()) { // Kiểm tra vị trí của ô Property
            PropertyInfo info = properties[i - 1];
            board[i] = TileBuilder()
                .withName(info.name)
                .withCost(info.cost)
                .withType(TileType::PROPERTY)
                .withHouseMax(4)
                .withColorGroup(info.colorGroup)
                .build();
        }
    }
}

void Board::renderPlayerAt(Player* player, int x, int y) {
    SDL_Texture* playerTexture = player->getSprite();
    gameInstance->drawPlayer(playerTexture, player->getX(), player->getY());
}

void Board::renderPlayers() {
    for (Player& player : gameInstance->getPlayers()) {
        // Lấy vị trí hiện tại của người chơi
        Tile& tile = board[player.getPosition()];

        // Lấy số lượng người chơi trên cùng một ô (tile)
        int numPlayers = tile.getPlayersOnTile().size();

        // Nếu không có người chơi nào trên ô, bỏ qua việc render
        if (numPlayers == 0) continue;

        // Render từng người chơi trên ô đó
        int i = 0;
        for (Player* p : tile.getPlayersOnTile()) {
            // Tính toán offset để người chơi được phân bố đều trên ô
            int offsetX = (i % 2 == 0) ? -10 : 10; // Chia người chơi thành 2 cột
            int offsetY = (i / 2) * 10;

            // Render người chơi với tọa độ đã điều chỉnh
            renderPlayerAt(p, tile.getX() + offsetX, tile.getY() + offsetY);
            i++;
        }
    }
}

void Board::renderBoards() {
    for (int i = 0; i < NUM_TILES; ++i) {
        int x, y;

        calculateTilePosition(i, x, y);
        // Save coordinates into the tile
        board[i].setPosition(x, y);
        
        // Draw the outer border of the tile (black)
        SDL_Rect borderRect = { x, y, TILE_SIZE, TILE_SIZE };
        SDL_SetRenderDrawColor(gameInstance->getRenderer(), 0, 0, 0, 255);
        SDL_RenderFillRect(gameInstance->getRenderer(), &borderRect);

        // Draw the inner part of the tile
        SDL_Rect innerRect = { x + 2, y + 2, TILE_SIZE - 4, TILE_SIZE - 4 };

        // Set tile color based on its type
        switch (board[i].getTileType()) {
        case TileType::START:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 0, 255, 0, 255);  // Green
            break;
        case TileType::LOST_ISLAND:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 128, 128, 128, 255);  // Gray
            break;
        case TileType::WORLDS:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 255, 165, 0, 255);  // Orange
            break;
        case TileType::CHANCE:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 255, 0, 0, 255);  // Red
            break;
        case TileType::WORLD_TOUR:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 255, 255, 0, 255);  // Yellow
            break;
        case TileType::BEACH:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 0, 127, 255, 255);  // Blue
            break;
        case TileType::TAX:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 0, 127, 255, 0);  
            break;
        default:
            SDL_SetRenderDrawColor(gameInstance->getRenderer(), 255, 255, 255, 255);  // White
        }

        SDL_RenderFillRect(gameInstance->getRenderer(), &innerRect);
        if (board[i].getTileType() == TileType::PROPERTY && board[i].getNumHouses() > 0) {
            // Tính toán tọa độ (x, y) cho ngôi nhà
            int houseX = x + 10; // Ví dụ: cách mép trái ô đất 10 pixel
            int houseY = y + 10; // Ví dụ: cách mép trên ô đất 10 pixel
            for (int j = 0; j < board[i].getNumHouses(); ++j) {
                renderHouse(gameInstance->getRenderer(), gameInstance->getHouseTexture(), houseX, houseY);
                houseX += 15; // Ví dụ: khoảng cách giữa các ngôi nhà là 15 pixel
            }
        }
        // Render the tile name
        SDL_Color textColor = { 0, 0, 0 };  // Black
        gameInstance->renderText(board[i].getName(), x + 5, y + 5, textColor);
    }
}

void Board::renderHouse(SDL_Renderer* renderer, SDL_Texture* houseTexture, int x, int y) {
    SDL_Rect renderQuad = { x, y, 50, 50 };
    SDL_RenderCopy(renderer, houseTexture, nullptr, &renderQuad);
}

void Board::createBoard() {
    board.resize(NUM_TILES);
    setupSpecialTiles();
}
