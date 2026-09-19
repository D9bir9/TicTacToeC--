# How Multiplayer WebSockets Work in This Project

This document explains the multiplayer changes in this Tic Tac Toe program.
It is intended as a guide for learning, not just as a reference for running
the program.

## 1. What changed?

Originally, the program was a local terminal game:

1. Read a move from `std::cin`.
2. Update a `Board`.
3. Check for a win or draw.
4. Ask whether to play again.

That design only supports players who share one terminal. The multiplayer
version adds a client/server architecture:

```text
Player terminal A ─┐
                   ├── WebSocket connections ── Game server
Player terminal B ─┘
```

The server owns the authoritative game state. Clients send requests such as
`MOVE 5`, and the server decides whether that move is valid. The server then
broadcasts the updated board to every connected client.

The main new files are:

- [`websocket.hpp`](./websocket.hpp): low-level WebSocket networking.
- [`multiplayer.hpp`](./multiplayer.hpp): multiplayer game rules and commands.
- [`main.cpp`](./main.cpp): command-line mode selection.

The existing local game remains in [`game.hpp`](./game.hpp), and the board
model remains in [`board.hpp`](./board.hpp).

## 2. What is a WebSocket?

WebSocket is a network protocol designed for two-way communication. Unlike a
normal HTTP request, the connection stays open:

```text
Client ── request ──> Server
Client <── response ── Server
Client ── message ──> Server
Client <── message ── Server
```

This is useful for a game because the server can notify both players
immediately after a move. The clients do not need to repeatedly ask whether
anything changed.

WebSockets begin with an HTTP upgrade handshake. The client sends an HTTP
request containing:

```text
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: <random value>
Sec-WebSocket-Version: 13
```

The server responds with status `101 Switching Protocols`. It calculates the
`Sec-WebSocket-Accept` value by:

1. Taking the client's key.
2. Appending the WebSocket protocol's fixed GUID.
3. Computing SHA-1.
4. Encoding the result with Base64.

After this response, the connection is no longer used as ordinary HTTP. Both
sides exchange WebSocket frames.

## 3. The `websocket.hpp` layer

`websocket.hpp` deliberately uses the operating system socket API instead of a
third-party library. This keeps the example self-contained.

### Opening sockets

The server calls `listenOn(port)`:

1. Create a TCP socket.
2. Bind it to the requested port.
3. Start listening for incoming connections.
4. Accept connections in a loop.

The client calls `connectTo(host, port)` to create a TCP connection to the
server.

TCP provides a reliable byte stream, but it does not preserve message
boundaries. WebSocket frames solve that problem by adding a small header before
each message.

### Sending a text frame

`Connection::sendText()` creates a WebSocket text frame:

```text
FIN + text opcode | payload length | optional mask | payload bytes
```

Client-to-server frames must be masked by the WebSocket protocol. Server-to-
client frames are not masked. The implementation handles both cases.

The current implementation supports text messages up to 65,535 bytes. That is
more than enough for the small game messages used here.

### Receiving a text frame

`Connection::receiveText()` reads:

1. The two-byte frame header.
2. Any extended payload length.
3. The masking key, if present.
4. The payload.

Masked payload bytes are unmasked using:

```text
payload[i] = payload[i] XOR mask[i % 4]
```

The implementation also recognizes close frames and rejects unsupported
large/binary messages.

## 4. The multiplayer server

The server is implemented by `MultiplayerServer` in
[`multiplayer.hpp`](./multiplayer.hpp).

When a client connects, the server starts a detached thread for that client.
This allows multiple clients to stay connected at the same time:

```cpp
std::thread(&MultiplayerServer::handle, this, connection).detach();
```

Each client goes through this flow:

1. Complete the WebSocket handshake.
2. Send `INFO Connected...`.
3. Wait for commands.
4. Process commands until the client disconnects.
5. Remove the client from the server's client list.

The server stores:

- The current `Board`.
- Connected clients.
- Each player's name.
- Each player's symbol (`X` or `O`).
- Whose turn it is.
- Whether the game has finished.

