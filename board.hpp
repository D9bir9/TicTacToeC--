#pragma once
#include <iostream>

class Board {
    private:
        char board[3][3] = {
            {' ', ' ', ' '},
            {' ', ' ', ' '},
            {' ', ' ', ' '}
        };
    
    public:
        bool replay = false;
        void showPositions() {
            for (int i = 0; i < 3; i++) {
                std::cout << "|";
                for (int j = 0; j < 3; j++) {
                    std::cout << " " << i * 3 + j + 1 << " |";  
                }
                std::cout << std::endl;
            }
        }

        void renderBoard() {
            for (int i = 0; i < 3; i++) {
                std::cout << "|";
                for (int j = 0; j < 3; j++) {
                    std::cout << " " << board[i][j] << " |";  
                }
                std::cout << std::endl;
            }
        }

        void updateBoard(int position, char player) {
            if (position < 1 || position > 9) {
                std::cout << "Invalid position! Please choose a number between 1 and 9." << std::endl;
                replay = true;
                return;
            }
            int row = (position - 1) / 3;
            int col = (position - 1) % 3;
            if (board[row][col] == ' ') {
                board[row][col] = player;
                replay = false;
            } else {
                std::cout << "Position already taken!" << std::endl;
                replay = true;
            }
        }

        bool checkWin(char player) {
            // Check rows and columns
            for (int i = 0; i < 3; i++) {
                if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
                    (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
                    return true;
                }
            }
            // Check diagonals
            if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
                (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
                return true;
            }
            return false;
        }

        bool isFull() {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == ' ') {
                        return false;
                    }
                }
            }
            return true;
        }

        void resetBoard() {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    board[i][j] = ' ';
                }
            }
        }
};