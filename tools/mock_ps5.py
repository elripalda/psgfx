#!/usr/bin/env python3
"""File-backed stand-in for PS5 Upload's payload (development only), following its runtime.c + register.c.
Usage: python3 tools/mock_ps5.py ROOT MGMT_PORT XFER_PORT   (set MOCK_FW13=1 to mimic firmware 12/13 register behaviour)
Serves management (FTX2) and transfer frames on two ports; paths map under ROOT."""
import base64, json, os, shutil, socket, struct, sys, threading

ROOT = sys.argv[1]; MPORT = int(sys.argv[2]); XPORT = int(sys.argv[3])
ALLOWED = ("/data/", "/user/", "/mnt/")
txs = {}

def P(p): return os.path.join(ROOT, p.lstrip("/"))
def allowed(p): return p.startswith(ALLOWED) and ".." not in p

def recv_exact(s, n):
    b = b""
    while len(b) < n:
        c = s.recv(n - len(b))
        if not c: raise EOFError
        b += c
    return b

def send(s, t, body=b""):
    if isinstance(body, str): body = body.encode()
    s.sendall(struct.pack("<IHHIQQ", 0x32585446, 1, t, 0, len(body), 0) + body)

def err(s, m): send(s, 3, m)

def handle(s, t, body):
    j = {}
    if body[:1] == b"{":
        try: j = json.loads(body)
        except Exception: pass
    if t == 62:  # APP_LIST_REGISTERED
        apps = []
        base = P("/user/app")
        for tid in sorted(os.listdir(base)) if os.path.isdir(base) else []:
            lnk = os.path.join(base, tid, "mount.lnk"); src = open(lnk).read() if os.path.exists(lnk) else ""
            name = tid
            pj = os.path.join(base, tid, "sce_sys", "param.json")
            if os.path.exists(pj):
                name = json.load(open(pj))["localizedParameters"]["en-US"]["titleName"]
            apps.append({"title_id": tid, "title_name": name, "src": src, "image_backed": False})
        return send(s, 63, json.dumps({"apps": apps}))
    if t == 120:  # APPDB_QUERY - names from app.db (mocked by ROOT/appdb.json)
        dbp = os.path.join(ROOT, "appdb.json")   # {"apps":[{"title_id":..,"app_id":..,"name":..}]}
        if os.environ.get("MOCK_NO_APPDB") or not os.path.exists(dbp): return err(s, "unsupported_in_mock")
        return send(s, 121, open(dbp).read())
    if t == 48:  # FS_READ
        p = j.get("path", "")
        if not allowed(p): return err(s, "fs_read_path_not_allowed")
        if not os.path.isfile(P(p)): return err(s, "fs_read_stat_failed")
        off = j.get("offset", 0); lim = min(j.get("limit", 2 << 20) or (2 << 20), 2 << 20)
        with open(P(p), "rb") as f: f.seek(off); return send(s, 49, f.read(lim))
    if t == 36:  # FS_LIST_DIR
        p = j.get("path", "")
        if not os.path.isdir(P(p)): return err(s, "fs_list_dir_opendir_errno_2")
        ents = [{"name": n, "kind": "dir" if os.path.isdir(os.path.join(P(p), n)) else "file",
                 "size": os.path.getsize(os.path.join(P(p), n)), "mtime": 0} for n in sorted(os.listdir(P(p)))]
        return send(s, 37, json.dumps({"path": p, "entries": ents, "truncated": False}))
    if t == 50:  # FS_COPY
        a, b = j.get("from", ""), j.get("to", "")
        if not (allowed(a) and allowed(b)): return err(s, "fs_copy_path_not_allowed")
        if not os.path.exists(P(a)): return err(s, "fs_copy_source_missing")
        ow = j.get("overwrite", 0)
        if os.path.exists(P(b)) and not (isinstance(ow, int) and not isinstance(ow, bool) and ow != 0): return err(s, "fs_copy_dest_exists")
        shutil.copyfile(P(a), P(b)); return send(s, 51)
    if t == 40:  # FS_DELETE
        p = j.get("path", "")
        if not allowed(p): return err(s, "fs_delete_path_not_allowed")
        try: os.remove(P(p))
        except FileNotFoundError: return err(s, "fs_delete_failed")
        return send(s, 41)
    if t == 130:  # FS_WRITE_BYTES
        p = j.get("path", "")
        if not allowed(p): return send(s, 131, '{"ok":false,"err":"path_unsafe"}')
        data = base64.b64decode(j["bytes"])
        open(P(p), "wb").write(data); return send(s, 131, '{"ok":true,"size":%d}' % len(data))
    if t == 56:  # APP_REGISTER - stage sce_sys like register.c
        src = j.get("src_path", "").rstrip("/")
        pj = json.load(open(P(src + "/sce_sys/param.json"))); tid = pj["titleId"]
        ua = P("/user/app/" + tid); meta = P("/user/appmeta/" + tid)
        os.makedirs(ua, exist_ok=True); os.makedirs(meta, exist_ok=True)
        shutil.copytree(P(src + "/sce_sys"), ua + "/sce_sys", dirs_exist_ok=True)
        if os.path.exists(P(src + "/sce_sys/icon0.png")): shutil.copyfile(P(src + "/sce_sys/icon0.png"), ua + "/icon0.png")
        for n in os.listdir(P(src + "/sce_sys")):
            if n == "param.json" or n.lower().endswith((".png", ".dds", ".at9")):
                shutil.copyfile(P(src + "/sce_sys/" + n), os.path.join(meta, n))
        open(ua + "/mount.lnk", "w").write(src)
        print("REGISTER", tid, "from", src, flush=True)
        if os.environ.get("MOCK_FW13"): return err(s, "register_install_api_unavailable")
        return send(s, 57, json.dumps({"title_id": tid, "title_name": tid, "used_nullfs": True}))
    if t == 10:  # BEGIN_TX
        tx = body[:16]; man = json.loads(body[24:])
        txs[tx] = {"dest": man["dest_root"], "data": b""}; return send(s, 11, '{"accepted":true,"last_acked_shard":0}')
    if t == 30:  # STREAM_SHARD
        tx = body[:16]; txs[tx]["data"] += body[64:]
        return send(s, 31, tx + struct.pack("<Q", 1) + bytes([1]) + bytes(7) + struct.pack("<QQ", len(body) - 64, 1))
    if t == 14:  # COMMIT_TX
        tx = body[:16]; d = txs.pop(tx)
        if not allowed(d["dest"]): return err(s, "path_not_allowed")
        os.makedirs(os.path.dirname(P(d["dest"])), exist_ok=True)
        open(P(d["dest"]), "wb").write(d["data"]); print("WROTE", d["dest"], len(d["data"]), flush=True)
        return send(s, 15, '{"committed":true}')
    return err(s, "unsupported_in_mock")

def serve_conn(s):
    try:
        while True:
            h = recv_exact(s, 28)
            magic, ver, t, flags, blen, trace = struct.unpack("<IHHIQQ", h)
            handle(s, t, recv_exact(s, blen) if blen else b"")
    except (EOFError, ConnectionResetError, BrokenPipeError):
        pass
    finally:
        s.close()

def listen(port):
    ls = socket.socket(); ls.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    ls.bind(("127.0.0.1", port)); ls.listen(16)
    while True:
        c, _ = ls.accept(); threading.Thread(target=serve_conn, args=(c,), daemon=True).start()

threading.Thread(target=listen, args=(XPORT,), daemon=True).start()
print("mock ps5 up", flush=True)
listen(MPORT)
