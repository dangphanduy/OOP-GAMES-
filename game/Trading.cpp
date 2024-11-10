#include "Game/Game.h"
#include "Player/Player.h"
#include "Main/Constants.h"
#include "Board/Board.h"
#include "Board/Tile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

void Game::handleBuyProperty(Player& currentPlayer, Tile& currentTile) {
    if (currentTile.getTileType() == TileType::PROPERTY && currentTile.getOwnerName().empty()) {
        // Hiển thị thông tin ô đất và giá tiền
        cout << currentTile.getName() << " is available for purchase for $" << currentTile.getHousePrice() << endl;

        if (currentPlayer.getMoney() >= currentTile.getHousePrice()) {
            char choice;
            cout << "Do you want to buy " << currentTile.getName() << "? (Y/N): ";
            cin >> choice;
            if (choice == 'Y' || choice == 'y') { // Nếu mua
                // Trừ tiền người chơi và đặt người chơi làm chủ sở hữu
                currentPlayer.setMoney(currentPlayer.getMoney() - currentTile.getHousePrice());
                currentTile.setOwnerName(currentPlayer.getName());
                currentPlayer.addProperty(&currentTile); // Thêm đất vào danh sách tài sản
                cout << currentPlayer.getName() << " bought " << currentTile.getName() << endl;
                checkAndHandleBankruptcy(currentPlayer);
            }
            else {
                auctionProperty(currentTile); // Nếu không mua, đấu giá
            }
        }
        else {
            cout << currentPlayer.getName() << " doesn't have enough money to buy " << currentTile.getName() << endl;
        }
    }
    else {
        cout << "You cannot buy this property." << endl;
    }
}

