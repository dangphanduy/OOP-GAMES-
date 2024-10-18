#include "Game.h"
#include <iostream>

using namespace std;

void displayInstructions() {
    cout << "============================= HOW TO PLAY ===============================\n";
    cout << "Space. do xuc xac\n";
    cout << "b. mua bai bien\n";
    cout << "h. mua nha\n";
    cout << "l. ban nha\n";
    cout << "=========================================================================\n";
}

int main(int argc, char* argv[]) {
    displayInstructions();
    int numPlayers;
    do {
        cout << "Nhap so luong nguoi choi (2-4): ";
        cin >> numPlayers;
        if (numPlayers < 2 || numPlayers > 4) cout << "So luong nguoi choi khong hop le!\n";
    } while (numPlayers < 2 || numPlayers > 4);

    vector<string> playerNames;
    for (int i = 0; i < numPlayers; ++i) {
        string name;
        cout << "Nhap ten nguoi choi thu " << i + 1 << ": ";
        cin >> name;
        playerNames.push_back(name);
    }

    Game game(playerNames);
    game.run();
    return 0;
}