// Minimal FTP client, used only for the home-screen database.
//
// PS5 Upload's own file commands only write under /data, /user and USB drives, but the
// home screen lives in /system_data/priv/mms/app.db. The payload can start its built-in
// FTP server on request (management frame FTP_START), and that server can reach the
// whole filesystem, so the Home layout feature goes through it.
#pragma once
#include "ps5client.h"
#include "lib/json.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace ftp {

using ps5::Error;

constexpr uint16_t FTP_START = 224, FTP_START_ACK = 225;
constexpr int DEFAULT_PORT = 2122;

// Ask PS5 Upload's payload to start its FTP server; returns the port it listens on.
inline int start_server(ps5::Client &c) {
    std::string body = nlohmann::json{{"port", DEFAULT_PORT}, {"root", "/"}, {"readonly", false}}.dump();
    auto j = nlohmann::json::parse(c.call_str(FTP_START, body, FTP_START_ACK, 15000));
    std::string err = j.value("error", "");
    if (j.value("ok", false) || err == "already_running") return j.value("port", DEFAULT_PORT) ? j.value("port", DEFAULT_PORT) : DEFAULT_PORT;
    if (err == "stopping") throw Error("PS5 Upload's file server is still shutting down. Try again in a few seconds.");
    throw Error("PS5 Upload couldn't start its file server (" + err + ").");
}

class Session {
  public:
    Session(const std::string &host, int port) : host_(host) {
        ctl_ = dial(host, port);
        expect(reply(), 2, "connect");
        cmd("USER anonymous"); int code = reply();
        if (code / 100 == 3) { cmd("PASS anonymous"); code = reply(); }
        if (code / 100 != 2) throw Error("The PS5's file server refused the login (" + last_ + ")");
        cmd("TYPE I"); expect(reply(), 2, "binary mode");
    }
    ~Session() { if (ctl_ != SOCK_BAD) { try { cmd("QUIT"); } catch (...) {} sock_close(ctl_); } }
    Session(const Session &) = delete; Session &operator=(const Session &) = delete;

    std::vector<uint8_t> get(const std::string &path) {
        sock_t d = pasv();
        cmd("RETR " + path);
        int code = reply();
        if (code / 100 != 1) { sock_close(d); throw Error("Couldn't read " + path + " (" + last_ + ")"); }
        std::vector<uint8_t> out; char buf[1 << 16];
        for (;;) { int n = recv(d, buf, sizeof buf, 0); if (n <= 0) break; out.insert(out.end(), buf, buf + n); }
        sock_close(d);
        expect(reply(), 2, "read " + path);
        return out;
    }

    void put(const std::string &path, const std::vector<uint8_t> &data) {
        sock_t d = pasv();
        cmd("STOR " + path);
        int code = reply();
        if (code / 100 != 1) { sock_close(d); throw Error("Couldn't write " + path + " (" + last_ + ")"); }
        size_t off = 0;
        while (off < data.size()) {
            int n = ::send(d, (const char *)data.data() + off, (int)std::min<size_t>(data.size() - off, 1 << 20), 0);
            if (n <= 0) { sock_close(d); throw Error("Upload of " + path + " was interrupted"); }
            off += (size_t)n;
        }
        sock_close(d);
        expect(reply(), 2, "write " + path);
    }

    std::vector<std::string> list(const std::string &dir) {
        sock_t d = pasv();
        cmd("NLST " + dir);
        int code = reply();
        std::vector<std::string> names;
        if (code / 100 != 1) { sock_close(d); return names; }
        std::string all; char buf[8192];
        for (;;) { int n = recv(d, buf, sizeof buf, 0); if (n <= 0) break; all.append(buf, n); }
        sock_close(d);
        reply();
        size_t p = 0;
        while (p < all.size()) {
            size_t e = all.find('\n', p); if (e == std::string::npos) e = all.size();
            std::string line = all.substr(p, e - p);
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) line.pop_back();
            if (!line.empty()) { size_t s = line.find_last_of('/'); names.push_back(s == std::string::npos ? line : line.substr(s + 1)); }
            p = e + 1;
        }
        return names;
    }

    bool exists(const std::string &path) {
        size_t s = path.find_last_of('/');
        std::string dir = s == 0 ? "/" : path.substr(0, s), name = path.substr(s + 1);
        for (auto &n : list(dir)) if (n == name) return true;
        return false;
    }

    void mkdirs(const std::string &path) {
        std::string cur;
        size_t p = 1;
        while (p <= path.size()) {
            size_t e = path.find('/', p); if (e == std::string::npos) e = path.size();
            cur += "/" + path.substr(p, e - p);
            cmd("MKD " + cur); reply();   // "already exists" is fine
            p = e + 1;
        }
    }

  private:
    std::string host_, last_, pending_;
    sock_t ctl_ = SOCK_BAD;

    static sock_t dial(const std::string &host, int port) {
        addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0 || !res) throw Error("Can't resolve " + host);
        sock_t s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
#ifdef _WIN32
        DWORD t = 30000;
#else
        timeval t{30, 0};
#endif
        setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char *)&t, sizeof t);
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char *)&t, sizeof t);
        if (s == SOCK_BAD || connect(s, res->ai_addr, (int)res->ai_addrlen) != 0) {
            freeaddrinfo(res); if (s != SOCK_BAD) sock_close(s);
            throw Error("Can't reach the PS5's file server at " + host + ":" + std::to_string(port));
        }
        freeaddrinfo(res);
        return s;
    }
    void cmd(const std::string &c) {
        std::string line = c + "\r\n";
        if (::send(ctl_, line.data(), (int)line.size(), 0) <= 0) throw Error("Lost the connection to the PS5's file server");
    }
    std::string read_line() {
        for (;;) {
            size_t e = pending_.find('\n');
            if (e != std::string::npos) { std::string l = pending_.substr(0, e); pending_.erase(0, e + 1); if (!l.empty() && l.back() == '\r') l.pop_back(); return l; }
            char buf[1024]; int n = recv(ctl_, buf, sizeof buf, 0);
            if (n <= 0) throw Error("The PS5's file server stopped responding");
            pending_.append(buf, n);
        }
    }
    // Reads one (possibly multi-line) reply; returns its code.
    int reply() {
        std::string l = read_line();
        if (l.size() >= 4 && l[3] == '-') {
            std::string code = l.substr(0, 3);
            for (;;) { std::string m = read_line(); if (m.size() >= 4 && m.compare(0, 3, code) == 0 && m[3] == ' ') { l = m; break; } }
        }
        last_ = l;
        return l.size() >= 3 ? atoi(l.substr(0, 3).c_str()) : 0;
    }
    void expect(int code, int cls, const std::string &what) {
        if (code / 100 != cls) throw Error("The PS5's file server refused to " + what + " (" + last_ + ")");
    }
    // Passive data connection. Connects to the same host as the control channel.
    sock_t pasv() {
        cmd("PASV");
        expect(reply(), 2, "open a data connection");
        int a, b, c, d, p1, p2;
        size_t o = last_.find('(');
        if (o == std::string::npos || sscanf(last_.c_str() + o, "(%d,%d,%d,%d,%d,%d)", &a, &b, &c, &d, &p1, &p2) != 6)
            throw Error("Unexpected reply from the PS5's file server: " + last_);
        return dial(host_, p1 * 256 + p2);
    }
};

}  // namespace ftp
