// PSGFX - change the home-screen icon and background of PS5 homebrew and installed games,
// through PS5 Upload's payload.
// Runs a tiny local web server and opens the UI in the default browser.
// Talks to PS5 Upload's resident payload (ports 9114 / 9113); nothing is installed on the PS5.

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "imaging.h"
#include "ps5client.h"
#include "lib/json.hpp"

#include <atomic>
#include <cstdio>
#include <fstream>
#include <map>
#include <mutex>
#include <set>
#include <algorithm>
#include <sstream>
#ifdef _WIN32
#include <shellapi.h>
#include <tlhelp32.h>
#endif
#define STUDIO_VERSION "1.0"

using json = nlohmann::json;

extern const char *UI_HTML;
extern const unsigned char sora300[], sora400[], sora600[], sora700[], mark_svg[], wordmark_png[];
extern const size_t sora300_len, sora400_len, sora600_len, sora700_len, mark_svg_len, wordmark_png_len;

// ─────────────────────────────── state ────────────────────────────────
struct Staged {                 // the user's image, decoded; converted per target file at apply time
    img::Image src;
    std::vector<uint8_t> preview_png;
    std::string note;
};
static std::mutex g_mu;
static std::string g_ip;
static std::map<std::string, Staged> g_staged;           // "icon" | "background"
static std::map<std::string, std::vector<uint8_t>> g_icon_cache;
static std::string g_config_path = "psgfx.json";
static std::string g_legacy_config_path;   // pre-1.0 name, read once if present

static void load_config() {
    std::ifstream f(g_config_path);
    if (!f && !g_legacy_config_path.empty()) f.open(g_legacy_config_path);
    if (!f) return;
    try { json j; f >> j; g_ip = j.value("ip", ""); } catch (...) {}
}
static void save_config() {
    std::ofstream f(g_config_path);
    f << json{{"ip", g_ip}}.dump(2);
}

static ps5::Client client() {
    ps5::Client c; c.host = g_ip;
    if (const char *e = getenv("PS5AS_HOST")) c.host = e;                  // test hooks (mock PS5)
    if (const char *e = getenv("PS5AS_MGMT_PORT")) c.mgmt_port = atoi(e);
    if (const char *e = getenv("PS5AS_XFER_PORT")) c.xfer_port = atoi(e);
    return c;
}

static std::string lower(std::string s) { for (auto &ch : s) ch = (char)tolower((unsigned char)ch); return s; }
static bool ends_with(const std::string &s, const std::string &t) { return s.size() >= t.size() && s.compare(s.size() - t.size(), t.size(), t) == 0; }
static std::string appmeta(const std::string &id) { return "/user/appmeta/" + id; }

// ─────────────────────────────── PS5 file helpers ─────────────────────
static json list_dir(ps5::Client &c, const std::string &dir) {
    try {
        json j = json::parse(c.call_str(ps5::FS_LIST_DIR, json{{"path", dir}, {"limit", 4096}}.dump(), ps5::FS_LIST_DIR_ACK));
        return j.value("entries", json::array());
    } catch (...) { return json::array(); }
}
static bool exists(ps5::Client &c, const std::string &dir, const std::string &name) {
    for (auto &e : list_dir(c, dir)) if (e.value("name", "") == name) return true;
    return false;
}
// FS_COPY's "overwrite" must be a number: the payload reads it as an integer field.
static void copy_file(ps5::Client &c, const std::string &from, const std::string &to, bool overwrite) {
    c.call(ps5::FS_COPY, json{{"from", from}, {"to", to}, {"overwrite", overwrite ? 1 : 0}}.dump(), ps5::FS_COPY_ACK, 30000);
}

