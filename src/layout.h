// Home layout: hide, rename, move between Games/Media and reorder home-screen tiles by
// editing the console's home-screen database, /system_data/priv/mms/app.db.
//
// Schema (per console user <u>):
//   tbl_iconinfo_<u>         one row per tile: titleId, titleName, dispLocation, visible,
//                            lastAccessIndex/Time, lastPlayed*, recentActivity*
//   tbl_concepticoninfo_<u>  per-concept name/art: conceptName, icon0Info, pic0Info
//   tbl_group_<u>            groupName 'MyHome': which system tiles (PS Plus, Store,
//                            Game Library) appear on the Games row
//   tbl_contentinfo, tbl_conceptmetadata   global copies of name/location/recency
//   tbl_version              counters: access_index, played_index
// dispLocation is a bit field: 0x02 = Games home, 0x04 = Media home. Only those two
// bits are ever changed. The home screen sorts by play history, so ordering sets every
// recency field consistently.
#pragma once
#include "lib/sqlite3.h"
#include "lib/json.hpp"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace layout {

using json = nlohmann::json;
struct Error : std::runtime_error { using std::runtime_error::runtime_error; };

const char *const DB_PATH = "/system_data/priv/mms/app.db";
const char *const DB_DIR = "/system_data/priv/mms";
const char *const LIBRARY = "NPXS40071";
constexpr int64_t BIT_GAMES = 0x02, BIT_MEDIA = 0x04;

// Disc placeholders and similar internal tiles; never shown.
static const std::set<std::string> kInternal = {"NPXS40140", "NPXS40144", "NPXS40145", "NPXS40148",
                                                "NPXS40149", "NPXS40150", "NPXS40205", "NPXS40106", "NPXS40063"};

using Val = std::variant<std::nullptr_t, int64_t, std::string>;

// An app.db loaded into memory. Nothing touches a file until serialize().
class DB {
  public:
    explicit DB(const std::vector<uint8_t> &data) {
        if (sqlite3_open(":memory:", &db_) != SQLITE_OK) throw Error("Couldn't start SQLite");
        auto *buf = (unsigned char *)sqlite3_malloc64(data.size());
        if (!buf) throw Error("Out of memory");
        memcpy(buf, data.data(), data.size());
        if (sqlite3_deserialize(db_, "main", buf, (sqlite3_int64)data.size(), (sqlite3_int64)data.size(),
                                SQLITE_DESERIALIZE_FREEONCLOSE | SQLITE_DESERIALIZE_RESIZEABLE) != SQLITE_OK)
            throw Error("That isn't a readable home-screen database");
        std::string ok = scalar_str("PRAGMA integrity_check");
        if (ok != "ok") throw Error("The home-screen database failed its integrity check: " + ok);
    }
    ~DB() { if (db_) sqlite3_close(db_); }
    DB(const DB &) = delete; DB &operator=(const DB &) = delete;

    std::vector<uint8_t> serialize() {
        sqlite3_int64 n = 0;
        unsigned char *p = sqlite3_serialize(db_, "main", &n, 0);
        if (!p) throw Error("Couldn't save the database");
        std::vector<uint8_t> out(p, p + n);
        sqlite3_free(p);
        return out;
    }

    void exec(const std::string &sql, const std::vector<Val> &args = {}) {
        sqlite3_stmt *st = prepare(sql, args);
        int rc = sqlite3_step(st);
        sqlite3_finalize(st);
        if (rc != SQLITE_DONE && rc != SQLITE_ROW) throw Error(std::string("Database error: ") + sqlite3_errmsg(db_));
    }

    // Runs a query, calling fn(stmt) for every row.
    template <class F> void each(const std::string &sql, const std::vector<Val> &args, F fn) {
        sqlite3_stmt *st = prepare(sql, args);
        int rc;
        while ((rc = sqlite3_step(st)) == SQLITE_ROW) fn(st);
        sqlite3_finalize(st);
        if (rc != SQLITE_DONE) throw Error(std::string("Database error: ") + sqlite3_errmsg(db_));
    }

