#include "game.hpp"

int main(int argc, char* argv[]) {
    if (argc == 3) {
        // If player names are provided as command line arguments, use them
        Game game('X', 'O', argv[1], argv[2]);
        game.start();
    } else {
        // If no player names are provided, use default names
        Game game('X', 'O');
        game.start();
    }
    return 0;
}