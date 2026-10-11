#!/usr/bin/env python3
"""Stand-in for the PS5 Upload app's local engine (development only): serves a folder on your computer
as if it were a PS5, through the same HTTP routes PSGFX uses (/api/mgmt/call, /api/ps5/fs/copy, /api/ps5/fs/delete).
Usage: python3 tools/mock_engine.py ROOT PORT   (MOCK_CHUNKED=1 answers with chunked transfer encoding)"""
import base64, json, os, shutil, sys
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
ROOT, PORT, CHUNKED = sys.argv[1], int(sys.argv[2]), os.environ.get("MOCK_CHUNKED") == "1"
FS_READ_MAX = 262128
def local(p): return os.path.join(ROOT, p.lstrip("/"))
class H(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    def log_message(self, *a): pass
    def reply(self, code, obj):
        data = json.dumps(obj).encode()
        self.send_response(code); self.send_header("Content-Type", "application/json"); self.send_header("Connection", "close")
        if CHUNKED:
            self.send_header("Transfer-Encoding", "chunked"); self.end_headers()
            for i in range(0, len(data), 1000): c = data[i:i+1000]; self.wfile.write(b"%x\r\n%s\r\n" % (len(c), c))
            self.wfile.write(b"0\r\n\r\n")
        else:
            self.send_header("Content-Length", str(len(data))); self.end_headers(); self.wfile.write(data)
    def do_POST(self):
        req = json.loads(self.rfile.read(int(self.headers["Content-Length"])))
        if self.path == "/api/ps5/fs/copy":
            if not os.path.exists(local(req["from"])): return self.reply(502, {"error": "copy failed: source missing"})
            os.makedirs(os.path.dirname(local(req["to"])), exist_ok=True); shutil.copy(local(req["from"]), local(req["to"]))
            return self.reply(200, {"ok": True})
        if self.path == "/api/ps5/fs/delete":
            p = local(req["path"]); os.path.exists(p) and os.remove(p); return self.reply(200, {"ok": True})
        if self.path != "/api/mgmt/call": return self.reply(404, {"error": "no route"})
        body = json.loads(base64.b64decode(req["body_b64"]) or b"{}")
        m = req["method"]
        def ok(raw): return self.reply(200, {"ok": True, "result": {"kind": "bytes", "body_b64": base64.b64encode(raw).decode()}})
        def bad(cause): return self.reply(200, {"ok": False, "error": {"kind": "mgmt", "label": req["label"], "status": 5, "cause": cause}})
        if m == 33:
            d = local(body["path"])
            if not os.path.isdir(d): return bad("fs_list_open_failed")
            return ok(json.dumps({"path": body["path"], "entries": [{"name": n, "kind": "dir" if os.path.isdir(os.path.join(d, n)) else "file", "size": 0, "mtime": 0} for n in sorted(os.listdir(d))]}).encode())
        if m == 38:
            if body["path"].startswith("/system") and not body.get("unsafe"): return bad("fs_read_path_not_allowed")
            p = local(body["path"])
            if not os.path.isfile(p): return bad("fs_read_stat_failed")
            with open(p, "rb") as f: f.seek(body.get("offset", 0)); return ok(f.read(min(body.get("limit", FS_READ_MAX), FS_READ_MAX)))
        if m == 39:
            data = base64.b64decode(body["bytes"])
            if len(data) > (16 << 20 if body.get("large") else 256 << 10): return bad("too_large")
            if not os.path.isdir(os.path.dirname(local(body["path"]))): return bad("fs_write_no_tmp_file")   # like the payload: no folder, no tmp file
            open(local(body["path"]), "wb").write(data); return ok(b"")
        if m == 35: os.makedirs(local(body["path"]), exist_ok=True); return ok(b"")
        if m == 51: return ok(json.dumps({"apps": [{"title_id": "PPSA99000", "title_name": "ProsperoStore", "src": "/data/homebrew/PPSA99000"}]}).encode())
        if m == 56: return ok(b'{"apps":[{"title_id":"PPSA01234","name":"Some Game"}]}')
        if m == 48: return ok(b'{"ok":true}')
        if m == 1000: return bad("ava1_not_paired")
        return bad("unknown method %d" % m)
ThreadingHTTPServer(("127.0.0.1", PORT), H).serve_forever()