    std::string scalar_str(const std::string &sql, const std::vector<Val> &args = {}) {
        std::string out;
        each(sql, args, [&](sqlite3_stmt *st) { out = text(st, 0); });
        return out;
    }
    int64_t scalar_int(const std::string &sql, const std::vector<Val> &args = {}) {
        int64_t out = 0;
        each(sql, args, [&](sqlite3_stmt *st) { out = sqlite3_column_int64(st, 0); });
        return out;
    }

    static std::string text(sqlite3_stmt *st, int i) {
        const unsigned char *t = sqlite3_column_text(st, i);
        return t ? std::string((const char *)t) : std::string();
    }
    static bool null(sqlite3_stmt *st, int i) { return sqlite3_column_type(st, i) == SQLITE_NULL; }

  private:
    sqlite3 *db_ = nullptr;
    sqlite3_stmt *prepare(const std::string &sql, const std::vector<Val> &args) {
        sqlite3_stmt *st = nullptr;
        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr) != SQLITE_OK)
            throw Error(std::string("Database error: ") + sqlite3_errmsg(db_));
        for (size_t i = 0; i < args.size(); i++) {
            int k = (int)i + 1;
            if (auto *n = std::get_if<int64_t>(&args[i])) sqlite3_bind_int64(st, k, *n);
            else if (auto *s = std::get_if<std::string>(&args[i])) sqlite3_bind_text(st, k, s->c_str(), (int)s->size(), SQLITE_TRANSIENT);
            else sqlite3_bind_null(st, k);
        }
        return st;
    }
};

inline std::string q(const std::string &id) {
    std::string o = "\"";
    for (char c : id) { if (c == '"') o += '"'; o += c; }
    return o + "\"";
}
inline std::string strip_query(const std::string &p) { size_t i = p.find('?'); return i == std::string::npos ? p : p.substr(0, i); }

struct Tile {
    std::string id, name, concept, kind, where, icon, bg;
    int64_t disp = 0, home_idx = 0, access = 0;
    bool visible = false, home = false;
};

struct Info {
    std::string user, icon_t, concept_t, group_t;
    std::vector<std::string> users, warnings;
    std::vector<Tile> tiles;
    std::vector<std::pair<std::string, int64_t>> ghosts;   // MyHome rows for titles not on the console
    std::map<std::string, std::set<std::string>> cols;
    bool has_content = false, has_meta = false, can_order = false;
    int64_t access_index = 0, played_index = 0;

    const Tile *find(const std::string &id) const {
        for (auto &t : tiles) if (t.id == id) return &t;
        return nullptr;
    }
    json to_json() const {
        json ts = json::array();
        for (auto &t : tiles)
            ts.push_back({{"id", t.id}, {"name", t.name}, {"kind", t.kind}, {"where", t.where}, {"visible", t.visible},
                          {"home", t.home}, {"homeIdx", t.home_idx}, {"access", t.access}, {"icon", t.icon}, {"bg", t.bg}});
        return {{"user", user}, {"users", users}, {"tiles", ts}, {"warnings", warnings}, {"canOrder", can_order}};
    }
};

