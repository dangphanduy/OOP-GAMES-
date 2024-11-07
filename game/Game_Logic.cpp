#include "Game.h"
#include "Player.h"
#include "Constants.h"
#include "Board.h"
#include "Tile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <random>

using namespace std;

std::random_device rd;
std::mt19937 gen(rd());

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();
    while (true) {
        handleEvents();

        Player& currentPlayer = players[currentPlayerIndex];
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            exit(0);
        }
        else if (e.type == SDL_KEYDOWN) {
            handleKeyPress(e.key.keysym.sym);
        }
    }
}

void Game::handleKeyPress(SDL_Keycode key) {
    Player& currentPlayer = players[currentPlayerIndex];
    if (key == SDLK_SPACE) {
        handleRollDice(currentPlayer);
    }
    else if (key == SDLK_h) {
        Tile& currentTile = board->getBoard()[currentPlayer.getPosition()];
        buyHouse(currentTile);
    }
    else if (key == SDLK_b) {
        Tile& currentTile = board->getBoard()[currentPlayer.getPosition()];
        buyBeach(currentPlayer, currentTile);
    }
    else if (key == SDLK_l) {
        sellHouse(currentPlayer);
    }
    else if (key == SDLK_n) {
        nextTurn();
    }
    else if (key == SDLK_e) {
        Tile& currentTile = board->getBoard()[currentPlayer.getPosition()];
        handleBuyProperty(currentPlayer, currentTile);
    }
    else if (key == SDLK_ESCAPE) {
        exit(1);
    }
}

void Game::update(float deltaTime) {
    Player& currentPlayer = players[currentPlayerIndex];

    // Kiểm tra thời gian trôi qua
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - turnStartTime >= TURN_TIME_LIMIT) {
        std::cout << "Time's up! Automatically switching to the next player." << std::endl;

        // Nếu người chơi chưa đổ xúc xắc, tự động đổ xúc xắc cho họ
        if (!currentPlayer.getIsMoving() && currentPlayer.getCanRollDice()) {
            int diceRoll = rollDice();
            currentPlayer.move(diceRoll, board->getBoard());

            // Cập nhật trạng thái di chuyển của người chơi
            currentPlayer.setIsMoving(true);
            currentPlayer.setCanRollDice(false);
        }
        else {
            // Chuyển lượt nếu người chơi đã đổ xúc xắc
            nextTurn();
        }
        return;
    }

    // Nếu player di chuyển, cập nhật vị trí của họ
    if (currentPlayer.getIsMoving()) {
        currentPlayer.updatePosition(deltaTime, players);
        // Kiểm tra xem player đã đến đích chưa
        if (!currentPlayer.getIsMoving()) {
            std::cout << currentPlayer.getName() << " has reached the target position." << std::endl;

            // Kích hoạt sự kiện nếu có
            Tile& landedTile = board->getBoard()[currentPlayer.getPosition()];
            landedTile.triggerOnLand(&currentPlayer);

            // Kiểm tra xem người chơi có đang ở trên Lost Island hay không
            if (currentPlayer.getIsOnLostIsland()) {
                currentPlayer.setTurnsOnLostIsland(currentPlayer.getTurnsOnLostIsland() + 1);
                std::cout << currentPlayer.getName() << " is stuck on Lost Island for "
                    << currentPlayer.getTurnsOnLostIsland() << " turns." << std::endl;

                // Kiểm tra điều kiện để thoát khỏi đảo
                if (currentPlayer.getTurnsOnLostIsland() >= 3 || hasRolledDoubles()) {
                    currentPlayer.setIsOnLostIsland(false);
                    currentPlayer.setTurnsOnLostIsland(0);
                    std::cout << currentPlayer.getName() << " escapes Lost Island!" << std::endl;
                }
                else {
                    // Nếu chưa thoát khỏi đảo, kết thúc lượt
                    return;
                }
            }

            // Kiểm tra xem ô đất có thuộc sở hữu của người chơi khác hay không
            if (landedTile.getTileType() == TileType::PROPERTY &&
                !landedTile.getOwnerName().empty() &&
                landedTile.getOwnerName() != currentPlayer.getName()) {

                // Tính toán số tiền cần trả (ví dụ: 10% giá trị ô đất)
                int rent = landedTile.getHousePrice() * 0.1;
                // Trừ tiền người chơi hiện tại
                currentPlayer.setMoney(currentPlayer.getMoney() - rent);
                // Cộng tiền cho chủ sở hữu
                Player& owner = getPlayer(landedTile.getOwnerName());
                owner.setMoney(owner.getMoney() + rent);

                std::cout << currentPlayer.getName() << " paid $" << rent << " to " << owner.getName() << " for landing on " << landedTile.getName() << std::endl;
            }

            // Kiểm tra xem người chơi có bị phá sản sau khi di chuyển hay không
            if (currentPlayer.getMoney() <= 0) {
                currentPlayer.setState(PlayerState::Bankrupt);
                std::cout << currentPlayer.getName() << " is bankrupt!" << std::endl;
            }
        }
    }
}

