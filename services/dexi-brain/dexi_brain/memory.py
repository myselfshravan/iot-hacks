"""Dexi's long-term memory, kept in memory.json next to .env (git-ignored).

Tracks when the owner is around each day, how often Dexi was petted, what Dexi
said recently, and short notes the LLM chooses to remember."""

import datetime as dt
import json
from pathlib import Path

PATH = Path(__file__).resolve().parent.parent / "memory.json"
MAX_NOTES, MAX_LINES, MAX_DAYS = 40, 30, 30


class Memory:
    def __init__(self, path: Path = PATH):
        self.path = path
        self.data = {"notes": [], "lines": [], "days": {}}
        if path.exists():
            self.data.update(json.loads(path.read_text()))

    def save(self) -> None:
        days = self.data["days"]
        for old in sorted(days)[:-MAX_DAYS]:
            del days[old]
        self.data["notes"] = self.data["notes"][-MAX_NOTES:]
        self.data["lines"] = self.data["lines"][-MAX_LINES:]
        self.path.write_text(json.dumps(self.data, indent=1))

    def _today(self) -> dict:
        key = dt.date.today().isoformat()
        return self.data["days"].setdefault(key, {"pets": 0, "first_seen": None, "last_seen": None})

    def saw_owner(self) -> None:
        day, now = self._today(), dt.datetime.now().strftime("%H:%M")
        day["first_seen"] = day["first_seen"] or now
        day["last_seen"] = now

    def petted(self, n: int = 1) -> None:
        self._today()["pets"] += n

    def said(self, line: str) -> None:
        if line:
            self.data["lines"].append(line)

    def note(self, text: str) -> None:
        text = text.strip()[:140]
        if text and text not in (n["text"] for n in self.data["notes"]):
            self.data["notes"].append({"date": dt.date.today().isoformat(), "text": text})

    def recent_lines(self, n: int = 8) -> list[str]:
        return self.data["lines"][-n:]

    def summary(self) -> dict:
        """What the LLM gets to see: today, the last week, and its notes."""
        days = self.data["days"]
        today = dt.date.today()
        week = []
        for i in range(1, 8):
            d = (today - dt.timedelta(days=i)).isoformat()
            if d in days:
                week.append({"date": d, **days[d]})
        return {
            "today": days.get(today.isoformat(), {}),
            "previous_days": week,
            "notes": [f"{n['date']}: {n['text']}" for n in self.data["notes"][-15:]],
        }
