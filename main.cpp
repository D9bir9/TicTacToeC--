#include "game.hpp"
#include "multiplayer.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
    if (argc == 3 && std::string(argv[1]) == "--server") {
        MultiplayerServer server(static_cast<unsigned short>(std::stoi(argv[2])));
    } else if ((argc == 4 || argc == 5) && std::string(argv[1]) == "--connect") {
        runMultiplayerClient(argv[2], static_cast<unsigned short>(std::stoi(argv[3])),
                             argc == 5 ? argv[4] : "Player");
    } else if (argc == 3) {
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
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }
    return 0;
}