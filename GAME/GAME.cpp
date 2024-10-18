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

Game::Game(const std::vector<std::string>& playerNames) : currentPlayerIndex(0) {
    initSDL();
    board = new Board(this); // Khởi tạo Board và truyền con trỏ đến Game
    board->createBoard(); // Gọi phương thức tạo bảng
    for (const auto& name : playerNames) {
        players.emplace_back(name, 1500);
    }
    initPlayers();
    currentPlayerIndex = 0;
    setupChanceEvents();
}

Game::~Game() {
    if (board != nullptr) {
        delete board;
    }
    cleanup();
}

void Game::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        exit(1);
    }
    // Khởi tạo SDL_ttf
    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << endl;
        exit(1);
    }
    // Tạo cửa sổ và renderer
    window = SDL_CreateWindow("Monopoly", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    // Tải phông chữ
    font = TTF_OpenFont("WorkSans-Italic.ttf", 15); // Đường dẫn tới phông chữ
    if (font == nullptr) {
        cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << endl;
    }
    // Tải các textures khác
    houseTexture = loadTexture(HOUSE_TEXTURE_PATH, renderer);
    if (houseTexture == nullptr) {
        cerr << "Failed to load house texture!" << endl;
    }
}

void Game::initPlayers() {
    std::vector<std::string> playerTextures = { "Player1.png", "Player2.png", "Player3.png", "Player4.png" };

    // Giả sử mỗi người chơi bắt đầu ở một ô riêng biệt
    int startingPositions = 0; // Ví dụ: 4 người chơi bắt đầu ở 4 ô đầu tiên

    for (int i = 0; i < players.size(); ++i) {
        // Tải hình ảnh người chơi thành SDL_Texture
        SDL_Texture* playerTexture = loadTexture(playerTextures[i], renderer);
        players[i].setSprite(playerTexture);

        // Thêm người chơi vào ô khởi đầu tương ứng
        board->getBoard()[startingPositions].addPlayer(&players[i]);

        // Cập nhật vị trí của người chơi
        players[i].setPosition(startingPositions);
    }
}

void Game::drawPlayer(SDL_Texture* texture, int x, int y) {
    SDL_Rect playerRect = { x, y, spriteWidth, spriteHeight };
    if (SDL_RenderCopy(renderer, texture, nullptr, &playerRect) != 0) {
        std::cerr << "SDL_RenderCopy error: " << SDL_GetError() << std::endl;
    }
}

void Game::quitGame() {
    isRunning = false;
    SDL_Quit();
}

void Game::setupChanceEvents() {
    chanceEvents = {
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Move to Start." << std::endl;
            player.setPosition(0);
            player.setMoney(player.getMoney() + 200);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Pay fine of 50." << std::endl;
            player.setMoney(player.getMoney() - 50);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Happy Birthday!" << std::endl;
            // Thêm logic để thu tiền từ mỗi người chơi (ví dụ: $50)
            int birthdayMoney = 50;
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - birthdayMoney);
                    player.setMoney(player.getMoney() + birthdayMoney);
                }
            }
        },
        [=](Player& player, std::vector<Player*>& players) { // Capture players by reference
            std::cout << player.getName() << " draws a Chance card: Go to Lost IsLand." << std::endl;
            player.setPosition(8);
            player.setIsOnLostIsland(true);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
        },
        [=](Player& player, std::vector<Player*>& players) { // Capture players by reference
            std::cout << player.getName() << " draws a Chance card: Advance to the nearest Beach" << std::endl;
            
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Advance to the World Tour" << std::endl;
            player.setPosition(24);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
            std::cout << player.getName() << " moved to World Tour" << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: Bank pays you dividend of $50." << std::endl;
            player.setMoney(player.getMoney() + 50);
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: A map which is can help you a lots..." << std::endl;
            // Thêm logic cho thẻ "A map which is can help you a lots..."
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
            int repairCost = 50; // Chi phí sửa chữa cho mỗi người chơi khác
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
            std::cout << player.getName() << " draws a Chance card: ADVANCED TO WORLDS" << std::endl;
            player.setPosition(16);
            player.updateTargetPosition();
            player.setTargetPosition(player.getX(), player.getY());
            std::cout << player.getName() << " moved to Worlds" << std::endl;
        },
        [=](Player& player, std::vector<Player*>& players) {
            std::cout << player.getName() << " draws a Chance card: This is Your Birthday!" << std::endl;
            // Thêm logic để thu tiền từ mỗi người chơi (ví dụ: $50)
            int birthdayMoney = 50;
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - birthdayMoney);
                    player.setMoney(player.getMoney() + birthdayMoney);
                }
            }
        },
        [=](Player& player, std::vector<Player*>& players) { // Capture players by reference
            std::cout << player.getName() << " draws a Chance card: Your building and loan matures." << std::endl;
            player.setMoney(player.getMoney() + 150);
        },
        [=](Player & player, std::vector<Player*>& players) { // Capture players by reference
            std::cout << player.getName() << " draws a Chance card: You have won a crossword competition." << std::endl;
            player.setMoney(player.getMoney() + 100);
        },
    };
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

bool Game::hasRolledDoubles() {
    std::uniform_int_distribution<> dis(1, 6);
    int dice1 = dis(gen);
    int dice2 = dis(gen);

    std::cout << "Player rolled: " << dice1 << " and " << dice2 << std::endl;
    return dice1 == dice2;  // Trả về true nếu lăn được "doubles"
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
        }
        currentPlayer.setIsMoving(true);
        currentPlayer.setCanRollDice(false);
    }
    else {
        std::cout << currentPlayer.getName() << " is still moving or cannot roll dice yet." << std::endl;
    }
}