// Title name from a PS4 param.sfo.
static std::string sfo_title(const std::vector<uint8_t> &d) {
    auto u32 = [&](size_t o) { return o + 4 <= d.size() ? ps5::get32(&d[o]) : 0u; };
    if (d.size() < 20 || memcmp(d.data(), "\0PSF", 4) != 0) return "";
    uint32_t kt = u32(8), dt = u32(12), n = u32(16);
    for (uint32_t i = 0; i < n && 20 + i * 16 + 16 <= d.size(); i++) {
        size_t e = 20 + i * 16;
        uint32_t ko = d[e] | d[e + 1] << 8, len = u32(e + 4), dof = u32(e + 12);
        if (kt + ko >= d.size()) break;
        std::string key((const char *)&d[kt + ko]);
        if (key == "TITLE" && dt + dof + len <= d.size()) {
            std::string v((const char *)&d[dt + dof], len);
            return v.substr(0, v.find('\0'));
        }
    }
    return "";
}
static std::string meta_title(ps5::Client &c, const std::string &id) {
    try {
        auto b = c.read_file(appmeta(id) + "/param.json", 1u << 20);
        json j = json::parse(b.begin(), b.end());
        auto lp = j.value("localizedParameters", json::object());
        std::string lang = lp.value("defaultLanguage", "en-US");
        for (auto &k : {lang, std::string("en-US")})
            if (lp.contains(k) && lp[k].contains("titleName")) return lp[k]["titleName"].get<std::string>();
    } catch (...) {}
    try { return sfo_title(c.read_file(appmeta(id) + "/param.sfo", 1u << 20)); } catch (...) {}
    return "";
}

// ─────────────────────────────── actions ──────────────────────────────
// Homebrew registered by PS5 Upload (we know its folder) + installed games (edited in
// /user/appmeta/<id>, where the home screen reads its art). System apps and nameless
// leftovers are marked hidden so the list stays clean.
static json list_apps() {
    auto c = client();
    std::map<std::string, std::string> src, name;
    json reg = json::parse(c.call_str(ps5::APP_LIST_REGISTERED, "", ps5::APP_LIST_REGISTERED_ACK));
    for (auto &a : reg.value("apps", json::array())) {
        std::string id = a.value("title_id", "");
        src[id] = a.value("src", "");
        std::string n = a.value("title_name", "");
        if (n != id) name[id] = n;
    }
    bool have_db = false;
    try {   // Sony's app list with real names (newer PS5 Upload payloads)
        json db = json::parse(c.call_str(ps5::APPDB_QUERY, "{}", ps5::APPDB_QUERY_ACK));
        for (auto &a : db.value("apps", json::array())) {
            std::string id = a.value("title_id", ""), n = a.value("name", "");
            if (id.empty()) continue;
            have_db = true;
            if (!src.count(id)) src[id] = "";
            if (!n.empty() && !name.count(id)) name[id] = n;
        }
    } catch (...) {}
    std::set<std::string> metas;
    for (auto &e : list_dir(c, "/user/appmeta")) metas.insert(e.value("name", ""));

    json out = json::array();
    for (auto &[id, s] : src) {
        std::string kind = !s.empty() ? "homebrew"
                         : (metas.count(id) && id.rfind("NPXS", 0) != 0) ? "game" : "other";
        std::string n = name.count(id) ? name[id] : "";
        if (n.empty() && kind != "other" && !have_db) n = meta_title(c, id);
        bool hidden = kind == "other" || (kind == "game" && n.empty());
        out.push_back({{"title_id", id}, {"title_name", n.empty() ? id : n}, {"src", s},
                       {"kind", kind}, {"editable", kind != "other"}, {"hidden", hidden}});
    }
    std::stable_sort(out.begin(), out.end(), [](const json &a, const json &b) {
        auto rank = [](const json &x) { return x["kind"] == "homebrew" ? 0 : x["kind"] == "game" ? 1 : 2; };
        if (rank(a) != rank(b)) return rank(a) < rank(b);
        return lower(a["title_name"].get<std::string>()) < lower(b["title_name"].get<std::string>());
    });
    return out;
}

static std::vector<uint8_t> fetch_icon(const std::string &id) {
    {
        std::lock_guard<std::mutex> l(g_mu);
        auto it = g_icon_cache.find(id);
        if (it != g_icon_cache.end()) return it->second;
    }
    auto c = client();
    std::vector<uint8_t> bytes;
    for (auto p : {appmeta(id) + "/icon0.png", "/user/app/" + id + "/icon0.png"}) {
        try { bytes = c.read_file(p, 8u << 20); if (!bytes.empty()) break; } catch (...) {}
    }
    std::lock_guard<std::mutex> l(g_mu);
    g_icon_cache[id] = bytes;
    return bytes;
}

