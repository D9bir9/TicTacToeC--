#pragma once

#include "player.hpp"

class Game {
    private:
        Board board;
        Player player1;
        Player player2;
        int currentPlayer = 1;

    public:
        Game(char sym1, char sym2) : player1(sym1), player2(sym2) {}

        void start() {
            int position;
            play:
            while (true) {
                board.showPositions();
                std::cout << "\nCurrent Board:\n" << std::endl;
                board.renderBoard();
                std::cout << "Player " << currentPlayer << ", enter your move (1-9): ";
                std::cin >> position;

                if (currentPlayer == 1) {
                    player1.move(board, position);
                    if(!board.replay) currentPlayer = 2;
                } else {
                    player2.move(board, position);
                    if(!board.replay) currentPlayer = 1;
                }

                if (board.checkWin(player1.getSymbol())) {
                    board.renderBoard();
                    std::cout << "Player 1 wins!" << std::endl;
                    player1.incrementScore();
                    break;
                } else if (board.checkWin(player2.getSymbol())) {
                    board.renderBoard();
                    std::cout << "Player 2 wins!" << std::endl;
                    player2.incrementScore();
                    break;
                }
                else if (board.isFull()) {
                    board.renderBoard();
                    std::cout << "It's a draw!" << std::endl;
                    break;
                }
            }
            std::cout << "Scores:\nPlayer 1: " << player1.getScore() << "\nPlayer 2: " << player2.getScore() << std::endl;
            std::cout << "Do you want to play again? (y/N): ";
            char choice;
            std::cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                board = Board(); // Reset the board
                currentPlayer = 1; // Reset to player 1
                goto play; // Restart the game
            }
        }
};