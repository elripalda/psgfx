// Client for PS5 Upload 6.x.
// PS5 Upload 6 replaced the FTX2 protocol (ports 9113/9114) with AVA1: one encrypted, paired
// connection on port 9120. Instead of speaking AVA1 itself, PSGFX goes through the PS5 Upload
// app's local engine (HTTP on 127.0.0.1:19113), which already holds the paired session:
//   POST /api/mgmt/call   one management method, legacy JSON body in and out (the engine converts)
//   POST /api/ps5/fs/copy, /api/ps5/fs/delete   (jobs on the console; the engine waits for them)
// So the PS5 Upload app must be open and paired with the console.
#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "lib/json.hpp"

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

// The old FTX2 frame numbers stay as operation ids so callers don't change.
enum Frame : uint16_t {
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

inline uint32_t get32(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }

inline std::string b64_encode(const uint8_t *p, size_t n) {
    static const char T[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string o; o.reserve((n + 2) / 3 * 4);
    for (size_t i = 0; i < n; i += 3) {
        uint32_t v = (uint32_t)p[i] << 16 | (i + 1 < n ? (uint32_t)p[i + 1] << 8 : 0) | (i + 2 < n ? p[i + 2] : 0);
        o += T[v >> 18 & 63]; o += T[v >> 12 & 63];
        o += i + 1 < n ? T[v >> 6 & 63] : '='; o += i + 2 < n ? T[v & 63] : '=';
    }
    return o;
}
inline std::vector<uint8_t> b64_decode(const std::string &s) {
    std::vector<uint8_t> o; uint32_t v = 0; int bits = 0;
    for (char ch : s) {
        int d = ch >= 'A' && ch <= 'Z' ? ch - 'A' : ch >= 'a' && ch <= 'z' ? ch - 'a' + 26 : ch >= '0' && ch <= '9' ? ch - '0' + 52
              : ch == '+' ? 62 : ch == '/' ? 63 : -1;
        if (d < 0) continue;   // '=' padding and whitespace
        v = v << 6 | (uint32_t)d; bits += 6;
        if (bits >= 8) { bits -= 8; o.push_back((uint8_t)(v >> bits)); }
    }
    return o;
}

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
            throw Error("Can't connect to " + host + ":" + std::to_string(port));
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
    void send_all(const char *p, size_t n) {
        while (n) {
            int w = ::send(s_, p, (int)std::min<size_t>(n, 1 << 20), 0);
            if (w <= 0) throw Error("The connection to PS5 Upload dropped while sending");
            p += w; n -= (size_t)w;
        }
    }
    std::string recv_to_eof() {
        std::string out; char buf[1 << 16];
        for (;;) {
            int r = ::recv(s_, buf, sizeof buf, 0);
            if (r == 0) return out;
            if (r < 0) throw Error("PS5 Upload didn't answer in time");
            out.append(buf, (size_t)r);
        }
    }

  private:
    sock_t s_ = SOCK_BAD;
};

struct Client {
    std::string host;                      // the PS5's IP
    std::string engine_host = "127.0.0.1"; // PS5 Upload's engine (the desktop app runs it)
    int engine_port = 19113;

    // One operation. `type` is the old frame number; `json` is the same body as before.
    // Returns the reply body (raw file bytes for FS_READ, JSON text otherwise; empty for plain acks).
    std::vector<uint8_t> call(uint16_t type, const std::string &body_json, uint16_t /*expect*/, int timeout_ms = 15000) {
        using nlohmann::json;
        if (type == FS_COPY || type == FS_DELETE) {
            json in = json::parse(body_json.empty() ? "{}" : body_json), req{{"addr", host}};
            if (type == FS_COPY) {
                req["from"] = in.value("from", ""); req["to"] = in.value("to", "");
                req["overwrite"] = in.contains("overwrite") && (in["overwrite"].is_boolean() ? in["overwrite"].get<bool>() : in["overwrite"].get<int>() != 0);
            } else {
                req["path"] = in.value("path", "");
            }
            auto [status, body] = post(type == FS_COPY ? "/api/ps5/fs/copy" : "/api/ps5/fs/delete", req.dump(), timeout_ms);
            if (status / 100 != 2) throw Error(engine_error(type == FS_COPY ? "copy" : "delete", body));
            return {};
        }
        uint16_t method; const char *label;
        switch (type) {
            case FS_LIST_DIR: method = 33; label = "FS_LIST_DIR"; break;
            case FS_READ: method = 38; label = "FS_READ"; break;
            case FS_WRITE_BYTES: method = 39; label = "FS_WRITE_BYTES"; break;
            case APP_REGISTER: method = 48; label = "APP_REGISTER"; break;
            case APP_LIST_REGISTERED: method = 51; label = "APP_LIST_REGISTERED"; break;
            case APPDB_QUERY: method = 56; label = "APP_DB_QUERY"; break;
            default: throw Error("Unsupported operation " + std::to_string(type));
        }
        // Over AVA1 the app list (paged) and the app database query take 10-75 s on firmware 13.x.
        if (method == 51 || method == 56) timeout_ms = std::max(timeout_ms, 120000);
        if (type == FS_WRITE_BYTES) {
            // fs.write doesn't create folders (FTX2's upload did): with the folder missing, the payload
            // can't create "<path>.ps5upload.tmp" and answers fs_write_no_tmp_file. Create it and retry.
            try { return rpc(method, label, body_json, timeout_ms); }
            catch (const Error &e) {
                std::string path = json::parse(body_json).value("path", "");
                size_t slash = path.find_last_of('/');
                if (std::string(e.what()).find("fs_write_no_tmp_file") == std::string::npos || slash == 0 || slash == std::string::npos) throw;
                rpc(35, "FS_MKDIR", json{{"path", path.substr(0, slash)}}.dump(), timeout_ms);
            }
        }
        return rpc(method, label, body_json, timeout_ms);
    }