// Current background of an app, decoded and shrunk for the preview. Empty if none / unreadable.
static std::vector<uint8_t> fetch_background(const std::string &id, const std::string &src) {
    auto c = client();
    std::vector<std::string> tries;
    if (!src.empty()) tries.push_back(src + "/sce_sys/pic0.dds");
    for (auto n : {"pic0.dds", "pic0.png", "pic1.png", "pic1.dds"}) tries.push_back(appmeta(id) + "/" + n);
    for (auto &p : tries) {
        try {
            auto b = c.read_file(p, 64u << 20);
            img::Image im;
            if (img::from_bc7_dds(b, im) || img::decode(b, im)) return img::to_png(img::resize(im, 960, 540));
        } catch (...) {}
    }
    return {};
}

static json stage(const std::string &kind, const std::vector<uint8_t> &bytes) {
    Staged s;
    if (!img::decode(bytes, s.src)) throw ps5::Error("That file isn't a PNG or JPEG image.");
    const img::Image &src = s.src;
    if (kind == "icon") {
        s.preview_png = img::to_png(img::cover(src, 256, 256));
        if (src.w != src.h) s.note = "Will be cropped to a square.";
        else if (src.w < 512) s.note = "Small image - it may look soft.";
    } else if (kind == "background") {
        s.preview_png = img::to_png(img::cover(src, 960, 540));
        if (std::abs(src.w * 9 - src.h * 16) > src.h) s.note = "Will be cropped to 16:9.";
        else if (src.w < 1920) s.note = "Small image - it will look soft in 4K.";
    } else {
        throw ps5::Error("Unknown slot.");
    }
    std::lock_guard<std::mutex> l(g_mu);
    g_staged[kind] = std::move(s);
    return {{"ok", true}, {"note", g_staged[kind].note}};
}

// A file on the PS5 we're going to replace, and the format it has to stay in.
struct Target { std::string name; bool dds = false; int w = 0, h = 0; };

static std::vector<uint8_t> encode_for(const std::string &kind, const img::Image &src, const Target &t) {
    img::Image im = img::cover(src, t.w, t.h);
    img::opaque(im);
    return t.dds ? img::to_bc7_dds(im) : img::to_png(im);
}

// Fixed layout for homebrew sce_sys (what PS5 titles ship).
static std::vector<Target> homebrew_targets(const std::string &kind) {
    if (kind == "icon") return {{"icon0.png", false, 512, 512}};
    if (kind == "background") return {{"pic0.dds", true, 3840, 2160}, {"pic1.dds", true, 3840, 2160}};
    return {};
}

// For installed games: whatever icon / background files the home screen already has,
// kept in their own size and format. Unsupported formats are reported, not touched.
static std::vector<Target> game_targets(ps5::Client &c, const std::string &id, const std::string &kind,
                                        json &log, std::vector<std::string> &seen) {
    std::vector<Target> out;
    for (auto &e : list_dir(c, appmeta(id))) {
        std::string name = e.value("name", ""), ln = lower(name);
        if (e.value("kind", "") != "file") continue;
        seen.push_back(name);
        bool png = ends_with(ln, ".png"), dds = ends_with(ln, ".dds");
        if (!png && !dds) continue;
        bool want = kind == "icon" ? ln.rfind("icon0", 0) == 0
                                   : (ln.rfind("pic0", 0) == 0 || ln.rfind("pic1", 0) == 0);
        if (!want) continue;
        std::vector<uint8_t> h;
        try { h = c.call(ps5::FS_READ, json{{"path", appmeta(id) + "/" + name}, {"offset", 0}, {"limit", 160}}.dump(), ps5::FS_READ_ACK); }
        catch (...) { continue; }
        Target t; t.name = name; t.dds = dds;
        if (png && h.size() >= 24 && memcmp(h.data(), "\x89PNG", 4) == 0) {
            t.w = (int)(h[16] << 24 | h[17] << 16 | h[18] << 8 | h[19]);
            t.h = (int)(h[20] << 24 | h[21] << 16 | h[22] << 8 | h[23]);
        } else if (dds && h.size() >= 148 && memcmp(h.data(), "DDS ", 4) == 0 && memcmp(&h[84], "DX10", 4) == 0 &&
                   (ps5::get32(&h[128]) == 98 || ps5::get32(&h[128]) == 99)) {
            t.h = (int)ps5::get32(&h[12]); t.w = (int)ps5::get32(&h[16]);
        } else {
            log.push_back("Skipped " + name + " (a format this tool can't write yet)");
            continue;
        }
        if (t.w < 16 || t.h < 16 || t.w > 8192 || t.h > 8192 || (t.dds && (t.w % 4 || t.h % 4))) {
            log.push_back("Skipped " + name + " (unexpected size)");
            continue;
        }
        out.push_back(t);
    }
    return out;
}