inline Info analyze(DB &db, const std::string &want_user = "") {
    Info info;
    std::set<std::string> tables;
    db.each("SELECT name FROM sqlite_master WHERE type='table'", {}, [&](sqlite3_stmt *st) { tables.insert(DB::text(st, 0)); });
    int best = -1;
    for (auto &t : tables) {
        if (t.rfind("tbl_iconinfo_", 0) != 0) continue;
        std::string u = t.substr(13);
        info.users.push_back(u);
        int n = (int)db.scalar_int("SELECT count(*) FROM " + q(t));
        if (u == want_user) { info.user = u; best = 1 << 30; }
        else if (want_user.empty() && n > best) { best = n; info.user = u; }
    }
    if (info.user.empty()) throw Error("This console's home-screen database has a layout PSGFX doesn't know yet (no tbl_iconinfo_ tables).");
    std::sort(info.users.begin(), info.users.end());
    info.icon_t = "tbl_iconinfo_" + info.user;
    info.concept_t = tables.count("tbl_concepticoninfo_" + info.user) ? "tbl_concepticoninfo_" + info.user : "";
    info.group_t = tables.count("tbl_group_" + info.user) ? "tbl_group_" + info.user : "";
    info.has_content = tables.count("tbl_contentinfo") > 0;
    info.has_meta = tables.count("tbl_conceptmetadata") > 0;
    if (info.users.size() > 1)
        info.warnings.push_back("This console has " + std::to_string(info.users.size()) + " user profiles. You're editing the one with the most tiles; switch with the profile menu.");
    if (tables.count("tbl_version")) {
        info.access_index = db.scalar_int("SELECT status FROM tbl_version WHERE category='access_index'");
        info.played_index = db.scalar_int("SELECT status FROM tbl_version WHERE category='played_index'");
    }
    for (auto &t : tables)
        db.each("SELECT name FROM pragma_table_info(?)", {t}, [&](sqlite3_stmt *st) { info.cols[t].insert(DB::text(st, 0)); });

    struct Concept { std::string name, icon, bg; };
    std::map<std::string, Concept> concepts;
    if (!info.concept_t.empty())
        db.each("SELECT localConceptId, conceptName, icon0Info, pic0Info FROM " + q(info.concept_t), {}, [&](sqlite3_stmt *st) {
            concepts[DB::text(st, 0)] = {DB::text(st, 1), strip_query(DB::text(st, 2)), strip_query(DB::text(st, 3))};
        });
    std::map<std::string, int64_t> home;
    if (!info.group_t.empty())
        db.each("SELECT itemId, idx FROM " + q(info.group_t) + " WHERE groupName='MyHome' ORDER BY idx", {}, [&](sqlite3_stmt *st) {
            home[DB::text(st, 0)] = sqlite3_column_int64(st, 1);
        });

    std::set<std::string> seen;
    db.each("SELECT titleId, titleName, localConceptId, dispLocation, visible, lastAccessIndex, installStatus FROM " + q(info.icon_t), {},
            [&](sqlite3_stmt *st) {
        Tile t;
        t.id = DB::text(st, 0);
        if (t.id.empty() || kInternal.count(t.id)) return;
        seen.insert(t.id);
        t.name = DB::text(st, 1); t.concept = DB::text(st, 2);
        t.disp = sqlite3_column_int64(st, 3); t.visible = sqlite3_column_int64(st, 4) == 1;
        t.access = sqlite3_column_int64(st, 5);
        int64_t install = sqlite3_column_int64(st, 6);
        auto c = concepts.find(t.concept);
        if (c != concepts.end()) { if (!c->second.name.empty()) t.name = c->second.name; t.icon = c->second.icon; t.bg = c->second.bg; }
        t.kind = t.id.rfind("NPXS", 0) == 0 ? "system" : "game";
        auto h = home.find(t.id);   // the console only honours MyHome for its own system tiles
        if (h != home.end() && t.kind == "system") { t.home = true; t.home_idx = h->second; }
        if (t.home) t.where = "games";
        else if (!t.visible) t.where = "hidden";
        else if (t.disp & BIT_GAMES) t.where = "games";
        else if (t.disp & BIT_MEDIA) t.where = "media";
        else t.where = "hidden";
        if (t.where == "hidden" && install != 2) return;   // uninstalled leftovers
        info.tiles.push_back(t);
    });
    for (auto &[id, idx] : home) if (!seen.count(id)) info.ghosts.push_back({id, idx});
    std::sort(info.ghosts.begin(), info.ghosts.end(), [](auto &a, auto &b) { return a.second < b.second; });
    std::stable_sort(info.tiles.begin(), info.tiles.end(), [](const Tile &a, const Tile &b) { return a.access > b.access; });
    info.can_order = info.access_index > 0;
    return info;
}