void Game::auctionProperty(Tile& tile) {
    std::cout << "\nStarting auction for " << tile.getName() << "!\n" << std::endl;
    int currentPrice = tile.getHousePrice();  // Giá khởi điểm bằng giá trị ban đầu của ô đất
    int highestBid = 0;                     // Giá thầu cao nhất hiện tại
    Player* highestBidder = nullptr;       // Người chơi trả giá cao nhất hiện tại
    bool biddingContinues = true;          // Biến kiểm soát vòng lặp đấu giá

    // Vòng lặp đấu giá
    while (biddingContinues) {
        biddingContinues = false;  // Giả định không có ai trả giá cao hơn

        // Duyệt qua danh sách người chơi
        for (Player& player : players) {
            if (player.getState() != PlayerState::Bankrupt && player.getMoney() > highestBid) {  // Chỉ người chơi chưa phá sản và có đủ tiền mới có thể tham gia đấu giá
                char choice;
                std::cout << player.getName() << ", do you want to bid higher than $" << highestBid << "? (Y/N): ";
                std::cin >> choice;

                if (choice == 'Y' || choice == 'y') {
                    int bid;
                    std::cout << "Enter your bid (must be higher than $" << highestBid << "): ";
                    std::cin >> bid;

                    if (bid > highestBid && bid <= player.getMoney()) {
                        highestBid = bid;
                        highestBidder = &player;
                        biddingContinues = true;  // Có người trả giá cao hơn, tiếp tục vòng lặp
                    }
                    else {
                        std::cout << "Invalid bid. Please enter a valid amount." << std::endl;
                    }
                }
            }
        }
    }

    // Kết thúc đấu giá
    if (highestBidder != nullptr) {
        highestBidder->setMoney(highestBidder->getMoney() - highestBid);  // Trừ tiền người chơi thắng cuộc
        tile.setOwnerName(highestBidder->getName());                    // Đặt người chơi thắng cuộc làm chủ sở hữu
        highestBidder->addProperty(&tile);                              // Thêm đất vào danh sách tài sản
        std::cout << highestBidder->getName() << " won the auction for " << tile.getName() << " with a bid of $" << highestBid << "!" << std::endl;
    }
    else {
        std::cout << "No one bid on " << tile.getName() << "." << std::endl;
    }
}

int Game::rollDice() {
    std::uniform_int_distribution<> dis(1, 6);
    int dice = dis(gen);

    // Cập nhật các biến lưu trữ kết quả
    auto currentPlayer = players[currentPlayerIndex];
    currentPlayer.setPreviousRoll(currentPlayer.getCurrentRoll());
    currentPlayer.setCurrentRoll(dice);

    return dice;
}

