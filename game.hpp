#pragma once

#include <cctype>
#include <limits>

#include "player.hpp"
#include "board.hpp"

class Game {
    private:
    // Game is composed of a board and two players. The currentPlayer variable keeps track of whose turn it is.
        Board board;
        Player player1;
        Player player2;
        int currentPlayer = 1;

        // Function to read a valid move from the player
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

        // Function to read the player's choice to replay or not
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
        // Constructor to initialize the game with two players and their respective symbols
        Game(char sym1, char sym2) : player1(sym1), player2(sym2) {}

        // Function to start the game loop, it contains the main logic for player turns, checking for wins or draws, and handling replay.
        void start() {
            while (true) {
                // Display the positions for the players
                board.showPositions();
                std::cout << "\nCurrent Board:\n" << std::endl;
                // Render the current state of the board
                board.renderBoard();

                // Read a valid move from the current player
                int position = readValidMove();

                // Update the board with the current player's move and switch turns if the move was valid
                if (currentPlayer == 1) {
                    player1.move(board, position);
                    if (!board.replay) currentPlayer = 2;
                } else {
                    player2.move(board, position);
                    if (!board.replay) currentPlayer = 1;
                }


                // Check for wins or draws after each move
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

            // Display the scores of both players after the game ends
            std::cout << "Scores:\nPlayer 1: " << player1.getScore() << "\nPlayer 2: " << player2.getScore() << std::endl;

            // Prompt the players to decide if they want to play again
            char choice = readReplayChoice();
            if (choice == 'y') {
                board.resetBoard(); // Reset the board for a new game
                currentPlayer = 2; // Reset to player 2 for the next game, switch starting player for fairness
                start(); // Start a new game
            }
        }
};