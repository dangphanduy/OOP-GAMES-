#include "Game/Game.h"
#include "Player/Player.h"
#include "Main/Constants.h"
#include "Board/Board.h"
#include "Board/Tile.h"
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
    for (Player& player : players) {
        player.render(renderer); // Gọi hàm render của từng người chơi
    }
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
    int cellWidth = 200;
    int cellHeight = 30;

    SDL_Color textColor = { 255, 255, 255, 255 }; // Màu trắng
    SDL_Color tableColor = { 0, 0, 0, 255 }; // Màu đen
    SDL_Color highlightColor = { 50, 50, 50, 255 }; // Màu nổi bật cho người chơi hiện tại

    int tableXPositions[4] = { 100, 100, 500, 500 };
    int tableYPositions[4] = { 100, 400, 100, 400 };

    for (int i = 0; i < players.size() && i < 4; ++i) {
        int tableX = tableXPositions[i];
        int tableY = tableYPositions[i];
        int lineOffset = 0;

        if (i == currentPlayerIndex) {
            Uint32 timeLeft = TURN_TIME_LIMIT - (SDL_GetTicks() - turnStartTime);
            string timeLeftText = "Time Left: " + to_string(timeLeft / 1000) + "s";
            renderText(timeLeftText, tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
            lineOffset++;
        }

        SDL_SetRenderDrawColor(renderer, (i == currentPlayerIndex) ? highlightColor.r : tableColor.r,
            (i == currentPlayerIndex) ? highlightColor.g : tableColor.g,
            (i == currentPlayerIndex) ? highlightColor.b : tableColor.b,
            tableColor.a);

        SDL_Rect nameRect = { tableX, tableY + lineOffset * cellHeight, cellWidth, cellHeight };
        SDL_RenderFillRect(renderer, &nameRect);

        renderText(players[i].getName(), tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
        lineOffset++;

        string moneyText = "Money: $" + to_string(players[i].getMoney());
        renderText(moneyText, tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
        lineOffset++;

        string positionText = "Position: " + to_string(players[i].getPosition());
        renderText(positionText, tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
        lineOffset++;

        int numBeaches = 0;
        for (const Tile* property : players[i].getOwnedProperty()) {
            if (property->getTileType() == TileType::BEACH) {
                numBeaches++;
            }
        }
        string beachText = "Beaches: " + to_string(numBeaches);
        renderText(beachText, tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
        lineOffset++;

        // Hiển thị tên của các ô đất đã sở hữu
        renderText("Properties:", tableX + 5, tableY + lineOffset * cellHeight + 5, textColor);
        lineOffset++;
        for (const Tile* property : players[i].getOwnedProperty()) { 
            renderText("- " + property->getName(), tableX + 10, tableY + lineOffset * cellHeight + 5, textColor);
            lineOffset++;
        }
    }
}



