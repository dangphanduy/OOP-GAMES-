#include "Game.h"
#include "Player.h"
#include "Constants.h"
#include "Board.h"
#include "Tile.h"
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

using namespace std;

void Game::initSDL() {
    // Khởi tạo SDL
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
}

SDL_Texture* Game::loadTexture(const string& path, SDL_Renderer* renderer) {
    SDL_Texture* texture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());

    if (loadedSurface == nullptr) {
        cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << endl;
        // Thêm texture mặc định nếu có
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 100, 100); // Texture mặc định kích thước 100x100
    }
    else {
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
    }

    return texture;
}

void Game::cleanup() {
    // Destroy player textures
    for (const auto& player : players) {
        if (player.getSprite()) {
            SDL_DestroyTexture(player.getSprite());
        }
    }
    // Destroy house texture
    for (const auto& player : players) {
        if (player.getHouseTexture()) {
            SDL_DestroyTexture(player.getHouseTexture());
        }
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

void Game::drawPlayer(SDL_Texture* texture, int x, int y) {
    SDL_Rect playerRect = { x, y, spriteWidth, spriteHeight };
    if (SDL_RenderCopy(renderer, texture, nullptr, &playerRect) != 0) {
        cerr << "SDL_RenderCopy error: " << SDL_GetError() << endl;
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

void Game::renderText(const string& message, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, message.c_str(), color);
    if (!surface) {
        cerr << "Error rendering text: " << TTF_GetError() << endl;
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

        if (i == currentPlayerIndex) {
            Uint32 timeLeft = TURN_TIME_LIMIT - (SDL_GetTicks() - turnStartTime);
            string timeLeftText = "Time Left: " + to_string(timeLeft / 1000) + "s"; // Hiển thị thời gian theo giây
            renderText(timeLeftText, tableX + 5, tableY + (i + 5) * cellHeight + 5, textColor); // Hiển thị dưới các thông tin khác
        }

        // Vẽ hình chữ nhật cho tên người chơi
        SDL_Rect nameRect = { tableX, tableY + i * cellHeight, cellWidth, cellHeight };
        SDL_SetRenderDrawColor(renderer, tableColor.r, tableColor.g, tableColor.b, tableColor.a);
        SDL_RenderFillRect(renderer, &nameRect);

        // Hiển thị tên người chơi
        renderText(players[i].getName(), tableX + 5, tableY + i * cellHeight + 5, textColor);

        // Hiển thị tiền của người chơi
        string moneyText = "Money: $" + to_string(players[i].getMoney());
        renderText(moneyText, tableX + 5, tableY + (i + 1) * cellHeight + 5, textColor);
        // Hiển thị vị trí của người chơi
        string positionText = "Position: " + to_string(players[i].getPosition());
        renderText(positionText, tableX + 5, tableY + (i + 2) * cellHeight + 5, textColor);
        // Hiển thị số nhà người chơi sở hữu
        int numHouses = 0;
        for (const Tile* tile : players[i].getOwnedProperties()) {
            numHouses += tile->getNumHouses();
        }
        string houseText = "Houses: " + to_string(numHouses);
        renderText(houseText, tableX + 5, tableY + (i + 3) * cellHeight + 5, textColor);
        // Hiển thị số bãi biển người chơi sở hữu
        int numBeaches = 0;
        for (const Tile* property : players[i].getOwnedProperties()) {
            if (property->getTileType() == TileType::BEACH) {
                numBeaches++;
            }
        }
        string beachText = "Beaches: " + to_string(numBeaches);
        renderText(beachText, tableX + 5, tableY + (i + 4) * cellHeight + 5, textColor);
    }
}

