#pragma once

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace websocket {

inline std::string base64(const unsigned char* data, std::size_t length) {
    static const char alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    for (std::size_t i = 0; i < length; i += 3) {
        const std::uint32_t value = (static_cast<std::uint32_t>(data[i]) << 16) |
            (i + 1 < length ? static_cast<std::uint32_t>(data[i + 1]) << 8 : 0) |
            (i + 2 < length ? data[i + 2] : 0);
        result += alphabet[(value >> 18) & 63];
        result += alphabet[(value >> 12) & 63];
        result += i + 1 < length ? alphabet[(value >> 6) & 63] : '=';
        result += i + 2 < length ? alphabet[value & 63] : '=';
    }
    return result;
}

// SHA-1 is used only for the websocket opening handshake.
inline std::string sha1Base64(const std::string& input) {
    std::uint32_t h[5] = {0x67452301, 0xefcdab89, 0x98badcfe,
                          0x10325476, 0xc3d2e1f0};
    std::vector<unsigned char> message(input.begin(), input.end());
    const std::uint64_t bits = static_cast<std::uint64_t>(message.size()) * 8;
    message.push_back(0x80);
    while (message.size() % 64 != 56) message.push_back(0);
    for (int i = 7; i >= 0; --i) message.push_back((bits >> (i * 8)) & 0xff);

    for (std::size_t offset = 0; offset < message.size(); offset += 64) {
        std::uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (message[offset + i * 4] << 24) |
                   (message[offset + i * 4 + 1] << 16) |
                   (message[offset + i * 4 + 2] << 8) |
                   message[offset + i * 4 + 3];
        }
        for (int i = 16; i < 80; ++i) {
            const std::uint32_t x = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
            w[i] = (x << 1) | (x >> 31);
        }
        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
        for (int i = 0; i < 80; ++i) {
            std::uint32_t f, k;
            if (i < 20) { f = (b & c) | (~b & d); k = 0x5a827999; }
            else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdc; }
            else { f = b ^ c ^ d; k = 0xca62c1d6; }
            const std::uint32_t rotate = (a << 5) | (a >> 27);
            const std::uint32_t next = rotate + f + e + k + w[i];
            e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = next;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
    }
    unsigned char digest[20];
    for (int i = 0; i < 5; ++i)
        for (int j = 0; j < 4; ++j) digest[i * 4 + j] = (h[i] >> (24 - j * 8)) & 0xff;
    return base64(digest, sizeof(digest));
}

inline bool sendAll(int fd, const void* data, std::size_t length) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    while (length) {
        const ssize_t written = send(fd, bytes, length, 0);
        if (written <= 0) return false;
        bytes += written;
        length -= static_cast<std::size_t>(written);
    }
    return true;
}

class Connection {
    int fd_;
    bool client_;
    std::mutex writeMutex_;

    bool receiveBytes(void* destination, std::size_t length) {
        auto* bytes = static_cast<unsigned char*>(destination);
        while (length) {
            const ssize_t count = recv(fd_, bytes, length, 0);
            if (count <= 0) return false;
            bytes += count;
            length -= static_cast<std::size_t>(count);
        }
        return true;
    }

public:
    Connection(int fd, bool client) : fd_(fd), client_(client) {}
    ~Connection() { if (fd_ >= 0) close(fd_); }
    Connection(const Connection&) = delete;

    bool handshake() {
        std::string request;
        char c;
        while (request.size() < 8192 && request.find("\r\n\r\n") == std::string::npos) {
            if (recv(fd_, &c, 1, 0) <= 0) return false;
            request += c;
        }
        const std::string keyHeader = "Sec-WebSocket-Key:";
        const auto keyStart = request.find(keyHeader);
        if (keyStart == std::string::npos) return false;
        const auto valueStart = request.find_first_not_of(" \t", keyStart + keyHeader.size());
        const auto valueEnd = request.find("\r\n", valueStart);
        if (valueStart == std::string::npos || valueEnd == std::string::npos) return false;
        const std::string key = request.substr(valueStart, valueEnd - valueStart);
        const std::string response =
            "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n"
            "Connection: Upgrade\r\nSec-WebSocket-Accept: " +
            sha1Base64(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11") + "\r\n\r\n";
        return sendAll(fd_, response.data(), response.size());
    }

    bool clientHandshake() {
        std::string response;
        char c;
        while (response.size() < 8192 && response.find("\r\n\r\n") == std::string::npos) {
            if (recv(fd_, &c, 1, 0) <= 0) return false;
            response += c;
        }
        return response.rfind("HTTP/1.1 101", 0) == 0 &&
               response.find("Sec-WebSocket-Accept:") != std::string::npos;
    }

    bool sendText(const std::string& text) {
        if (text.size() > 65535) return false;
        std::vector<unsigned char> frame;
        frame.push_back(0x81);
        if (text.size() < 126) frame.push_back(static_cast<unsigned char>(text.size()));
        else {
            frame.push_back(126);
            frame.push_back(static_cast<unsigned char>(text.size() >> 8));
            frame.push_back(static_cast<unsigned char>(text.size()));
        }
        if (client_) {
            frame[1] |= 0x80;
            const unsigned char mask[4] = {0x12, 0x34, 0x56, 0x78};
            frame.insert(frame.end(), mask, mask + 4);
            for (std::size_t i = 0; i < text.size(); ++i) frame.push_back(text[i] ^ mask[i % 4]);
        } else frame.insert(frame.end(), text.begin(), text.end());
        std::lock_guard<std::mutex> lock(writeMutex_);
        return sendAll(fd_, frame.data(), frame.size());
    }

    bool receiveText(std::string& text) {
        unsigned char header[2];
        if (!receiveBytes(header, 2)) return false;
        if ((header[0] & 0x0f) == 0x8) return false;
        if ((header[0] & 0x0f) == 0x9) {
            std::string ignored;
            if (!receiveText(ignored)) return false;
        }
        std::uint64_t length = header[1] & 0x7f;
        if (length == 126) {
            unsigned char extended[2];
            if (!receiveBytes(extended, 2)) return false;
            length = (extended[0] << 8) | extended[1];
        } else if (length == 127 || length > 65535) return false;
        const bool masked = (header[1] & 0x80) != 0;
        unsigned char mask[4] = {};
        if (masked && !receiveBytes(mask, 4)) return false;
        text.resize(static_cast<std::size_t>(length));
        if (length && !receiveBytes(text.data(), text.size())) return false;
        if (masked) for (std::size_t i = 0; i < text.size(); ++i) text[i] ^= mask[i % 4];
        return (header[0] & 0x0f) == 0x1;
    }

    int fd() const { return fd_; }
};

inline int listenOn(unsigned short port) {
    const int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("could not create socket");
    int enabled = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0 ||
        listen(fd, 16) < 0) {
        close(fd);
        throw std::runtime_error("could not bind/listen on port " + std::to_string(port));
    }
    return fd;
}

inline int connectTo(const std::string& host, unsigned short port) {
    addrinfo hints{}, *result = nullptr;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result) != 0)
        throw std::runtime_error("could not resolve host");
    int fd = -1;
    for (auto* item = result; item; item = item->ai_next) {
        fd = socket(item->ai_family, item->ai_socktype, item->ai_protocol);
        if (fd >= 0 && connect(fd, item->ai_addr, item->ai_addrlen) == 0) break;
        if (fd >= 0) { close(fd); fd = -1; }
    }
    freeaddrinfo(result);
    if (fd < 0) throw std::runtime_error("could not connect to server");
    return fd;
}
}
