#pragma once

#include <cctype>
#include <limits>

#include "player.hpp"
#include <limits>

class Game {
    private:
        Board board;
        Player player1;
        Player player2;
        int currentPlayer = 1;

        int readValidMove() {
            int position = 0;

            while (true) {
                std::cout << "Player " << currentPlayer << ", enter your move (1-9): ";
                if (!(std::cin >> position)) {
                    std::cout << "Invalid input! Please enter a number between 1 and 9." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                if (position < 1 || position > 9) {
                    std::cout << "Invalid position! Please choose a number between 1 and 9." << std::endl;
                    continue;
                }

                return position;
            }
        }

        char readReplayChoice() {
            char choice = 'n';

            while (true) {
                std::cout << "Do you want to play again? (y/N): ";
                if (!(std::cin >> choice)) {
                    std::cout << "Invalid input! Please enter 'y' or 'n'." << std::endl;
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    continue;
                }

                choice = std::tolower(static_cast<unsigned char>(choice));
                if (choice == 'y' || choice == 'n') {
                    return choice;
                }

                std::cout << "Please enter 'y' or 'n'." << std::endl;
            }
        }

    public:
        Game(char sym1, char sym2) : player1(sym1), player2(sym2) {}

        void start() {
            while (true) {
                board.showPositions();
                std::cout << "\nCurrent Board:\n" << std::endl;
                board.renderBoard();

                int position = readValidMove();


                if (currentPlayer == 1) {
                    player1.move(board, position);
                    if (!board.replay) currentPlayer = 2;
                } else {
                    player2.move(board, position);
                    if (!board.replay) currentPlayer = 1;
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
                } else if (board.isFull()) {
                    board.renderBoard();
                    std::cout << "It's a draw!" << std::endl;
                    break;
                }
            }

            std::cout << "Scores:\nPlayer 1: " << player1.getScore() << "\nPlayer 2: " << player2.getScore() << std::endl;

            char choice = readReplayChoice();
            if (choice == 'y') {
                board = Board();
                currentPlayer = 1;
                start();
            }
        }
};