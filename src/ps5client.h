// Minimal client for PS5 Upload's resident payload (FTX2 protocol).
//   port 9114: management (JSON request -> ACK)
//   port 9113: transfers   (BeginTx -> StreamShard -> CommitTx)
// Frame = 28-byte little-endian header + body.
#pragma once
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>
#include <random>
#include "lib/blake3.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET sock_t;
#define SOCK_BAD INVALID_SOCKET
#define sock_close closesocket
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
typedef int sock_t;
#define SOCK_BAD (-1)
#define sock_close close
#endif

namespace ps5 {

enum Frame : uint16_t {
    ERROR_ = 3,
    BEGIN_TX = 10, BEGIN_TX_ACK = 11, COMMIT_TX = 14, COMMIT_TX_ACK = 15,
    ABORT_TX = 16, ABORT_TX_ACK = 17,
    STREAM_SHARD = 30, SHARD_ACK = 31,
    FS_LIST_DIR = 36, FS_LIST_DIR_ACK = 37,
    FS_DELETE = 40, FS_DELETE_ACK = 41,
    FS_READ = 48, FS_READ_ACK = 49,
    FS_COPY = 50, FS_COPY_ACK = 51,
    APP_REGISTER = 56, APP_REGISTER_ACK = 57,
    APP_LIST_REGISTERED = 62, APP_LIST_REGISTERED_ACK = 63,
    APPDB_QUERY = 120, APPDB_QUERY_ACK = 121,
    FS_WRITE_BYTES = 130, FS_WRITE_BYTES_ACK = 131,
};

struct Error : std::runtime_error { using std::runtime_error::runtime_error; };

inline void put16(uint8_t *p, uint16_t v) { p[0] = v & 255; p[1] = v >> 8; }
inline void put32(uint8_t *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = (uint8_t)(v >> (8 * i)); }
inline void put64(uint8_t *p, uint64_t v) { for (int i = 0; i < 8; i++) p[i] = (uint8_t)(v >> (8 * i)); }
inline uint64_t get64(const uint8_t *p) { uint64_t v = 0; for (int i = 7; i >= 0; i--) v = (v << 8) | p[i]; return v; }
inline uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

class Conn {
  public:
    Conn(const std::string &host, int port, int timeout_ms) {
        addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0 || !res)
            throw Error("Can't resolve " + host);
        s_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s_ == SOCK_BAD) { freeaddrinfo(res); throw Error("socket() failed"); }
        set_timeout(timeout_ms);
        int one = 1; setsockopt(s_, IPPROTO_TCP, TCP_NODELAY, (const char *)&one, sizeof one);
        if (connect(s_, res->ai_addr, (int)res->ai_addrlen) != 0) {
            freeaddrinfo(res); sock_close(s_); s_ = SOCK_BAD;
            throw Error("Can't reach the PS5 at " + host + ":" + std::to_string(port) +
                        " - is PS5 Upload's payload running?");
        }
        freeaddrinfo(res);
    }
    ~Conn() { if (s_ != SOCK_BAD) sock_close(s_); }
    Conn(const Conn &) = delete; Conn &operator=(const Conn &) = delete;

    void set_timeout(int ms) {
#ifdef _WIN32
        DWORD t = (DWORD)ms;
#else
        timeval t{ms / 1000, (ms % 1000) * 1000};
#endif
        setsockopt(s_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof t);
        setsockopt(s_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&t, sizeof t);
    }

    void send_frame(uint16_t type, const uint8_t *a, size_t alen, const uint8_t *b = nullptr, size_t blen = 0) {
        uint8_t h[28] = {0};
        put32(h, 0x32585446u);          // "FTX2"
        put16(h + 4, 1);                 // version
        put16(h + 6, type);
        put32(h + 8, 0);                 // flags
        put64(h + 12, (uint64_t)(alen + blen));
        put64(h + 20, 0);                // trace id 0 = untracked fast path
        send_all(h, 28);
        if (alen) send_all(a, alen);
        if (blen) send_all(b, blen);
    }

    // Returns (frame_type, body). Throws on transport errors.
    std::pair<uint16_t, std::vector<uint8_t>> recv_frame() {
        uint8_t h[28];
        recv_all(h, 28);
        if (get32(h) != 0x32585446u) throw Error("Unexpected reply from the PS5 (not PS5 Upload's payload?)");
        uint16_t type = (uint16_t)(h[6] | h[7] << 8);
        uint64_t len = get64(h + 12);
        if (len > (256ull << 20)) throw Error("Reply too large");
        std::vector<uint8_t> body((size_t)len);
        if (len) recv_all(body.data(), (size_t)len);
        return {type, std::move(body)};
    }

  private:
    sock_t s_ = SOCK_BAD;
    void send_all(const uint8_t *p, size_t n) {
        while (n) {
            int w = ::send(s_, (const char *)p, (int)std::min<size_t>(n, 1 << 20), 0);
            if (w <= 0) throw Error("Connection to the PS5 dropped while sending");
            p += w; n -= (size_t)w;
        }
    }
    void recv_all(uint8_t *p, size_t n) {
        while (n) {
            int r = ::recv(s_, (char *)p, (int)std::min<size_t>(n, 1 << 20), 0);
            if (r <= 0) throw Error("The PS5 didn't answer in time");
            p += r; n -= (size_t)r;
        }
    }
};