void Game::handleRollDice(Player& currentPlayer) {
    if (!currentPlayer.getIsMoving() && currentPlayer.getCanRollDice()) {
        if (currentPlayer.getOnWorldTour()) {
            std::cout << currentPlayer.getName() << " is using World Tour to choose a destination." << std::endl;
            currentPlayer.move(0, board->getBoard());
        }
        else {
            int diceRoll = rollDice();
            std::cout << "\n" << currentPlayer.getName() << " rolled a " << diceRoll << std::endl;
            currentPlayer.move(diceRoll, board->getBoard());
            currentPlayer.setIsMoving(true);
            currentPlayer.setCanRollDice(false);
        }
    }
    else {
        std::cout << currentPlayer.getName() << " has already rolled the dice." << std::endl;
    }
}

void Game::nextTurn() {
    do {
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    } while (players[currentPlayerIndex].getState() == PlayerState::Bankrupt); // Bỏ qua người chơi đã phá sản

    Player& currentPlayer = players[currentPlayerIndex];
    std::cout << currentPlayer.getName() << "'s turn." << std::endl;
    currentPlayer.setCanRollDice(true);
    turnStartTime = SDL_GetTicks(); // Reset thời gian bắt đầu lượt
}

bool Game::hasRolledDoubles() {
    std::uniform_int_distribution<> dis(1, 6);
    int dice1 = dis(gen);
    int dice2 = dis(gen);

    std::cout << "Player rolled: " << dice1 << " and " << dice2 << std::endl;
    return dice1 == dice2;  // Trả về true nếu lăn được "doubles"
}

void Game::setupChanceEvents() {
    chanceEvents = {
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Advanced to Start." << std::endl;
            player.setPosition(0);
            player.setMoney(player.getMoney() + 200);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Go to Lost IsLand." << std::endl;
            player.setPosition(8);
            player.setIsOnLostIsland(true);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
            std::cout << player.getName() << " moved to Lost IsLand." << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Advanced To Worlds" << std::endl;
            player.setPosition(16);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
            std::cout << player.getName() << " moved to Worlds" << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Pay fine of 50." << std::endl;
            player.setMoney(player.getMoney() - 50);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Happy Birthday!" << std::endl;
            int birthdayMoney = 50;
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - birthdayMoney);
                    player.setMoney(player.getMoney() + birthdayMoney);
                }
            }
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Bank pays you dividend of $50." << std::endl;
            player.setMoney(player.getMoney() + 50);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: A map" << std::endl;
            player.setHasMap(true);
            std::cout << player.getName() << " now has a map to escape Lost Island!" << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Go forward 2 spaces." << std::endl;
            int newPosition = (player.getPosition() + 2 + NUM_TILES) % NUM_TILES;
            player.setPosition(newPosition);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY()); // Bắt đầu di chuyển đến vị trí mới

            std::cout << player.getName() << " moved back to " << player.getPosition() << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Go back 3 spaces." << std::endl;
            // Tính toán vị trí mới sau khi lùi 3 bước
            int newPosition = (player.getPosition() - 3 + NUM_TILES) % NUM_TILES;
            // Di chuyển người chơi đến vị trí mới
            player.setPosition(newPosition);
            // Cập nhật tọa độ hiển thị và trạng thái di chuyển
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY()); // Bắt đầu di chuyển đến vị trí mới
            std::cout << player.getName() << " moved back to " << player.getPosition() << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Make general repairs on all your property." << std::endl;
            int repairCost = 75; // Chi phí sửa chữa cho mỗi người chơi khác
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - repairCost);
                    player.setMoney(player.getMoney() + repairCost);
                }
            }
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Pay poor tax of $75." << std::endl;
            player.setMoney(player.getMoney() - 75);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Your building and loan matures." << std::endl;
            player.setMoney(player.getMoney() + 150);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: You have won a crossword competition." << std::endl;
            player.setMoney(player.getMoney() + 100);
        },
    };
}