void Game::auctionProperty(Tile& tile) {
    cout << "\nStarting auction for " << tile.getName() << "!\n" << endl;
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
                cout << player.getName() << ", do you want to bid higher than $" << highestBid << "? (Y/N): ";
                cin >> choice;

                if (choice == 'Y' || choice == 'y') {
                    int bid;
                    cout << "Enter your bid (must be higher than $" << highestBid << "): ";
                    cin >> bid;

                    if (bid > highestBid && bid <= player.getMoney()) {
                        highestBid = bid;
                        highestBidder = &player;
                        biddingContinues = true;  // Có người trả giá cao hơn, tiếp tục vòng lặp
                    }
                    else {
                        cout << "Invalid bid. Please enter a valid amount." << endl;
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
        cout << highestBidder->getName() << " won the auction for " << tile.getName() << " with a bid of $" << highestBid << "!" << endl;
        checkAndHandleBankruptcy(*highestBidder);
    }
    else {
        cout << "No one bid on " << tile.getName() << "." << endl;
    }
}

void Game::buyHouse(Tile& tile) {
    Player& currentPlayer = players[currentPlayerIndex];

    // Kiểm tra loại ô đất
    if (tile.getTileType() != TileType::PROPERTY) {
        cout << "You can only buy a house on regular plots." << endl;
        return;
    }

    if (tile.getOwnerName() != currentPlayer.getName()) {
        cout << "You do not own this property." << endl;
        // Yêu cầu người chơi mua đất trước
        cout << "You need to buy this property first before buying a house." << endl;
        return;
    }

    // Kiểm tra quyền sở hữu
    if (!tile.getOwnerName().empty() && tile.getOwnerName() != currentPlayer.getName()) {
        cout << "This property is already owned by another player." << endl;
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
        cout << "Are you sure you want to buy a beach on " << tile.getName() << " for " << tile.getHousePrice() << " ? (Y / N) : ";
        cin >> choice;

        if (choice == 'Y' || choice == 'y') {
            currentPlayer.setMoney(currentPlayer.getMoney() - tile.getHousePrice());
            tile.setNumHouses(tile.getNumHouses() + 1);
            cout << "You have bought a house in " << tile.getName() << "." << endl;
            checkAndHandleBankruptcy(currentPlayer);
        }
        else {
            cout << "You didn't buy a house." << endl;
        }
    }
    else {
        if (tile.getNumHouses() >= MAX_HOUSE) {
            cout << "You have reached the limit on the number of houses on this plot." << endl;
        }
        else {
            cout << (currentPlayer.getMoney() < tile.getHousePrice() ? "You can't afford to buy a house on this plot." : "You cannot buy a house on this plot.") << endl;
        }
    }
}

void Game::buyBeach(Player& player, Tile& tile) {
    // Kiểm tra nếu người chơi hiện tại đang đứng trên bãi biển
    if (tile.getTileType() == TileType::BEACH) {
        // Kiểm tra nếu bãi biển đã được sở hữu
        if (!tile.getOwnerName().empty()) { // Kiểm tra ownerName
            cout << "This beach is already owned by another player." << endl;
            return;
        }

        // Kiểm tra người chơi có đủ tiền để mua bãi biển
        if (player.getMoney() >= BEACH_COST) {
            char choice;
            cout << "Are you sure you want to buy a beach on " << tile.getName() << " for " << tile.getHousePrice() << " ? (Y / N) : ";
            cin >> choice;

            if (choice == 'Y' || choice == 'y') {
                player.setMoney(player.getMoney() - BEACH_COST);
                tile.setOwnerName(player.getName());
                player.addProperty(&tile);
                cout << player.getName() << " bought the beach." << endl;
                checkAndHandleBankruptcy(player);
            }
            else cout << player.getName() << " didn't buy the beach.";
            // Kiểm tra nếu người chơi sở hữu cả 4 bãi biển
            __int64 beachCount = count_if(
                player.getProperties().begin(),
                player.getProperties().end(),
                [](const Tile* property) {
                    return property->getTileType() == TileType::BEACH;
                }
            );
            if (beachCount == 4) {
                cout << player.getName() << " owns all 4 beaches and wins the game!" << endl;
                exit(0); // Kết thúc trò chơi
            }
        }
        else {
            cout << player.getName() << " doesn't have enough money to buy the beach!" << endl;
        }
    }
    else {
        cout << "This is not a beach." << endl;
    }
}

void Game::sellHouse(Player& player) {
    vector<Tile*> ownedTiles = getOwnedTilesWithHouses(player);
    if (ownedTiles.empty()) {
        cout << player.getName() << " does not own any houses to sell." << endl;
        return;
    }

    displayOwnedTiles(player, ownedTiles);
    int choice = getPlayerChoice(ownedTiles.size());

    if (choice == -1) {
        cout << "Invalid choice. Please select a valid property number." << endl;
        return;
    }

    Tile* selectedTile = ownedTiles[choice - 1];

    char confirm;
    cout << "Are you sure you want to sell a house on " << selectedTile->getName() << "? (Y/N): ";
    cin >> confirm;

    if (confirm == 'Y' || confirm == 'y') {
        sellHouseOnTile(player, selectedTile);
    }
    else {
        cout << player.getName() << " did not sell the house." << endl;
    }
}

vector<Tile*> Game::getOwnedTilesWithHouses(Player& player) {
    vector<Tile*> ownedTiles;
    for (Tile& tile : board->getBoard()) {
        if (tile.getOwnerName() == player.getName() && tile.getNumHouses() > 0) {
            ownedTiles.push_back(&tile);
        }
    }
    return ownedTiles;
}

void Game::displayOwnedTiles(Player& player, const vector<Tile*>& ownedTiles) {
    cout << player.getName() << ", select a property to sell a house from:" << endl;
    for (size_t i = 0; i < ownedTiles.size(); ++i) {
        cout << i + 1 << ": " << ownedTiles[i]->getName()
            << " (Houses: " << ownedTiles[i]->getNumHouses() << ")" << endl;
    }
}

int Game::getPlayerChoice(size_t numChoices) {
    int choice;
    cin >> choice;
    if (cin.fail() || choice <= 0 || choice > static_cast<int>(numChoices)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
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
        cout << player.getName() << " sold a house on " << tile->getName()
            << " for $" << sellPrice << endl;
    }
    else {
        cout << "No houses to sell on " << tile->getName() << endl;
    }
}