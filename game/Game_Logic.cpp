#include "Game/Game.h"
#include "Player/Player.h"
#include "Main/Constants.h"
#include "Board/Board.h"
#include "Board/Tile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <random>

using namespace std;

random_device rd;
mt19937 gen(rd());

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();
    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    while (true) {
        frameStart = SDL_GetTicks();

        handleEvents();

        Player& currentPlayer = players[currentPlayerIndex];
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        update(deltaTime);
        render();

        frameTime = SDL_GetTicks() - frameStart;

        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
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
        cout << "Time's up!" << endl;
        nextTurn();
        return;
    }
    // Xử lý logic Lost Island trong hàm update
    if (currentPlayer.getIsOnLostIsland()) {
        cout << currentPlayer.getName() << " is stuck on Lost Island." << endl;
        currentPlayer.setTurnsOnLostIsland(currentPlayer.getTurnsOnLostIsland() + 1);

        if (currentPlayer.getTurnsOnLostIsland() == 1 && hasRolledDoubles()) {
            currentPlayer.setIsOnLostIsland(false);
            currentPlayer.setTurnsOnLostIsland(0);
            cout << currentPlayer.getName() << " escapes Lost Island!" << endl;
        }
        else if (currentPlayer.getTurnsOnLostIsland() >= 3) {
            currentPlayer.setIsOnLostIsland(false);
            currentPlayer.setTurnsOnLostIsland(0);
            cout << currentPlayer.getName() << " escapes Lost Island!" << endl;
        }
        else {
            // Người chơi vẫn bị mắc kẹt, chuyển lượt
            nextTurn();
            return;
        }
    }

    // Nếu player di chuyển, cập nhật vị trí của họ
    if (currentPlayer.getIsMoving()) {
        currentPlayer.updatePosition(deltaTime, players);
        // Kiểm tra xem player đã đến đích chưa
        if (!currentPlayer.getIsMoving()) {

            // Kích hoạt sự kiện nếu có
            Tile& landedTile = board->getBoard()[currentPlayer.getPosition()];
            landedTile.triggerOnLand(&currentPlayer);

            // Kiểm tra xem ô đất có thuộc sở hữu của người chơi khác hay không
            if (landedTile.getTileType() == TileType::PROPERTY &&
                !landedTile.getOwnerName().empty() &&
                landedTile.getOwnerName() != currentPlayer.getName()) {

                // Tính toán số tiền cần trả
                int rent = landedTile.getHousePrice() * 0.2;
                // Trừ tiền người chơi hiện tại
                currentPlayer.setMoney(currentPlayer.getMoney() - rent);
                // Cộng tiền cho chủ sở hữu
                Player& owner = getPlayer(landedTile.getOwnerName());
                owner.setMoney(owner.getMoney() + rent);

                cout << currentPlayer.getName() << " paid $" << rent << " to " << owner.getName() << " for landing on " << landedTile.getName() << endl;
            }
            // Kiểm tra trạng thái của người chơi
            checkAndHandleBankruptcy(currentPlayer);
        }
    }
}

int Game::rollDice() {
    uniform_int_distribution<> dis(1, 6);
    int dice = dis(gen);

    // Cập nhật các biến lưu trữ kết quả
    auto currentPlayer = players[currentPlayerIndex];
    currentPlayer.setPreviousRoll(currentPlayer.getCurrentRoll());
    currentPlayer.setCurrentRoll(dice);

    return dice;
}

void Game::handleRollDice(Player& currentPlayer) {
    if (!currentPlayer.getIsMoving() && currentPlayer.getCanRollDice()) {
        int diceRoll = rollDice();
        cout << "\n" << currentPlayer.getName() << " rolled a " << diceRoll << endl;
        currentPlayer.move(diceRoll, board->getBoard());
        currentPlayer.setIsMoving(true);
        currentPlayer.setCanRollDice(false);
    }
    else {
        cout << currentPlayer.getName() << " has already rolled the dice." << endl;
    }
}

void Game::nextTurn() {
    do {
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    } while (players[currentPlayerIndex].getState() == PlayerState::Bankrupt); // Bỏ qua người chơi đã phá sản

    Player& currentPlayer = players[currentPlayerIndex];
    cout << currentPlayer.getName() << "'s turn." << endl;
    currentPlayer.setCanRollDice(true);
    turnStartTime = SDL_GetTicks(); // Reset thời gian bắt đầu lượt
}

bool Game::hasRolledDoubles() {
    uniform_int_distribution<> dis(1, 6);
    int dice1 = dis(gen);
    int dice2 = dis(gen);

    cout << "Player rolled: " << dice1 << " and " << dice2 << endl;
    return dice1 == dice2;  // Trả về true nếu lăn được "doubles"
}

void Game::checkAndHandleBankruptcy(Player& player) {
    if (player.getMoney() <= 0) {
        player.setState(PlayerState::Bankrupt);
        cout << player.getName() << " is bankrupt!" << endl;
        handleBankruptcy(player);

        // Kiểm tra xem có phải người chơi cuối cùng không
        __int64 numPlayersRemaining = count_if(
            players.begin(),
            players.end(),
            [](const Player& p) { return p.getState() != PlayerState::Bankrupt; }
        );

        if (numPlayersRemaining == 1) {
            // Tìm người chơi chiến thắng
            Player& winner = *find_if(
                players.begin(),
                players.end(),
                [](const Player& p) { return p.getState() != PlayerState::Bankrupt; }
            );

            cout << winner.getName() << " is the last player standing and wins the game!" << endl;
            exit(0); // Kết thúc trò chơi
        }
    }
}

void Game::handleBankruptcy(Player& player) {
    // Lấy danh sách tài sản của người chơi
    unordered_set<Tile*> properties = player.getProperties();

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