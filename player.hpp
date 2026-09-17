#pragma once
#include "board.hpp"
class Player{
    private:
    // Each player has a symbol (X or O) and a score that keeps track of the number of games won.
        char _symbol;
        int _score = 0;
    public:
        Player(char sym) : _symbol(sym) {}
        // Function to make a move on the board at the specified position
        void move(Board& board, int position) {
            board.updateBoard(position, _symbol);
        }
        // Getter for the player's symbol
        char getSymbol() {
            return _symbol;
        }
        // Function to increment the player's score when they win a game
        void incrementScore() {
            _score++;
        }
        // Getter for the player's score
        int getScore() {
            return _score;
        }
};