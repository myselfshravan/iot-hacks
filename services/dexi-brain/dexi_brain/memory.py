"""Dexi's long-term memory, kept in memory.json next to .env (git-ignored).

Tracks when the owner is around each day, how often Dexi was petted, what Dexi
said recently, and short notes the LLM chooses to remember."""

import datetime as dt
import json
import re
from pathlib import Path

PATH = Path(__file__).resolve().parent.parent / "memory.json"
MAX_NOTES, MAX_LINES, MAX_DAYS, MAX_LOG = 40, 30, 30, 300

# Status snapshots that aren't worth remembering: clock times, counts of pets.
STATUS = re.compile(
    r"\b\d{1,2}:\d{2}\b|\b\d+\s*(pets?|pokes?|taps?|times)\b"
    r"|\b(pets?|pokes?|taps?)\s+total\b|\b(today|tonight|right now|just now)\b", re.I)


class Memory:
    def __init__(self, path: Path = PATH):
        self.path = path
        self.data = {"notes": [], "lines": [], "days": {}, "log": []}
        if path.exists():
            self.data.update(json.loads(path.read_text()))

    def save(self) -> None:
        days = self.data["days"]
        for old in sorted(days)[:-MAX_DAYS]:
            del days[old]
        self.data["notes"] = self.data["notes"][-MAX_NOTES:]
        self.data["lines"] = self.data["lines"][-MAX_LINES:]
        self.data["log"] = self.data["log"][-MAX_LOG:]
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

    def note(self, text: str) -> bool:
        """Keeps durable facts only; rejects status snapshots (times, counts, today's app)."""
        text = text.strip()[:140]
        if not text or STATUS.search(text) or text in (n["text"] for n in self.data["notes"]):
            return False
        self.data["notes"].append({"date": dt.date.today().isoformat(), "text": text})
        return True

    def log(self, trigger: str, context: dict, decision: dict) -> dict:
        """Records one thought for the console's replayable log."""
        log = self.data["log"]
        entry = {"id": (log[-1]["id"] + 1) if log else 1,
                 "time": dt.datetime.now().isoformat(timespec="seconds"),
                 "trigger": trigger, "app": context.get("owner_is_using"),
                 "mood": decision["mood"], "say": decision["say"],
                 "remember": decision.get("remember", "")}
        log.append(entry)
        return entry

    def find_log(self, entry_id: int) -> dict | None:
        return next((e for e in self.data["log"] if e["id"] == entry_id), None)

    def delete_note(self, index: int) -> None:
        if 0 <= index < len(self.data["notes"]):
            del self.data["notes"][index]

    def recent_lines(self, n: int = 8) -> list[str]:
        return self.data["lines"][-n:]

    def summary(self) -> dict:
        """What the LLM gets to see. Deliberately qualitative: exact counts and
        timestamps here get recited back ("pet 27!") instead of used."""
        days = self.data["days"]
        today = dt.date.today()

        def day_view(d: dict) -> dict:
            pets = d.get("pets", 0)
            affection = "none" if not pets else "a little" if pets < 5 else "a lot" if pets < 20 else "tons"
            return {"petting": affection, "around_from": d.get("first_seen"), "until": d.get("last_seen")}

        week = []
        for i in range(1, 8):
            d = (today - dt.timedelta(days=i))
            if d.isoformat() in days:
                week.append({"day": d.strftime("%A"), **day_view(days[d.isoformat()])})
        return {
            "today": day_view(days.get(today.isoformat(), {})),
            "previous_days": week,
            "things_you_know_about_owner": [n["text"] for n in self.data["notes"][-15:]],
        }
