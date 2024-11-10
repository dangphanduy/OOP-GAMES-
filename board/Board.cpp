#include <iostream>
#include <string>
#include <algorithm>  
#include <random> 
#include "Player.h"
#include "Board.h"
#include "Constants.h"
#include "Game.h"
#include "TileBuilder.h"

using namespace std;

void Tile::triggerOnLand(Player* player) {
    if (onLand) {
        onLand(player);
    }
}

Board::Board(Game* game) : game(game) {}
// Xử lý các sự kiện
void Board::handleChanceEvent(Player& player) {
    random_device rd;
    mt19937 gen{ rd() };
    uniform_int_distribution<size_t> dis(0, game->getChanceEvents().size() - 1);
    size_t eventIndex = dis(gen);
    vector<Player*> playerPointers;
    for (auto& player : game->getPlayers()) {
        playerPointers.push_back(&player);
    }
    game->getChanceEvents()[eventIndex](player, playerPointers);
}

void Board::handleWorldsEvent(Player& player) {
    cout << player.getName() << " landed on Worlds!" << endl;

    // Lấy danh sách các ô đất mà người chơi sở hữu
    const auto ownedProperties = player.getOwnedProperty();

    // Nếu người chơi không sở hữu ô đất nào
    if (ownedProperties.empty()) {
        cout << "You don't own any properties yet." << endl;
        return;
    }

    // Hiển thị danh sách các ô đất
    cout << "Choose a property to increase its value:" << endl;
    for (size_t i = 0; i < ownedProperties.size(); ++i) {
        cout << i + 1 << ". " << ownedProperties[i]->getName() << endl;
    }

    // Nhận lựa chọn của người chơi
    int choice;
    if (!(cin >> choice)) { // Kiểm tra xem người chơi có nhập số nguyên hay không
        cout << "Invalid choice." << endl;
        cin.clear(); // Xóa lỗi
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Xóa bộ đệm
        return;
    }

    // Xử lý lựa chọn
    if (choice > 0 && choice <= ownedProperties.size()) {
        Tile* selectedTile = ownedProperties[choice - 1];
        selectedTile->setValueMultiplier(selectedTile->getValueMultiplier() + 1); // Tăng hệ số nhân lên theo cấp số cộng
        player.setWorldsUsed(player.getWorldsUsed() + 1); // Tăng số lần sử dụng "Worlds"

        cout << "The value of " << selectedTile->getName() << " has been multiplied" << endl;
        cout << "Multiplier: " << selectedTile->getValueMultiplier() << endl;
    }
    else {
        cout << "Invalid choice." << endl;
    }
}