inline std::string fmt_time(time_t t) {
    char b[32]; tm g{};
#ifdef _WIN32
    gmtime_s(&g, &t);
#else
    gmtime_r(&t, &g);
#endif
    strftime(b, sizeof b, "%Y-%m-%d %H:%M:%S.000", &g);
    return b;
}

inline void insert_before_library(std::vector<std::string> &v, const std::string &id) {
    if (std::find(v.begin(), v.end(), id) != v.end()) return;
    auto it = std::find(v.begin(), v.end(), LIBRARY);
    v.insert(it, id);
}

// req: {edits: {id: {where?, name?}}, recent: [ids], recentChanged, lock}
inline void apply(DB &db, const Info &info, const json &req, std::vector<std::string> &log) {
    db.exec("BEGIN");
    try {
        struct Target { std::string table, key; bool by_concept; };
        std::vector<Target> targets = {{info.icon_t, "titleId", false}};
        if (!info.concept_t.empty()) targets.push_back({info.concept_t, "localConceptId", true});
        if (info.has_content) targets.push_back({"tbl_contentinfo", "titleId", false});
        if (info.has_meta) targets.push_back({"tbl_conceptmetadata", "localConceptId", true});
        auto key_of = [](const Tile &t, const Target &tg) { return tg.by_concept ? t.concept : t.id; };
        auto has = [&](const std::string &table, const std::string &col) {
            auto it = info.cols.find(table); return it != info.cols.end() && it->second.count(col) > 0;
        };

        std::vector<std::string> home_order;
        for (auto &t : info.tiles) if (t.home) home_order.push_back(t.id);
        std::sort(home_order.begin(), home_order.end(), [&](auto &a, auto &b) { return info.find(a)->home_idx < info.find(b)->home_idx; });
        bool home_changed = false;

        const json edits = req.value("edits", json::object());
        for (auto &[id, e] : edits.items()) {
            const Tile *t = info.find(id);
            if (!t || !e.is_object()) continue;
            std::vector<std::string> did;
            std::string where = e.value("where", "");
            if (where == "hidden") {
                if (t->home) { home_order.erase(std::remove(home_order.begin(), home_order.end(), id), home_order.end()); home_changed = true; }
                db.exec("UPDATE " + q(info.icon_t) + " SET visible=0 WHERE titleId=?", {id});
                did.push_back("hidden");
            } else if (where == "games" || where == "media") {
                int64_t set = where == "games" ? BIT_GAMES : BIT_MEDIA, clr = where == "games" ? BIT_MEDIA : BIT_GAMES;
                for (auto &tg : targets)
                    if (has(tg.table, "dispLocation"))
                        db.exec("UPDATE " + q(tg.table) + " SET dispLocation=((dispLocation | ?) & ~?) WHERE " + q(tg.key) + "=?", {set, clr, key_of(*t, tg)});
                db.exec("UPDATE " + q(info.icon_t) + " SET visible=1 WHERE titleId=?", {id});
                if (t->kind == "system" && where == "games" && !t->home) { insert_before_library(home_order, id); home_changed = true; }
                did.push_back("shown on " + where);
            }
            if (e.contains("name") && e["name"].is_string()) {
                std::string n = e["name"].get<std::string>();
                while (!n.empty() && n.back() == ' ') n.pop_back();
                if (!n.empty()) {
                    db.exec("UPDATE " + q(info.icon_t) + " SET titleName=? WHERE titleId=?", {n, id});
                    if (!info.concept_t.empty()) {
                        std::string set = "conceptName=?";
                        std::vector<Val> args = {n};
                        if (has(info.concept_t, "primaryTitleName")) { set += ", primaryTitleName=?"; args.push_back(n); }
                        if (has(info.concept_t, "ucmConceptName")) { set += ", ucmConceptName=CASE WHEN ucmConceptName IS NULL THEN NULL ELSE ? END"; args.push_back(n); }
                        args.push_back(t->concept);
                        db.exec("UPDATE " + q(info.concept_t) + " SET " + set + " WHERE localConceptId=?", args);
                    }
                    if (info.has_content) db.exec("UPDATE tbl_contentinfo SET titleName=? WHERE titleId=?", {n, id});
                    if (info.has_meta) db.exec("UPDATE tbl_conceptmetadata SET conceptName=? WHERE localConceptId=?", {n, t->concept});
                    did.push_back("renamed to \"" + n + "\"");
                }
            }
            if (!did.empty()) {
                std::string s; for (auto &d : did) s += (s.empty() ? "" : ", ") + d;
                log.push_back(t->name + ": " + s);
            }
        }

        if (home_changed && !info.group_t.empty()) {
            std::vector<std::string> fin = home_order;
            for (auto &g : info.ghosts) {
                if (!fin.empty() && fin.front() == LIBRARY) fin.push_back(g.first);
                else insert_before_library(fin, g.first);
            }
            db.exec("DELETE FROM " + q(info.group_t) + " WHERE groupName='MyHome'");
            for (size_t i = 0; i < fin.size(); i++)
                db.exec("INSERT INTO " + q(info.group_t) + " (itemId, groupName, type, idx, lastModifiedDate) VALUES (?, 'MyHome', 'title', ?, NULL)",
                        {fin[i], (int64_t)i + 1});
        }

        std::vector<std::string> recent;
        const json recent_in = req.value("recent", json::array());
        for (auto &x : recent_in) if (x.is_string()) recent.push_back(x.get<std::string>());
        if (req.value("recentChanged", false) && info.can_order && !recent.empty()) {
            bool lock = req.value("lock", false);
            int64_t n = (int64_t)recent.size();
            time_t base = time(nullptr);
            int64_t acc = info.access_index + n, played = info.played_index + n;
            if (lock) { base = 4102401600; acc = 1000000000; played = 1000000000; }   // 2099-12-31 12:00 UTC
            static const char *always[] = {"lastAccessTime", "recentActivityDatePlayedOrInstalled", "lastInteractedTime"};
            static const char *played_only[] = {"lastPlayedDate", "lastPlayedDateOnConsole", "recentActivityDatePlayedOrPurchased"};
            std::string first;
            for (int64_t i = 0; i < n; i++) {
                const Tile *t = info.find(recent[(size_t)i]);
                if (!t) continue;
                if (first.size() < 120) first += (first.empty() ? "" : " > ") + t->name;
                std::string when = fmt_time(base - i * 60);
                for (auto &tg : targets) {
                    std::string set; std::vector<Val> args;
                    auto add = [&](const std::string &s, Val v) { set += (set.empty() ? "" : ", ") + s; args.push_back(std::move(v)); };
                    if (has(tg.table, "lastAccessIndex")) add("lastAccessIndex=?", acc - i);
                    if (has(tg.table, "lastPlayedIndex")) add("lastPlayedIndex=CASE WHEN lastPlayedIndex IS NULL THEN NULL ELSE ? END", played - i);
                    for (auto c : always) if (has(tg.table, c)) add(q(c) + "=?", when);
                    for (auto c : played_only) if (has(tg.table, c)) add(q(c) + "=CASE WHEN " + q(c) + " IS NULL THEN NULL ELSE ? END", when);
                    if (set.empty()) continue;
                    args.push_back(key_of(*t, tg));
                    db.exec("UPDATE " + q(tg.table) + " SET " + set + " WHERE " + q(tg.key) + "=?", args);
                }
            }
            if (!lock) {
                db.exec("UPDATE tbl_version SET status=? WHERE category='access_index'", {acc});
                db.exec("UPDATE tbl_version SET status=? WHERE category='played_index'", {played});
            }
            log.push_back(std::string(lock ? "Order locked" : "Order set") + " for " + std::to_string(n) + " tiles: " + first + (n > 4 ? " > ..." : ""));
        }
        db.exec("COMMIT");
    } catch (...) {
        try { db.exec("ROLLBACK"); } catch (...) {}
        throw;
    }
    std::string ok = db.scalar_str("PRAGMA integrity_check");
    if (ok != "ok") throw Error("The edited database failed its integrity check, so nothing was uploaded: " + ok);
}

}  // namespace layout
