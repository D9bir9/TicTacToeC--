#pragma once
#include <iostream>
#define EMPTY ' '

class Board {
    private:
    // 3x3 Game board initialized with empty spaces
        char board[3][3] = {
            {EMPTY, EMPTY, EMPTY},
            {EMPTY, EMPTY, EMPTY},
            {EMPTY, EMPTY, EMPTY}
        };
    
    public:
        // replay becomes true if the player tries to make a move in an already occupied position,
        // or if the player enters an invalid position. This will prompt the player to re-enter their move.
        bool replay = false;

        // Function to display the positions on the board for the players
        void showPositions() {
            for (int i = 0; i < 3; i++) {
                std::cout << "|";
                for (int j = 0; j < 3; j++) {
                    std::cout << " " << i * 3 + j + 1 << " |";  
                }
                std::cout << std::endl;
            }
        }

        // Function to render the current state of the board
        void renderBoard() {
            for (int i = 0; i < 3; i++) {
                std::cout << "|";
                for (int j = 0; j < 3; j++) {
                    std::cout << " " << board[i][j] << " |";  
                }
                std::cout << std::endl;
            }
        }

        // Function to update the board with the player's move
        void updateBoard(int position, char player) {
            if (position < 1 || position > 9) {
                std::cout << "Invalid position! Please choose a number between 1 and 9." << std::endl;
                replay = true;
                return;
            }
            int row = (position - 1) / 3;
            int col = (position - 1) % 3;
            if (board[row][col] == EMPTY) {
                board[row][col] = player;
                replay = false;
            } else {
                std::cout << "Position already taken!" << std::endl;
                replay = true;
            }
        }

        // Function to check if a player has won the game
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

        // Function to check if the board is full (i.e., a draw)
        bool isFull() {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (board[i][j] == EMPTY) {
                        return false;
                    }
                }
            }
            return true;
        }

        // Function to reset the board to its initial empty state
        void resetBoard() {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    board[i][j] = EMPTY;
                }
            }
        }
};