void Game::handleBuyProperty(Player& currentPlayer, Tile& currentTile) {
    if (currentTile.getTileType() == TileType::PROPERTY && currentTile.getOwnerName().empty()) {
        // Hiển thị thông tin ô đất và giá tiền
        std::cout << currentTile.getName() << " is available for purchase for $" << currentTile.getHousePrice() << std::endl;

        if (currentPlayer.getMoney() >= currentTile.getHousePrice()) {
            char choice;
            std::cout << "Do you want to buy " << currentTile.getName() << "? (Y/N): ";
            std::cin >> choice;
            if (choice == 'Y' || choice == 'y') {
                // Trừ tiền người chơi và đặt người chơi làm chủ sở hữu
                currentPlayer.setMoney(currentPlayer.getMoney() - currentTile.getHousePrice());
                currentTile.setOwnerName(currentPlayer.getName());
                currentPlayer.addProperty(&currentTile); // Thêm đất vào danh sách tài sản
                std::cout << currentPlayer.getName() << " bought " << currentTile.getName() << std::endl;
            }
        }
        else {
            std::cout << currentPlayer.getName() << " doesn't have enough money to buy " << currentTile.getName() << std::endl;
        }
    }
    else {
        std::cout << "You cannot buy this property." << std::endl;
    }
}

void Game::buyHouse(Tile& tile) {
    Player& currentPlayer = players[currentPlayerIndex];

    // Kiểm tra loại ô đất
    if (tile.getTileType() != TileType::PROPERTY) {
        std::cout << "You can only buy a house on regular plots." << std::endl;
        return;
    }

    if (tile.getOwnerName() != currentPlayer.getName()) {
        std::cout << "You do not own this property." << std::endl;
        // Yêu cầu người chơi mua đất trước
        std::cout << "You need to buy this property first before buying a house." << std::endl;
        return;
    }

    // Kiểm tra quyền sở hữu
    if (!tile.getOwnerName().empty() && tile.getOwnerName() != currentPlayer.getName()) {
        std::cout << "This property is already owned by another player." << std::endl;
        return;
    }

    // Nếu người chơi chưa sở hữu ô đất, thêm nó vào danh sách tài sản
    if (tile.getOwnerName().empty()) {
        tile.setOwnerName(currentPlayer.getName());  // Đặt người chơi làm chủ sở hữu ô đất
        currentPlayer.addProperty(&tile);
    }

    // Kiểm tra khả năng mua nhà
    if (currentPlayer.canBuyHouse(tile) && currentPlayer.getMoney() >= tile.getHousePrice()) {
        char choice;
        std::cout << "Are you sure you want to buy a beach on " << tile.getName() << " for " << tile.getHousePrice() << " ? (Y / N) : ";
        std::cin >> choice;

        if (choice == 'Y' || choice == 'y') {
            currentPlayer.setMoney(currentPlayer.getMoney() - tile.getHousePrice());
            tile.setNumHouses(tile.getNumHouses() + 1);
            std::cout << "You have bought a house in " << tile.getName() << "." << std::endl;
        }
        else {
            std::cout << "You didn't buy a house." << std::endl;
        }
    }
    else {
        if (tile.getNumHouses() >= MAX_HOUSE) {
            std::cout << "You have reached the limit on the number of houses on this plot." << std::endl;
        }
        else {
            std::cout << (currentPlayer.getMoney() < tile.getHousePrice() ? "You can't afford to buy a house on this plot." : "You cannot buy a house on this plot.") << std::endl;
        }
    }
}

void Game::buyBeach(Player& player, Tile& tile) {
    // Kiểm tra nếu người chơi hiện tại đang đứng trên bãi biển
    if (tile.getTileType() == TileType::BEACH) {
        // Kiểm tra nếu bãi biển đã được sở hữu
        if (!tile.getOwnerName().empty()) { // Kiểm tra ownerName
            std::cout << "This beach is already owned by another player." << std::endl;
            return;
        }

        // Kiểm tra người chơi có đủ tiền để mua bãi biển
        if (player.getMoney() >= BEACH_COST) {
            char choice;
            std::cout << "Are you sure you want to buy a beach on " << tile.getName() << " for " << tile.getHousePrice() << " ? (Y / N) : ";
            std::cin >> choice;

            if (choice == 'Y' || choice == 'y') {
                player.setMoney(player.getMoney() - BEACH_COST);
                tile.setOwnerName(player.getName());
                player.addProperty(&tile);
                std::cout << player.getName() << " bought the beach." << std::endl;
            }
            else std::cout << player.getName() << " didn't buy the beach.";
            // Kiểm tra nếu người chơi sở hữu cả 4 bãi biển
            int beachCount = std::count_if(
                player.getProperties().begin(),
                player.getProperties().end(),
                [](const Tile* property) {
                    return property->getTileType() == TileType::BEACH;
                }
            );
            if (beachCount == 4) {
                std::cout << player.getName() << " owns all 4 beaches and wins the game!" << std::endl;
                exit(0); // Kết thúc trò chơi
            }
        }
        else {
            std::cout << player.getName() << " doesn't have enough money to buy the beach!" << std::endl;
        }
    }
    else {
        std::cout << "This is not a beach." << std::endl;
    }
}

