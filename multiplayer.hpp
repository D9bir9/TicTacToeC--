#pragma once

#include "game.hpp"
#include "websocket.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

class MultiplayerServer {
    struct Client {
        std::shared_ptr<websocket::Connection> connection;
        std::string name;
        char symbol = 0;
    };
    Board board;
    std::vector<std::shared_ptr<Client>> clients;
    std::mutex mutex;
    int currentPlayer = 1;
    bool finished = false;

    std::string state() {
        std::string result = "STATE ";
        for (int i = 1; i <= 9; ++i) result += board.cell(i);
        result += " TURN " + std::to_string(currentPlayer);
        return result;
    }
    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex);
        for (auto& client : clients) client->connection->sendText(message);
    }
    void remove(const std::shared_ptr<Client>& client) {
        std::lock_guard<std::mutex> lock(mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }
    void handle(std::shared_ptr<websocket::Connection> connection) {
        if (!connection->handshake()) return;
        auto client = std::make_shared<Client>();
        client->connection = connection;
        {
            std::lock_guard<std::mutex> lock(mutex);
            clients.push_back(client);
        }
        connection->sendText("INFO Connected. Send JOIN <name> to enter the game.");
        std::string message;
        while (connection->receiveText(message)) {
            std::istringstream input(message);
            std::string command;
            input >> command;
            if (command == "JOIN") {
                input >> client->name;
                if (client->name.empty()) client->name = "Player";
                bool joined = false;
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    int playerCount = 0;
                    for (const auto& existing : clients) if (existing->symbol != 0) ++playerCount;
                    if (client->symbol == 0 && playerCount < 2) {
                        client->symbol = playerCount == 0 ? 'X' : 'O';
                        joined = true;
                        connection->sendText(std::string("WELCOME ") + client->symbol);
                        connection->sendText(state());
                    } else connection->sendText("INFO You are watching this game.");
                }
                if (joined) {
                    broadcast("INFO " + client->name + " joined the game.");
                    broadcast(state());
                }
            } else if (command == "MOVE") {
                int position;
                if (!(input >> position)) { connection->sendText("ERROR Move must be a number from 1 to 9."); continue; }
                std::string result;
                {
                std::lock_guard<std::mutex> lock(mutex);
                if (client->symbol == 0) { connection->sendText("ERROR Join as one of the two players first."); continue; }
                if (finished || ((client->symbol == 'X' && currentPlayer != 1) ||
                                 (client->symbol == 'O' && currentPlayer != 2))) {
                    connection->sendText("ERROR It is not your turn.");
                    continue;
                }
                board.updateBoard(position, client->symbol);
                if (board.replay) { connection->sendText("ERROR Invalid or occupied position."); continue; }
                currentPlayer = client->symbol == 'X' ? 2 : 1;
                if (board.checkWin(client->symbol) || board.isFull()) {
                    finished = true;
                    result = board.checkWin(client->symbol) ? "RESULT " + client->name + " wins!" : "RESULT Draw!";
                }
                }
                broadcast(state());
                if (!result.empty()) broadcast(result);
            } else if (command == "QUIT") break;
            else connection->sendText("ERROR Unknown command. Use JOIN, MOVE, or QUIT.");
        }
        remove(client);
    }

public:
    explicit MultiplayerServer(unsigned short port) {
        const int listener = websocket::listenOn(port);
        std::cout << "Websocket server listening on port " << port << std::endl;
        while (true) {
            const int fd = accept(listener, nullptr, nullptr);
            if (fd < 0) continue;
            std::thread(&MultiplayerServer::handle, this,
                        std::make_shared<websocket::Connection>(fd, false)).detach();
        }
    }
};

inline void runMultiplayerClient(const std::string& host, unsigned short port,
                                 const std::string& name) {
    auto connection = std::make_shared<websocket::Connection>(
        websocket::connectTo(host, port), true);
    const std::string key = "dGhlIHNhbXBsZSBub25jZQ==";
    const std::string request =
        "GET / HTTP/1.1\r\nHost: " + host + "\r\nUpgrade: websocket\r\n"
        "Connection: Upgrade\r\nSec-WebSocket-Key: " + key +
        "\r\nSec-WebSocket-Version: 13\r\n\r\n";
    if (!websocket::sendAll(connection->fd(), request.data(), request.size()) ||
        !connection->clientHandshake()) throw std::runtime_error("websocket handshake failed");
    connection->sendText("JOIN " + name);
    std::thread receiver([connection] {
        std::string message;
        while (connection->receiveText(message)) std::cout << "\n" << message << "\n> " << std::flush;
    });
    std::cout << "Connected. Type 'move 1' through 'move 9', or 'quit'.\n> " << std::flush;
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "quit") { connection->sendText("QUIT"); break; }
        if (input.rfind("move ", 0) == 0) connection->sendText("MOVE " + input.substr(5));
        else std::cout << "Use: move <1-9> or quit\n> " << std::flush;
    }
    if (receiver.joinable()) receiver.join();
}
