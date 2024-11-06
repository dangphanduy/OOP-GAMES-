#include "PlayerInput.h"
#include <iostream>

std::vector<std::string> getPlayerNames() {
    int numPlayers;
    do {
        std::cout << "Nhap so luong nguoi choi (2-4): ";
        std::cin >> numPlayers;
        if (numPlayers < 2 || numPlayers > 4) {
            std::cout << "So luong nguoi choi khong hop le!\n";
        }
    } while (numPlayers < 2 || numPlayers > 4);

    std::vector<std::string> playerNames;
    for (int i = 0; i < numPlayers; ++i) {
        std::string name;
        std::cout << "Nhap ten nguoi choi thu " << i + 1 << ": ";
        std::cin >> name;
        playerNames.push_back(name);
    }
    return playerNames;
}