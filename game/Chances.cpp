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

void Game::setupChanceEvents() {
    chanceEvents = {
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Advanced to Start." << endl;
            player.setPosition(0);
            player.setMoney(player.getMoney() + 200);
            player.setX(0);
            player.setY(0);
            player.setIsMoving(true);
            player.updateTargetPosition();
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Go to Lost IsLand." << endl;
            player.setPosition(8);
            player.setIsOnLostIsland(true);
            player.setX(712);
            player.setY(0);
            player.setIsMoving(true);
            player.updateTargetPosition();
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Advanced To Free Parking" << endl;
            player.setPosition(16);

            player.setIsMoving(true);
            player.setX(712);
            player.setY(712);
            player.updateTargetPosition();
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Advanced To Worlds" << endl;
            player.setPosition(24);

            player.setIsMoving(true);
            player.setX(0);
            player.setY(712);
            player.updateTargetPosition();
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Go back" << endl;
            uniform_int_distribution<> distrib(1, 6);
            random_device rd;
            mt19937 gen(rd());
            int stepsBack = distrib(gen);

            int newPosition = (player.getPosition() - stepsBack + NUM_TILES) % NUM_TILES;

            player.setPosition(newPosition);


            player.setIsMoving(true);
            player.updateTargetPosition();
            cout << player.getName() << " moved back to " << player.getPosition() << endl;
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Go forward, but not past GO!" << endl;
            uniform_int_distribution<> distrib(1, 6);
            random_device rd;
            mt19937 gen(rd());
            int stepsForward = distrib(gen);

            int newPosition = (player.getPosition() + stepsForward + NUM_TILES) % NUM_TILES;

            if (newPosition < NUM_TILES) {
                player.setPosition(newPosition);

                player.setIsMoving(true);
                player.updateTargetPosition();

                cout << player.getName() << " moved back to " << player.getPosition() << endl;
            }
            else {
                cout << player.getName() << " stays put." << endl;
            }
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Make general repairs on all your property." << endl;
            int repairCost = 75; // Chi phí sửa chữa cho mỗi người chơi khác
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - repairCost);
                }
            }
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Pay fine of 50." << endl;
            player.setMoney(player.getMoney() - 50);
            checkAndHandleBankruptcy(player);
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Happy Birthday!" << endl;
            int birthdayMoney = 50;
            for (auto& otherPlayer : players) {
                if (otherPlayer != &player) {
                    otherPlayer->setMoney(otherPlayer->getMoney() - birthdayMoney);
                    player.setMoney(player.getMoney() + birthdayMoney);
                }
            }
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Bank pays you dividend of $50." << endl;
            player.setMoney(player.getMoney() + 50);
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: A map" << endl;
            player.setHasMap(true);
            cout << player.getName() << " now has a map to escape Lost Island!" << endl;
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Pay poor tax of $75." << endl;
            player.setMoney(player.getMoney() - 75);
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: Your building and loan matures." << endl;
            player.setMoney(player.getMoney() + 150);
        },
        [&](Player& player, vector<Player*>& players) {
            cout << player.getName() << " draws a Chance card: You have won a crossword competition." << endl;
            player.setMoney(player.getMoney() + 100);
        },
    };
}