struct Client {
    std::string host;
    int mgmt_port = 9114, xfer_port = 9113;

    // One management round-trip. `expect` is the ACK frame type.
    std::vector<uint8_t> call(uint16_t type, const std::string &json, uint16_t expect, int timeout_ms = 15000) {
        Conn c(host, mgmt_port, timeout_ms);
        c.send_frame(type, (const uint8_t *)json.data(), json.size());
        auto [ft, body] = c.recv_frame();
        if (ft == ERROR_) throw Error(std::string(body.begin(), body.end()));
        if (ft != expect) throw Error("Unexpected reply type " + std::to_string(ft));
        return body;
    }
    std::string call_str(uint16_t type, const std::string &json, uint16_t expect, int timeout_ms = 15000) {
        auto b = call(type, json, expect, timeout_ms);
        return std::string(b.begin(), b.end());
    }

    // Whole file (<= 64 MiB) in one transaction with a single BLAKE3-verified shard.
    void upload(const std::string &dest, const std::vector<uint8_t> &data) {
        uint8_t tx[16];
        std::random_device rd; for (auto &b : tx) b = (uint8_t)rd();
        Conn c(host, xfer_port, 60000);

        std::string manifest = "{\"dest_root\":\"" + json_escape(dest) + "\",\"file_count\":1,\"total_bytes\":" +
                               std::to_string(data.size()) + ",\"total_shards\":1}";
        std::vector<uint8_t> begin(24);
        memcpy(begin.data(), tx, 16); put32(&begin[16], 1); put32(&begin[20], 0);
        begin.insert(begin.end(), manifest.begin(), manifest.end());
        c.send_frame(BEGIN_TX, begin.data(), begin.size());
        expect_ack(c, BEGIN_TX_ACK, "start upload");

        uint8_t sh[64] = {0};
        memcpy(sh, tx, 16); put64(sh + 16, 1);
        blake3_hasher hs; blake3_hasher_init(&hs);
        if (!data.empty()) blake3_hasher_update(&hs, data.data(), data.size());
        blake3_hasher_finalize(&hs, sh + 24, 32);
        put32(sh + 56, 1); put32(sh + 60, 0);
        c.send_frame(STREAM_SHARD, sh, 64, data.data(), data.size());
        expect_ack(c, SHARD_ACK, "send data");

        uint8_t commit[24] = {0};
        memcpy(commit, tx, 16);
        c.send_frame(COMMIT_TX, commit, 24);
        expect_ack(c, COMMIT_TX_ACK, "finish upload");
    }

    // Read a whole file in <= 2 MiB pieces.
    std::vector<uint8_t> read_file(const std::string &path, size_t max_total = 64u << 20) {
        std::vector<uint8_t> out;
        for (;;) {
            auto part = call(FS_READ, "{\"path\":\"" + json_escape(path) + "\",\"offset\":" + std::to_string(out.size()) +
                                          ",\"limit\":2097152}", FS_READ_ACK, 20000);
            if (part.empty()) break;
            out.insert(out.end(), part.begin(), part.end());
            if (part.size() < 2097152 || out.size() >= max_total) break;
        }
        return out;
    }

    static std::string json_escape(const std::string &s) {
        std::string o;
        for (char ch : s) {
            if (ch == '"' || ch == '\\') { o += '\\'; o += ch; }
            else if ((unsigned char)ch < 0x20) { char b[8]; snprintf(b, sizeof b, "\\u%04x", ch); o += b; }
            else o += ch;
        }
        return o;
    }

  private:
    static void expect_ack(Conn &c, uint16_t want, const char *what) {
        auto [ft, body] = c.recv_frame();
        if (ft == ERROR_) throw Error(std::string("PS5 refused to ") + what + ": " + std::string(body.begin(), body.end()));
        if (ft != want) throw Error(std::string("Unexpected reply during ") + what);
    }
};

}  // namespace ps5