// Keep the first original as <name>.bak; mark files we create with <name>.added.
static void backup_once(ps5::Client &c, const std::string &dir, const std::string &name, json &log) {
    if (exists(c, dir, name + ".bak") || exists(c, dir, name + ".added")) return;
    if (exists(c, dir, name)) { copy_file(c, dir + "/" + name, dir + "/" + name + ".bak", false); log.push_back("Backed up " + name); }
    else c.call(ps5::FS_WRITE_BYTES, json{{"path", dir + "/" + name + ".added"}, {"bytes", "MQ=="}}.dump(), ps5::FS_WRITE_BYTES_ACK);
}

// Homebrew: copy the app's sce_sys art to where the home screen reads it, then ask PS5 Upload
// to re-register. On 12.xx/13.xx Sony's installer call isn't reachable
// (register_install_api_unavailable); the copies are what matter, so that's a note, not a failure.
static void sync_home(ps5::Client &c, const std::string &id, const std::string &src,
                      const std::vector<std::string> &names, json &log) {
    const std::string dir = src + "/sce_sys";
    auto have = list_dir(c, dir);
    for (auto &name : names) {
        bool present = false;
        for (auto &e : have) if (e.value("name", "") == name) present = true;
        if (!present) continue;
        std::vector<std::string> dests = {appmeta(id) + "/" + name, "/user/app/" + id + "/sce_sys/" + name};
        if (name == "icon0.png") dests.push_back("/user/app/" + id + "/icon0.png");
        for (auto &d : dests) {
            try { copy_file(c, dir + "/" + name, d, true); }
            catch (const std::exception &e) { log.push_back("Couldn't update " + d + ": " + e.what()); }
        }
    }
    try {
        c.call_str(ps5::APP_REGISTER, json{{"src_path", src}}.dump(), ps5::APP_REGISTER_ACK, 60000);
        log.push_back("Refreshed the home screen. If the old art still shows, restart the PS5.");
    } catch (const std::exception &e) {
        std::string m = e.what();
        if (m.find("install_api_unavailable") != std::string::npos)
            log.push_back("Done. Your firmware doesn't allow an instant home-screen refresh, so restart the PS5 to see the change.");
        else
            log.push_back("Done, but the refresh step said: " + m + ". Restart the PS5 to see the change.");
    }
}

