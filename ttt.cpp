#include <iostream>
#include <string>
#include <unistd.h>
#include <ostream>
using namespace std;

void printBoard(int board[9]) {
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            printf(" ");
        } else if (board[i] == 1) {
            printf("O");
        } else {
            printf("X");
        }
        if (i % 3 == 2) {
            printf("\n");
        } else {
            printf("|");
        }
    }
}


int checkWinner(int board[9]){
    for (int i = 0; i < 3; i++) {
        if (board[i] != 0 && board[i] == board[i + 3] && board[i] == board[i + 6]) {
            return board[i];
        }
        if (board[i * 3] != 0 && board[i * 3] == board[i * 3 + 1] && board[i * 3] == board[i * 3 + 2]) {
            return board[i * 3];
        }
    }
    if (board[0] != 0 && board[0] == board[4] && board[0] == board[8]) {
        return board[0];
    }
    if (board[2] != 0 && board[2] == board[4] && board[2] == board[6]) {
        return board[2];
    }
    return 0;

}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <name>" << endl;
        return 1;
    }
    char* temp = argv[1];
    string num = temp;
    if(num.length() != 9)
    {
        cerr << "Usage: " << argv[0] << " <name>" << endl;
        return 1;
    }
    bool arr[9] = {false};
    for(int i = 0; i < 9; i++)
    {
        if (arr[num[i] - '1'] == true) {
            cout << "Invalid input" << endl;
            return 1;
        }
        arr[num[i] - '1'] = true;
    }
    int board[9] = {0};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[num[j] - '1'] == 0) {
                cout << "BOT move: " << num[j] << endl;
                board[num[j] - '1'] = 1;
                break;
            }
        }
        printBoard(board);
        if (checkWinner(board) == -1) {
            cout << "BOT win!" << endl;
            return 0;
        }
        if (checkWinner(board) == 1) {
            cout << "Player win!" << endl;
            return 0;
        }
        cout << "Player move: ";
        int move;
        scanf("%d", &move);
        cout << move << endl;
        if (move < 1 || move > 9 || board[move - 1] != 0) {
            cerr << "Invalid move!" << endl;
            return 1;
        }
        board[move - 1] = 2;
        printBoard(board);
        if (checkWinner(board) == 1) {
            cout << "BOT win!" << endl;
            return 0;
        }
        if (checkWinner(board) == 2) {
            cout << "Player win!" << endl;
            return 0;
        }
    }

    cout << "Draw!" << endl;
    return 0;
}