The server uses a mutex because multiple client threads can access this state
at the same time. For example, two clients might send moves nearly
simultaneously. The mutex ensures that checking and applying a move happens as
one protected operation.

## 5. The text protocol

The game uses simple text messages inside WebSocket frames. This makes the
protocol easy to inspect while learning.

### Client-to-server commands

```text
JOIN Alice
MOVE 5
QUIT
```

`JOIN` assigns the first available player slot. The first player becomes `X`;
the second becomes `O`. Further clients are spectators.

`MOVE` contains a board position from `1` to `9`:

```text
1 | 2 | 3
---------
4 | 5 | 6
---------
7 | 8 | 9
```

The server checks that:

- The client joined as a player.
- It is that player's turn.
- The position is between `1` and `9`.
- The position is empty.
- The game has not already finished.

### Server-to-client messages

Examples include:

```text
WELCOME X
STATE X        TURN 2
INFO Alice joined the game.
ERROR It is not your turn.
RESULT Alice wins!
```

`STATE` contains the nine board cells followed by the next player number.
Spaces represent empty cells. Every connected client receives the new state
after a successful move.

## 6. The client flow

`runMultiplayerClient()` performs these steps:

1. Connect to the server's TCP port.
2. Send the WebSocket upgrade request.
3. Validate the `101 Switching Protocols` response.
4. Send `JOIN <name>`.
5. Start a receiver thread.
6. Read commands from the terminal.
7. Translate `move 5` into `MOVE 5`.

The receiver thread is important. Without it, the client would be blocked
waiting for local keyboard input and could not display server updates promptly.

The client and receiver therefore work concurrently:

```text
Main thread:     read terminal input and send commands
Receiver thread: receive and print server messages
```

## 7. Running the program

Compile the program with a C++17 compiler:

```sh
g++ -std=c++17 main.cpp -pthread -o tictactoe
```

Start the server:

```sh
./tictactoe --server 8080
```

Open two other terminals and connect players:

```sh
./tictactoe --connect 127.0.0.1 8080 Alice
./tictactoe --connect 127.0.0.1 8080 Bob
```

Then type commands such as:

```text
move 5
move 1
```

The first player to connect is `X` and starts the game. A third client can
connect and observe the state, but cannot make moves.

To connect from another machine, replace `127.0.0.1` with the server
machine's network address and make sure the selected port is allowed through
the firewall:

```sh
./tictactoe --connect 192.168.1.25 8080 Charlie
```

## 8. Why the server must own the game state

It would be unsafe for each client to decide its own moves independently.
Two clients could disagree about:

- Whose turn it is.
- Whether a square is occupied.
- Whether somebody has already won.

The server avoids this by being authoritative:

```text
Client: "I want position 5."
Server: validate position 5
Server: update the board
Server: broadcast the new state
```

This is a common multiplayer pattern. Clients send intentions, while the
server validates and applies them.

## 9. Important implementation limitations

This is a compact learning implementation, not a production server. Some
features that could be added later are:

- A proper close-frame response and graceful shutdown.
- Ping/pong heartbeat frames to detect dead connections.
- Reconnecting clients.
- Multiple simultaneous game rooms.
- A reset/rematch command.
- Structured JSON messages instead of text commands.
- Authentication and input/name validation.
- A build system such as CMake.
- More complete protocol validation, including fragmented frames.
- TLS (`wss://`) for encrypted internet connections.

The server currently hosts one game per process. Restarting the server resets
the board and disconnects all clients.

## 10. Suggested exercises

These are good next changes for learning:

1. Add a `RESET` command that starts a new game after a result.
2. Include both player names in the `STATE` message.
3. Add a `BOARD` command that lets a newly connected spectator request the
   current state.
4. Replace the text protocol with JSON.
5. Add a browser client using JavaScript's built-in `WebSocket` class.
6. Add a CMake file so the program can be built with `cmake --build`.
7. Write tests for the board rules independently from the network code.

The most important design lesson is to keep the game rules separate from the
transport. `Board` knows how Tic Tac Toe works, while `websocket.hpp` knows how
bytes travel over the network. That separation makes it easier to replace the
terminal client with a browser or mobile client later.