void Board::applyTax(Player& player) {
    // Tìm ô Tax trên bàn cờ
    static Tile* taxTile = nullptr;
    if (taxTile == nullptr) {
        for (Tile& t : board) {
            if (t.getTileType() == TileType::TAX) {
                taxTile = &t;
                break;
            }
        }
    }

    if (taxTile != nullptr) {
        int totalHouseValue = 0;

        // Tính tổng giá trị nhà của người chơi
        for (const Tile* p : player.getProperties()) {
            totalHouseValue += p->getNumHouses() * p->getHousePrice();
        }

        // Tính tiền thuế (10% tổng giá trị nhà)
        int taxAmount = static_cast<int>(0.1 * totalHouseValue);
        player.setMoney(player.getMoney() - taxAmount);

        cout << player.getName() << " paid $" << taxAmount << " in taxes at "
            << taxTile->getName() << endl;
    }

    game->checkAndHandleBankruptcy(player);
}
// Tính toán vị trí cho từng ô
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
    // Các ô đặc biệt
    board[0] = TileBuilder()
        .withName("Start")
        .withType(TileType::START)
        .withOnLand([this](Player* player) {  player->setMoney(player->getMoney() + 200); })
        .build();
    board[8] = TileBuilder()
        .withName("Lost Island")
        .withType(TileType::LOST_ISLAND)
        .withOnLand([this](Player* player) { 
        player->setIsOnLostIsland(true);
        cout << player->getName() << " is stranded on Lost Island!" << endl; })
        .build();
    board[16] = TileBuilder()
        .withName("Free Parking")
        .withType(TileType::FREE_PARKING)
        .build();
    board[24] = TileBuilder()
        .withName("Worlds")
        .withType(TileType::WORLDS)
        .withOnLand([this](Player* player) { 
        this->handleWorldsEvent(*player); 
            })
        .build();
    board[30] = TileBuilder()
        .withName("TAX")
        .withType(TileType::TAX)
        .withOnLand([this](Player* player) {
        this->applyTax(*player);
            })
        .build();
    // Các ô cơ hội 
    for (int i : {12, 20, 28}) {
        board[i] = TileBuilder()
            .withName("Chance")
            .withType(TileType::CHANCE)
            .withOnLand([this](Player *player) {  this->handleChanceEvent(*player); })
            .build();
    }

    // Các ô bãi biển
    for (int i : {4, 14, 18, 25}) {
        board[i] = TileBuilder()
            .withName("Beach")
            .withCost(200)
            .withType(TileType::BEACH)
            .withHouseMax(1)
            .build();
    }

    bool usedPositions[NUM_TILES] = { false }; // Khởi tạo tất cả là false
    for (int pos : specialTilePositions) {
        usedPositions[pos] = true; // Đánh dấu các vị trí của ô đặc biệt là true
    }

    //Các ô đất thường
    struct PropertyInfo {
        string name;
        int cost;
        ColorGroup colorGroup;
    };
    vector<PropertyInfo> properties = {
        {"GRANADA", 120, ColorGroup::BROWN},
        {"SEVILLE", 120, ColorGroup::BROWN},
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
        {"CHICAGO", 450,ColorGroup::NAVAJO_WHITE},
        {"LAS VEGAS", 470, ColorGroup::NAVAJO_WHITE},
        {"NEW YORK", 500, ColorGroup::NAVAJO_WHITE},
        {"LYON", 520,  ColorGroup::GREEN},
        {"PARIS", 560, ColorGroup::GREEN},
        {"OSAKA", 580, ColorGroup::LIGHT_BLUE},
        {"TOKYO", 650, ColorGroup::LIGHT_BLUE}
    };

    int propertyIndex = 0;
    for (int i = 0; i < NUM_TILES && propertyIndex < properties.size(); ++i) {
        if (!usedPositions[i]) { // Chỉ tạo ô nếu vị trí chưa được sử dụng
            PropertyInfo info = properties[propertyIndex];
            board[i] = TileBuilder()
                .withName(info.name)
                .withCost(info.cost)
                .withType(TileType::PROPERTY)
                .withHouseMax(4)
                .withColorGroup(info.colorGroup)
                .build();
            propertyIndex++;
        }
    }
}

