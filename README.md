# Tic Tac Toe

## Local game

```sh
g++ -std=c++17 main.cpp -pthread -o tictactoe
./tictactoe Alice Bob
```

## Multiplayer websocket game

Start one server on the machine that will host the game:

```sh
./tictactoe --server 8080
```

Then connect two or more terminals to it. The first two clients become `X` and
`O`; additional clients can watch the game.

```sh
./tictactoe --connect 127.0.0.1 8080 Alice
./tictactoe --connect 127.0.0.1 8080 Bob
```

In a connected terminal, enter `move 1` through `move 9` to play, or `quit` to
disconnect. The server speaks RFC 6455 websocket frames, so websocket clients
such as browser applications can connect to the same endpoint using
`ws://<host>:8080/`. The text commands are `JOIN <name>`, `MOVE <1-9>`, and
`QUIT`; server updates use `STATE`, `WELCOME`, `INFO`, `ERROR`, and `RESULT`
messages.