void Game::sellHouse(Player& player) {
    std::vector<Tile*> ownedTiles = getOwnedTilesWithHouses(player);
    if (ownedTiles.empty()) {
        std::cout << player.getName() << " does not own any houses to sell." << std::endl;
        return;
    }

    displayOwnedTiles(player, ownedTiles);
    int choice = getPlayerChoice(ownedTiles.size());

    if (choice == -1) {
        std::cout << "Invalid choice. Please select a valid property number." << std::endl;
        return;
    }

    Tile* selectedTile = ownedTiles[choice - 1];

    char confirm;
    std::cout << "Are you sure you want to sell a house on " << selectedTile->getName() << "? (Y/N): ";
    std::cin >> confirm;

    if (confirm == 'Y' || confirm == 'y') {
        sellHouseOnTile(player, selectedTile);
    }
    else {
        std::cout << player.getName() << " did not sell the house." << std::endl;
    }
}

std::vector<Tile*> Game::getOwnedTilesWithHouses(Player& player) {
    std::vector<Tile*> ownedTiles;
    for (Tile& tile : board->getBoard()) {
        if (tile.getOwnerName() == player.getName() && tile.getNumHouses() > 0) {
            ownedTiles.push_back(&tile);
        }
    }
    return ownedTiles;
}

void Game::displayOwnedTiles(Player& player, const std::vector<Tile*>& ownedTiles) {
    std::cout << player.getName() << ", select a property to sell a house from:" << std::endl;
    for (size_t i = 0; i < ownedTiles.size(); ++i) {
        std::cout << i + 1 << ": " << ownedTiles[i]->getName()
            << " (Houses: " << ownedTiles[i]->getNumHouses() << ")" << std::endl;
    }
}

int Game::getPlayerChoice(size_t numChoices) {
    int choice;
    std::cin >> choice;
    if (std::cin.fail() || choice <= 0 || choice > static_cast<int>(numChoices)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1; // Trả về -1 nếu lựa chọn không hợp lệ
    }
    return choice;
}

void Game::sellHouseOnTile(Player& player, Tile* tile) {
    if (tile->getNumHouses() > 0) {
        int sellPrice = tile->getHousePrice() / 2;
        tile->setHousePrice(sellPrice);
        player.setMoney(player.getMoney() + sellPrice);
        tile->setNumHouses(tile->getNumHouses() - 1);
        std::cout << player.getName() << " sold a house on " << tile->getName()
            << " for $" << sellPrice << std::endl;
    }
    else {
        std::cout << "No houses to sell on " << tile->getName() << std::endl;
    }
}

void Game::handleBankruptcy(Player& player) {
    // Lấy danh sách tài sản của người chơi
    std::unordered_set<Tile*> properties = player.getProperties();

    // Duyệt qua danh sách tài sản và đặt ownerName thành rỗng
    for (Tile* property : properties) {
        property->setOwnerName("");
        property->setNumHouses(0); // Reset số lượng nhà trên ô đất
    }

    // Xóa toàn bộ tài sản của người chơi
    player.clearProperties();
}

void Player::clearProperties() {
    properties.clear(); // Xóa tất cả các phần tử trong vector properties
}