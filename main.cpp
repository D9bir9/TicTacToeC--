#include "game.hpp"

int main() {
    // Create a Game Instance with player symbols 'X' and 'O', and start the game loop
    Game game('X', 'O');
    game.start();
    return 0;
}