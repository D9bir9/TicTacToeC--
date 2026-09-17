#include "game.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 3) {
        // If player names are provided as command line arguments, use them
        Game game('X', 'O', argv[1], argv[2]);
        game.start();
    } else {
        // If no player names are provided, use default names
        std::cout << "No player names provided. Using default names: Player 1 and Player 2." << std::endl;
        std::cout << "To provide player names, run the program with two arguments: " << argv[0] << " <Player1Name> <Player2Name>" << std::endl;
        Game game('X', 'O');
        game.start();
    }
    return 0;
}