void Board::renderBoards() {
    for (int i = 0; i < NUM_TILES; ++i) {

        int x, y;
        calculateTilePosition(i, x, y);
        board[i].setPosition(x, y);

        // Vẽ bảng mặc định
        SDL_Rect borderRect = { x, y, TILE_SIZE, TILE_SIZE };
        SDL_SetRenderDrawColor(game->getRenderer(), 0, 0, 0, 255);
        SDL_RenderFillRect(game->getRenderer(), &borderRect);
        SDL_Rect innerRect = { x + 2, y + 2, TILE_SIZE - 4, TILE_SIZE - 4 };

        // Vẽ màu cho từng ô
        if(board[i].getTileType() == TileType::PROPERTY) {
            switch (board[i].getColorGroup()) {
            case ColorGroup::BROWN:
                SDL_SetRenderDrawColor(game->getRenderer(), 150, 75, 0, 255);
                break;
            case ColorGroup::DARK_BLUE:
                SDL_SetRenderDrawColor(game->getRenderer(), 0, 0, 139, 255);
                break;
            case ColorGroup::GREEN:
                SDL_SetRenderDrawColor(game->getRenderer(), 0, 128, 0, 255);
                break;
            case ColorGroup::LIGHT_BLUE:
                SDL_SetRenderDrawColor(game->getRenderer(), 173, 216, 230, 255);
                break;
            case ColorGroup::ORANGE:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 165, 0, 255);
                break;
            case ColorGroup::YELLOW:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 255, 0, 255);
                break;
            case ColorGroup::PINK:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 192, 203, 255);
                break;
            case ColorGroup::NAVAJO_WHITE:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 222, 173, 255); // navajo trắng
                break;
            default:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 255, 255, 255); 
                break;
            }
        }
        else {
            switch (board[i].getTileType()) {
            case TileType::START:
                SDL_SetRenderDrawColor(game->getRenderer(), 0, 255, 0, 255);  // Xanh lá
                break;
            case TileType::LOST_ISLAND:
                SDL_SetRenderDrawColor(game->getRenderer(), 0, 206, 209, 255);  // Xanh lá
                break;
            case TileType::FREE_PARKING:
                SDL_SetRenderDrawColor(game->getRenderer(), 128, 128, 128, 255);  // Màu xám
                break;
            case TileType::WORLDS:
                SDL_SetRenderDrawColor(game->getRenderer(), 100, 149, 237, 255);  // màu xanh hoa ngô đồng
                break;
            case TileType::CHANCE:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 0, 0, 255);  // Màu đỏ
                break;
            case TileType::BEACH:
                SDL_SetRenderDrawColor(game->getRenderer(), 0, 127, 255, 255);  // Màu xanh dương
                break;
            case TileType::TAX:
                SDL_SetRenderDrawColor(game->getRenderer(), 245, 255, 250, 255);  // kem bạc hà
                break;
            default:
                SDL_SetRenderDrawColor(game->getRenderer(), 255, 255, 255, 255);  // Màu trắng
                break;
            }
        }

        SDL_RenderFillRect(game->getRenderer(), &innerRect);
        if (board[i].getTileType() == TileType::PROPERTY && board[i].getNumHouses() > 0) {
            // Tính toán tọa độ (x, y) cho ngôi nhà
            int houseX = x + 10; // Ví dụ: cách mép trái ô đất 10 pixel
            int houseY = y + 10; // Ví dụ: cách mép trên ô đất 10 pixel
            SDL_Texture* houseTexture = game->getPlayer(board[i].getOwnerName()).getHouseTexture();
            for (int j = 0; j < board[i].getNumHouses(); ++j) {
                renderHouse(game->getRenderer(), houseTexture, houseX, houseY);
                houseX += 15; // Ví dụ: khoảng cách giữa các ngôi nhà là 15 pixel
            }
        }
        // Render tên ô 
        SDL_Color textColor = { 0, 0, 0 }; 
        game->renderText(board[i].getName(), x + 5, y + 5, textColor);
    }
}
// Vẽ người chơi
void Board::renderPlayerAt(Player* player, int x, int y) {
    SDL_Texture* playerTexture = player->getSprite();
    game->drawPlayer(playerTexture, static_cast<int>(player->getX()), static_cast<int>(player->getY()));
}

void Board::renderPlayers() {
    for (Player& player : game->getPlayers()) {
        // Lấy ô hiện tại của người chơi
        Tile& tile = board[player.getPosition()];

        // Lấy số lượng người chơi trên cùng một ô (tile)
        size_t numPlayers = tile.getPlayersOnTile().size();

        // Nếu không có người chơi nào trên ô, bỏ qua việc render
        if (numPlayers == 0) continue;

        // Render từng người chơi trên ô đó
        int i = 0;
        for (Player* p : tile.getPlayersOnTile()) {
            // Tính toán offset để người chơi được phân bố đều trên ô
            int offsetX = (i % 2 == 0) ? -10 : 10; // Chia người chơi thành 2 cột
            int offsetY = (i / 2) * 10;

            // Sử dụng tọa độ x, y của người chơi để render
            renderPlayerAt(p, static_cast<int>(p->getX() + offsetX), static_cast<int>(p->getY() + offsetY));
            i++;
        }
    }
}
// Vẽ nhà
void Board::renderHouse(SDL_Renderer* renderer, SDL_Texture* houseTexture, int x, int y) {
    SDL_Rect renderQuad = { x, y, 50, 50 };
    SDL_RenderCopy(renderer, houseTexture, nullptr, &renderQuad);
}
// Tạo ra bảng
void Board::createBoard() {
    board.resize(NUM_TILES);
    setupSpecialTiles();
}