void Game::nextTurn() {
    do {
        currentPlayerIndex = (currentPlayerIndex + 1) % players.size();
    } while (players[currentPlayerIndex].getState() == PlayerState::Bankrupt); // Bỏ qua người chơi đã phá sản

    Player& currentPlayer = players[currentPlayerIndex];
    std::cout << currentPlayer.getName() << "'s turn." << std::endl;
}

void Game::buyHouse(Tile& tile) {
    Player& currentPlayer = players[currentPlayerIndex];

    // Kiểm tra loại ô đất
    if (tile.getTileType() != TileType::PROPERTY) {
        std::cout << "You can only buy a house on regular plots." << std::endl;
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
        currentPlayer.setMoney(currentPlayer.getMoney() - tile.getHousePrice());
        tile.setNumHouses(tile.getNumHouses() + 1);
        std::cout << "You have successfully purchased a house on " << tile.getName() << "." << std::endl;
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
            player.setMoney(player.getMoney() - BEACH_COST);
            tile.setOwnerName(player.getName());
            player.addProperty(&tile);
            std::cout << player.getName() << " bought the beach: " << tile.getName() << std::endl;

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
    sellHouseOnTile(player, selectedTile);
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

void Game::update(float deltaTime) {
    Player& currentPlayer = players[currentPlayerIndex];

    // Nếu player di chuyển, cập nhật vị trí của họ
    if (currentPlayer.getIsMoving()) {
        currentPlayer.updatePosition(deltaTime, players);

        // Kiểm tra xem player đã đến đích chưa
        if (!currentPlayer.getIsMoving()) {
            std::cout << currentPlayer.getName() << " has reached the target position." << std::endl;

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

            // Kiểm tra xem người chơi có bị phá sản sau khi di chuyển hay không
            if (currentPlayer.getMoney() < 0) {
                currentPlayer.setState(PlayerState::Bankrupt);
                std::cout << currentPlayer.getName() << " is bankrupt!" << std::endl;
            }

            // Chuyển lượt người chơi
            nextTurn();
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    board->renderBoards();
    board->renderPlayers();
    renderPlayerInfo();
    SDL_RenderPresent(renderer);
}

void Game::renderText(const std::string& message, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!surface) {
        std::cerr << "Error rendering text: " << TTF_GetError() << std::endl;
        return;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int width = surface->w;
    int height = surface->h;

    SDL_Rect renderQuad = { x, y, width, height };
    SDL_RenderCopy(renderer, texture, nullptr, &renderQuad);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void Game::renderPlayerInfo() {
    // Kích thước mỗi ô trong bảng
    int cellWidth = 200;
    int cellHeight = 30;

    SDL_Color textColor = { 255, 255, 255, 255 }; // Trắng
    SDL_Color tableColor = { 0, 0, 0, 255 }; // Đen

    // Tọa độ X cho mỗi bảng
    int tableXPositions[4] = { 100, 100, 500, 500 };
    int tableYPositions[4] = { 100, 400, 40, 340 };

    for (int i = 0; i < players.size(); ++i) {
        // Vị trí bắt đầu của bảng thông tin cho mỗi người chơi
        int tableX = tableXPositions[i];
        int tableY = tableYPositions[i];

        // Vẽ hình chữ nhật cho tên người chơi
        SDL_Rect nameRect = { tableX, tableY + i * cellHeight, cellWidth, cellHeight };
        SDL_SetRenderDrawColor(renderer, tableColor.r, tableColor.g, tableColor.b, tableColor.a);
        SDL_RenderFillRect(renderer, &nameRect);

        // Hiển thị tên người chơi
        renderText(players[i].getName(), tableX + 5, tableY + i * cellHeight + 5, textColor);

        // Hiển thị tiền của người chơi
        std::string moneyText = "Money: $" + std::to_string(players[i].getMoney());
        renderText(moneyText, tableX + 5, tableY + (i + 1) * cellHeight + 5, textColor);
        // Hiển thị vị trí của người chơi
        std::string positionText = "Position: " + std::to_string(players[i].getPosition());
        renderText(positionText, tableX + 5, tableY + (i + 2) * cellHeight + 5, textColor);
        // Hiển thị số nhà người chơi sở hữu
        int numHouses = 0;
        for (const Tile* tile : players[i].getOwnedProperties()) {
            numHouses += tile->getNumHouses();
        }
        std::string houseText = "Houses: " + std::to_string(numHouses);
        renderText(houseText, tableX + 5, tableY + (i + 3) * cellHeight + 5, textColor);
        // Hiển thị số bãi biển người chơi sở hữu
        int numBeaches = 0;
        for (const Tile* property : players[i].getOwnedProperties()) {
            if (property->getTileType() == TileType::BEACH) {
                numBeaches++;
            }
        }
        std::string beachText = "Beaches: " + std::to_string(numBeaches);
        renderText(beachText, tableX + 5, tableY + (i + 4) * cellHeight + 5, textColor);
    }
}

void Game::cleanup() {
    // Destroy player textures
    for (const auto& player : players) {
        if (player.getSprite()) {
            SDL_DestroyTexture(player.getSprite());
        }
    }

    // Destroy house texture
    if (houseTexture) {
        SDL_DestroyTexture(houseTexture);
        houseTexture = nullptr;
    }

    // Destroy renderer
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    // Destroy window
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    // Close font and quit TTF
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();

    // Quit SDL
    SDL_Quit();
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
    else if (key == SDLK_p) {
        currentPlayer.printRollHistory();
    }
}

SDL_Texture* Game::loadTexture(const std::string& path, SDL_Renderer* renderer) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());

    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        // Thêm texture mặc định nếu có
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 100, 100); // Texture mặc định kích thước 100x100
    }
    else {
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }

    return texture;
}

