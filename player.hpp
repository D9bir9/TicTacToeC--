#pragma once
#include "board.hpp"
class Player{
    private:
        char _symbol;
        int _score = 0;
    public:
        Player(char sym) : _symbol(sym) {}
        void move(Board& board, int position) {
            board.updateBoard(position, _symbol);
        }
        char getSymbol() {
            return _symbol;
        }
        void incrementScore() {
            _score++;
        }
        int getScore() {
            return _score;
        }
};