    // One management method through the engine's /api/mgmt/call.
    std::vector<uint8_t> rpc(uint16_t method, const char *label, const std::string &body_json, int timeout_ms) {
        using nlohmann::json;
        json req{{"op", "call"}, {"addr", host}, {"method", method}, {"label", label},
                 {"body_b64", b64_encode((const uint8_t *)body_json.data(), body_json.size())}, {"timeout_ms", timeout_ms}};
        auto [status, body] = post("/api/mgmt/call", req.dump(), timeout_ms + 5000);
        json r = json::parse(body, nullptr, false);
        if (status / 100 != 2 || r.is_discarded()) throw Error(engine_error(label, body));
        if (!r.value("ok", false)) {
            auto e = r.value("error", json::object());
            std::string cause = e.value("cause", e.value("message", std::string("unknown error")));
            if (cause.find("pair") != std::string::npos)
                throw Error("PS5 Upload isn't paired with the PS5 at " + host + ". Open PS5 Upload, connect to the console and confirm the code shown on the PS5.");
            throw Error("The PS5 refused " + std::string(label) + ": " + cause);
        }
        return b64_decode(r["result"].value("body_b64", ""));
    }
    std::string call_str(uint16_t type, const std::string &body_json, uint16_t expect, int timeout_ms = 15000) {
        auto b = call(type, body_json, expect, timeout_ms);
        return std::string(b.begin(), b.end());
    }

    // Whole file in one write (the engine sends it in chunks and commits once). Max 16 MiB.
    void upload(const std::string &dest, const std::vector<uint8_t> &data) {
        if (data.size() > (16u << 20)) throw Error("File too large to send (" + std::to_string(data.size() >> 20) + " MiB, max 16 MiB): " + dest);
        nlohmann::json req{{"path", dest}, {"bytes", b64_encode(data.data(), data.size())}, {"mode", "overwrite"}, {"large", true}};
        call(FS_WRITE_BYTES, req.dump(), FS_WRITE_BYTES_ACK, 120000);
    }

    // Read a whole file; each call returns at most ~256 KiB, so loop until an empty piece.
    std::vector<uint8_t> read_file(const std::string &path, size_t max_total = 64u << 20) {
        std::vector<uint8_t> out;
        bool system = path.rfind("/system", 0) == 0;   // /system, /system_data, /system_ex need "unsafe" (read-only)
        while (out.size() < max_total) {
            nlohmann::json q{{"path", path}, {"offset", out.size()}, {"limit", 2097152}};
            if (system) q["unsafe"] = true;
            auto part = call(FS_READ, q.dump(), FS_READ_ACK, 20000);
            if (part.empty()) break;
            out.insert(out.end(), part.begin(), part.end());
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
    // Minimal HTTP/1.1 POST to the local engine. Returns (status, body).
    std::pair<int, std::string> post(const std::string &path, const std::string &body, int timeout_ms) {
        std::unique_ptr<Conn> c;
        try { c = std::make_unique<Conn>(engine_host, engine_port, timeout_ms); }
        catch (const Error &) {
            throw Error("Can't reach PS5 Upload on this PC (" + engine_host + ":" + std::to_string(engine_port) +
                        "). Open the PS5 Upload app (6.0 or newer) and connect it to your PS5 first.");
        }
        std::string req = "POST " + path + " HTTP/1.1\r\nHost: " + engine_host + ":" + std::to_string(engine_port) +
                          "\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
                          "\r\nConnection: close\r\n\r\n" + body;
        c->send_all(req.data(), req.size());
        std::string raw = c->recv_to_eof();
        size_t he = raw.find("\r\n\r\n");
        if (raw.compare(0, 5, "HTTP/") != 0 || he == std::string::npos) throw Error("Unexpected reply from PS5 Upload");
        int status = atoi(raw.c_str() + raw.find(' ') + 1);
        std::string head = raw.substr(0, he), out = raw.substr(he + 4);
        for (auto &ch : head) ch = (char)tolower((unsigned char)ch);
        if (head.find("transfer-encoding: chunked") != std::string::npos) {
            std::string d; size_t p = 0;
            for (;;) {
                size_t le = out.find("\r\n", p);
                if (le == std::string::npos) break;
                size_t n = strtoul(out.c_str() + p, nullptr, 16);
                if (!n) break;
                d.append(out, le + 2, n); p = le + 2 + n + 2;
            }
            out.swap(d);
        }
        return {status, out};
    }
    static std::string engine_error(const std::string &what, const std::string &body) {
        auto j = nlohmann::json::parse(body, nullptr, false);
        std::string msg = !j.is_discarded() && j.is_object() ? j.value("error", body) : body;
        return "PS5 Upload couldn't " + what + ": " + (msg.empty() ? "no details" : msg);
    }
};

}  // namespace ps5