static json do_apply(const std::string &id, const std::string &src, const std::string &kind_of_app) {
    json log = json::array();
    std::map<std::string, Staged> staged;
    { std::lock_guard<std::mutex> l(g_mu); staged = g_staged; }
    if (staged.empty()) throw ps5::Error("Add at least one image first.");
    auto c = client();

    if (kind_of_app == "game") {
        int written = 0;
        std::vector<std::string> seen;
        for (auto &[kind, s] : staged) {
            auto targets = game_targets(c, id, kind, log, seen);
            if (targets.empty()) log.push_back(std::string("This game has no ") + (kind == "icon" ? "icon" : "background") + " file this tool can replace.");
            for (auto &t : targets) {
                backup_once(c, appmeta(id), t.name, log);
                auto bytes = encode_for(kind, s.src, t);
                c.upload(appmeta(id) + "/" + t.name, bytes);
                log.push_back("Replaced " + t.name + " (" + std::to_string(t.w) + "x" + std::to_string(t.h) + ")");
                written++;
            }
        }
        if (!written) {
            std::string list; std::sort(seen.begin(), seen.end()); seen.erase(std::unique(seen.begin(), seen.end()), seen.end());
            for (auto &n : seen) list += (list.empty() ? "" : ", ") + n;
            throw ps5::Error("Nothing was changed. Files in this game's art folder: " + (list.empty() ? std::string("(none)") : list));
        }
        log.push_back("Done. Restart the PS5 to see the new art.");
    } else {
        if (src.empty()) throw ps5::Error("This app's folder is unknown.");
        const std::string dir = src + "/sce_sys";
        std::vector<std::string> changed;
        for (auto &[kind, s] : staged) {
            for (auto &t : homebrew_targets(kind)) {
                changed.push_back(t.name);
                backup_once(c, dir, t.name, log);
                auto bytes = encode_for(kind, s.src, t);
                c.upload(dir + "/" + t.name, bytes);
                log.push_back("Uploaded " + t.name + " (" + std::to_string(bytes.size() / 1024) + " KB)");
            }
        }
        sync_home(c, id, src, changed, log);
    }
    { std::lock_guard<std::mutex> l(g_mu); g_icon_cache.erase(id); g_staged.clear(); }
    return {{"ok", true}, {"log", log}};
}

static json do_restore(const std::string &id, const std::string &src, const std::string &kind_of_app) {
    json log = json::array();
    auto c = client();
    const std::string dir = kind_of_app == "game" ? appmeta(id) : src + "/sce_sys";
    if (kind_of_app != "game" && src.empty()) throw ps5::Error("This app's folder is unknown.");
    std::vector<std::string> names;
    for (auto &e : list_dir(c, dir)) {
        std::string n = e.value("name", "");
        if (ends_with(n, ".bak")) names.push_back(n.substr(0, n.size() - 4));
        else if (ends_with(n, ".added")) names.push_back(n.substr(0, n.size() - 6));
    }
    if (names.empty()) throw ps5::Error("Nothing to restore - this still has its original art.");
    for (auto &name : names) {
        const std::string path = dir + "/" + name;
        if (exists(c, dir, name + ".bak")) {
            copy_file(c, path + ".bak", path, true);
            c.call(ps5::FS_DELETE, json{{"path", path + ".bak"}}.dump(), ps5::FS_DELETE_ACK);
            log.push_back("Restored " + name);
        } else {
            try { c.call(ps5::FS_DELETE, json{{"path", path}}.dump(), ps5::FS_DELETE_ACK); } catch (...) {}
            c.call(ps5::FS_DELETE, json{{"path", path + ".added"}}.dump(), ps5::FS_DELETE_ACK);
            if (kind_of_app != "game")   // registering copied it here too, and never deletes
                for (const std::string &copy : {appmeta(id) + "/" + name, "/user/app/" + id + "/sce_sys/" + name})
                    try { c.call(ps5::FS_DELETE, json{{"path", copy}}.dump(), ps5::FS_DELETE_ACK); } catch (...) {}
            log.push_back("Removed " + name + " (it wasn't there originally)");
        }
    }
    if (kind_of_app == "game") log.push_back("Done. Restart the PS5 to see the original art.");
    else sync_home(c, id, src, names, log);
    { std::lock_guard<std::mutex> l(g_mu); g_icon_cache.erase(id); }
    return {{"ok", true}, {"log", log}};
}

// ─────────────────────────────── tiny HTTP server ─────────────────────
struct Req { std::string method, path, query; std::map<std::string, std::string> q; std::vector<uint8_t> body; };

static std::string url_decode(const std::string &s) {
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) { o += (char)strtol(s.substr(i + 1, 2).c_str(), nullptr, 16); i += 2; }
        else if (s[i] == '+') o += ' ';
        else o += s[i];
    }
    return o;
}

