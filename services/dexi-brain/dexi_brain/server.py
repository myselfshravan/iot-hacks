"""Console: a terminal-style dashboard + JSON API served from the brain.

  GET  /                      console page (console.html)
  GET  /api/status            Dexi's live state, the last context, timing
  GET  /api/log               thought log, newest first
  POST /api/replay            {"id": n}       replay a logged thought on Dexi
  GET  /api/memory            notes, days, recent lines
  POST /api/memory/note       {"text": "..."} add a note
  POST /api/memory/forget     {"index": n}    delete a note
  POST /api/say               {"text": "..."}
  POST /api/mood              {"mood": "..."}
  POST /api/think             think right now
"""

import json
import logging
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

from .dexi import Dexi
from .memory import Memory
from .state import Shared

PAGE = (Path(__file__).parent / "console.html").read_bytes()
log = logging.getLogger("dexi-brain")


def serve(host: str, port: int, dexi: Dexi, memory: Memory, shared: Shared, model: str) -> None:
    class Handler(BaseHTTPRequestHandler):
        def log_message(self, *args):  # keep the terminal for thoughts
            pass

        def _send(self, code: int, body: bytes, ctype: str = "application/json") -> None:
            self.send_response(code)
            self.send_header("Content-Type", ctype)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def _json(self, data, code: int = 200) -> None:
            self._send(code, json.dumps(data).encode())

        def _body(self) -> dict:
            n = int(self.headers.get("Content-Length") or 0)
            return json.loads(self.rfile.read(n) or b"{}")

        def do_GET(self):
            if self.path == "/":
                return self._send(200, PAGE, "text/html; charset=utf-8")
            with shared.lock:
                if self.path == "/api/status":
                    return self._json({
                        "dexi": shared.dexi_state, "context": shared.last_context,
                        "status": shared.status, "model": model, "dexi_url": dexi.base,
                        "next_think_s": max(0, round(shared.next_think - time.time())),
                        "uptime_s": round(time.time() - shared.started)})
                if self.path == "/api/log":
                    return self._json(list(reversed(memory.data["log"])))
                if self.path == "/api/memory":
                    d = memory.data
                    return self._json({"notes": d["notes"], "days": d["days"], "lines": d["lines"][-10:]})
            self._json({"error": "not found"}, 404)

        def do_POST(self):
            try:
                body = self._body()
                with shared.lock:
                    if self.path == "/api/replay":
                        e = memory.find_log(int(body["id"]))
                        if not e:
                            return self._json({"error": "no such thought"}, 404)
                        dexi.mood(e["mood"], secs=10)
                        if e["say"]:
                            dexi.say(e["say"], secs=10)
                        return self._json({"ok": True})
                    if self.path == "/api/memory/note":
                        memory.note(body["text"])
                        memory.save()
                        return self._json({"ok": True})
                    if self.path == "/api/memory/forget":
                        memory.delete_note(int(body["index"]))
                        memory.save()
                        return self._json({"ok": True})
                if self.path == "/api/say":
                    return self._json(dexi.say(body["text"], secs=10))
                if self.path == "/api/mood":
                    return self._json(dexi.mood(body["mood"]))
                if self.path == "/api/think":
                    shared.think_now.set()
                    return self._json({"ok": True})
                self._json({"error": "not found"}, 404)
            except Exception as e:
                self._json({"error": str(e)}, 500)

    server = ThreadingHTTPServer((host, port), Handler)
    threading.Thread(target=server.serve_forever, daemon=True).start()
    log.info("console: http://%s:%d", "localhost" if host in ("127.0.0.1", "0.0.0.0") else host, port)
