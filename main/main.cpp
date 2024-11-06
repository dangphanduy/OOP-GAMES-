#include "Game.h"
#include "PlayerInput.h"
#include <iostream>
#include <iomanip>

using namespace std;

void displayInstructions() {
    cout << "============================= CACH CHOI ===============================\n";
    cout << "| Phim | Chuc nang                                                    |\n";
    cout << "|------|--------------------------------------------------------------|\n";
    cout << "| Space| Do xuc xac                                                   |\n";
    cout << "| b    | Mua bãi biển                                                 |\n";
    cout << "| h    | Mua nha                                                      |\n";
    cout << "| l    | Ban nha                                                      |\n";
    cout << "| ESC  | Thoat khoi tro choi                                          |\n";
    cout << "========================================================================\n";
    cout << "\nCac o dac biet:\n";
    cout << "--------------------------------------------\n";
    cout << "1. Bat dau:  Nhan tien khi di qua.\n";
    cout << "2. Dao Hoang (Lost Island) : bi mac ket trong 3 luot hoac cho den khi do xuc xac duoc doi.\n";
    cout << "3. Worlds:  Nhan doi gia tri cua mot o dat ban so huu.\n";
    cout << "4. Du lich vong quanh the gioi: Chon mot o dat ma ban so huu de di chuyen den.\n";
    cout << "5. Co hoi:  Rut mot the bai co hoi.\n";
    cout << "6. Thue:  Tra thue dua tren tong gia tri nha cua ban.\n";
    cout << "--------------------------------------------\n";
    cout << "\nCach thang:\n";
    cout << "--------------------------------------------\n";
    cout << "1. So huu tat ca cac bai bien.\n";
    cout << "2. La nguoi choi cuoi cung con lai khong bi pha san.\n";
    cout << "--------------------------------------------\n";
}

int main(int argc, char* argv[]) {
    displayInstructions();

    std::vector<std::string> playerNames = getPlayerNames();

    Game game(playerNames);
    game.run();
    return 0;
}