static bool read_request(sock_t s, Req &r) {
    std::string head;
    char ch;
    while (head.find("\r\n\r\n") == std::string::npos) {
        int n = recv(s, &ch, 1, 0);
        if (n <= 0 || head.size() > 16384) return false;
        head += ch;
    }
    std::istringstream is(head);
    std::string target;
    is >> r.method >> target;
    size_t qp = target.find('?');
    r.path = url_decode(target.substr(0, qp));
    if (qp != std::string::npos) {
        r.query = target.substr(qp + 1);
        std::istringstream qs(r.query); std::string kv;
        while (std::getline(qs, kv, '&')) {
            size_t e = kv.find('=');
            if (e != std::string::npos) r.q[url_decode(kv.substr(0, e))] = url_decode(kv.substr(e + 1));
        }
    }
    size_t len = 0;
    std::string line;
    std::istringstream hs(head);
    while (std::getline(hs, line)) {
        std::string lower = line;
        for (auto &c : lower) c = (char)tolower((unsigned char)c);
        if (lower.rfind("content-length:", 0) == 0) len = (size_t)strtoull(line.c_str() + 15, nullptr, 10);
    }
    if (len > (200u << 20)) return false;
    r.body.resize(len);
    size_t got = 0;
    while (got < len) {
        int n = recv(s, (char *)r.body.data() + got, (int)std::min<size_t>(len - got, 1 << 20), 0);
        if (n <= 0) return false;
        got += (size_t)n;
    }
    return true;
}

static void respond(sock_t s, int code, const std::string &type, const void *data, size_t len) {
    const char *msg = code == 200 ? "OK" : code == 404 ? "Not Found" : "Error";
    std::string h = "HTTP/1.1 " + std::to_string(code) + " " + msg + "\r\nContent-Type: " + type +
                    "\r\nContent-Length: " + std::to_string(len) + "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n";
    send(s, h.data(), (int)h.size(), 0);
    size_t off = 0;
    while (off < len) {
        int n = send(s, (const char *)data + off, (int)std::min<size_t>(len - off, 1 << 20), 0);
        if (n <= 0) break;
        off += (size_t)n;
    }
}
static void respond_json(sock_t s, const json &j, int code = 200) { std::string b = j.dump(); respond(s, code, "application/json", b.data(), b.size()); }

static void handle(sock_t s, Req &r) {
    try {
        if (r.path == "/") { respond(s, 200, "text/html; charset=utf-8", UI_HTML, strlen(UI_HTML)); return; }
        if (r.path == "/wordmark.png") { respond(s, 200, "image/png", wordmark_png, wordmark_png_len); return; }
        if (r.path == "/mark.svg") { respond(s, 200, "image/svg+xml", mark_svg, mark_svg_len); return; }
        if (r.path.rfind("/f/sora", 0) == 0) {
            const unsigned char *d = nullptr; size_t n = 0;
            if (r.path == "/f/sora300.woff2") { d = sora300; n = sora300_len; }
            else if (r.path == "/f/sora400.woff2") { d = sora400; n = sora400_len; }
            else if (r.path == "/f/sora600.woff2") { d = sora600; n = sora600_len; }
            else if (r.path == "/f/sora700.woff2") { d = sora700; n = sora700_len; }
            if (d) { respond(s, 200, "font/woff2", d, n); return; }
        }
        if (r.path == "/api/config") { std::lock_guard<std::mutex> l(g_mu); respond_json(s, {{"ip", g_ip}}); return; }
        if (r.path == "/api/connect" && r.method == "POST") {
            json j = json::parse(std::string(r.body.begin(), r.body.end()));
            { std::lock_guard<std::mutex> l(g_mu); g_ip = j.value("ip", ""); g_icon_cache.clear(); }
            if (g_ip.empty()) throw ps5::Error("Enter your PS5's IP address.");
            json apps = list_apps();
            save_config();
            respond_json(s, {{"ok", true}, {"apps", apps}});
            return;
        }
        if (r.path.rfind("/api/icon/", 0) == 0) {
            auto b = fetch_icon(r.path.substr(10));
            if (b.empty()) { respond(s, 404, "text/plain", "none", 4); return; }
            respond(s, 200, "image/png", b.data(), b.size()); return;
        }
        if (r.path.rfind("/api/current/", 0) == 0) {
            auto b = fetch_background(r.path.substr(13), r.q["src"]);
            if (b.empty()) { respond(s, 404, "text/plain", "none", 4); return; }
            respond(s, 200, "image/png", b.data(), b.size()); return;
        }
        if (r.path.rfind("/api/stage/", 0) == 0 && r.method == "POST") { respond_json(s, stage(r.path.substr(11), r.body)); return; }
        if (r.path.rfind("/api/unstage/", 0) == 0) { std::lock_guard<std::mutex> l(g_mu); g_staged.erase(r.path.substr(13)); respond_json(s, {{"ok", true}}); return; }
        if (r.path.rfind("/api/staged/", 0) == 0) {
            std::vector<uint8_t> b;
            { std::lock_guard<std::mutex> l(g_mu); auto it = g_staged.find(r.path.substr(12)); if (it != g_staged.end()) b = it->second.preview_png; }
            if (b.empty()) { respond(s, 404, "text/plain", "none", 4); return; }
            respond(s, 200, "image/png", b.data(), b.size()); return;
        }
        if ((r.path == "/api/apply" || r.path == "/api/restore") && r.method == "POST") {
            json j = json::parse(std::string(r.body.begin(), r.body.end()));
            auto id = j.value("title_id", ""), src = j.value("src", ""), kind = j.value("kind", "homebrew");
            respond_json(s, r.path == "/api/apply" ? do_apply(id, src, kind) : do_restore(id, src, kind)); return;
        }
        respond(s, 404, "text/plain", "not found", 9);
    } catch (std::exception &e) {
        respond_json(s, {{"ok", false}, {"error", e.what()}}, 200);
    }
}

int main(int argc, char **argv) {
#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    char exe[MAX_PATH]; GetModuleFileNameA(nullptr, exe, MAX_PATH);
    std::string ep = exe; std::string exedir = ep.substr(0, ep.find_last_of("\\/") + 1);
    g_config_path = exedir + "psgfx.json"; g_legacy_config_path = exedir + "ps5-art-studio.json";
    // Close any other running copy (e.g. an older version still open in its console window),
    // so the browser can only ever talk to this one.
    {
        std::string me = ep.substr(ep.find_last_of("\\/") + 1);
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 pe{}; pe.dwSize = sizeof pe;
        bool closed = false;
        if (snap != INVALID_HANDLE_VALUE && Process32First(snap, &pe)) do {
            if (pe.th32ProcessID != GetCurrentProcessId() &&
                (_stricmp(pe.szExeFile, me.c_str()) == 0 || _strnicmp(pe.szExeFile, "PS5ArtStudio", 12) == 0 || _strnicmp(pe.szExeFile, "PSGFX", 5) == 0)) {
                HANDLE h = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pe.th32ProcessID);
                if (h) { TerminateProcess(h, 0); WaitForSingleObject(h, 3000); CloseHandle(h); closed = true; }
            }
        } while (Process32Next(snap, &pe));
        if (snap != INVALID_HANDLE_VALUE) CloseHandle(snap);
        if (closed) printf("  Closed an older PSGFX window.\n");
    }
#endif
    load_config();
    (void)argc; (void)argv;

    sock_t ls = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
#ifdef _WIN32
    setsockopt(ls, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (const char *)&one, sizeof one);  // never share a port with another copy
#else
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof one);
#endif
    sockaddr_in a{}; a.sin_family = AF_INET; a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    int port = 0;
    for (int p = 47655; p < 47700; p++) {
        a.sin_port = htons((uint16_t)p);
        if (bind(ls, (sockaddr *)&a, sizeof a) == 0) { port = p; break; }
    }
    if (!port || listen(ls, 16) != 0) { fprintf(stderr, "Couldn't open a local port.\n"); return 1; }
    std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
    printf("\n  PSGFX " STUDIO_VERSION " is running.\n  Open %s if your browser didn't open.\n  Close this window to quit.\n\n", url.c_str());
    fflush(stdout);
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#endif
    for (;;) {
        sock_t s = accept(ls, nullptr, nullptr);
        if (s == SOCK_BAD) continue;
        std::thread([s] {
            Req r;
            if (read_request(s, r)) handle(s, r);
            sock_close(s);
        }).detach();
    }
}
