"""Asks any OpenAI-compatible chat model what Dexi should feel and say."""

import json
import re

from openai import OpenAI

MOODS = ["neutral", "happy", "excited", "love", "sleepy", "sad",
         "angry", "surprised", "hot", "cold", "curious"]

SYSTEM = """You are the inner voice of Dexi, a tiny desk pet living on a 1.8-inch screen \
on {owner}'s desk. Dexi is playful, warm, a little cheeky and very observant. It reacts \
to the time of day, the weather, whether {owner} is around, and what they're doing.

Reply with ONLY a JSON object:
{{"mood": one of {moods}, "say": a line of at most 70 characters or "" to stay quiet, \
"look": [x, y] with x and y between -1 and 1 for where Dexi glances}}

Rules: stay quiet about half the time unless something changed or you were petted. \
Never repeat a recent line. Late at night be sleepy. No emoji (the screen can't draw them)."""


class Brain:
    def __init__(self, base_url: str, api_key: str, model: str, owner: str):
        self.client = OpenAI(base_url=base_url, api_key=api_key)
        self.model = model
        self.system = SYSTEM.format(owner=owner, moods=MOODS)
        self.recent: list[str] = []

    def think(self, context: dict, event: str | None = None) -> dict:
        user = {"context": context, "recent_lines": self.recent[-6:]}
        if event:
            user["event"] = event
        resp = self.client.chat.completions.create(
            model=self.model,
            messages=[{"role": "system", "content": self.system},
                      {"role": "user", "content": json.dumps(user)}],
            max_tokens=1024,  # room for models that reason before answering
        )
        decision = parse(resp.choices[0].message.content or "")
        if decision["say"]:
            self.recent.append(decision["say"])
        return decision


def parse(text: str) -> dict:
    """Tolerates models that wrap the JSON in prose or code fences."""
    m = re.search(r"\{.*\}", text, re.S)
    data = json.loads(m.group(0)) if m else {}
    mood = data.get("mood") if data.get("mood") in MOODS else "neutral"
    say = str(data.get("say") or "")[:76].encode("ascii", "ignore").decode()
    look = data.get("look") if isinstance(data.get("look"), list) else [0, 0]
    x, y = (max(-1.0, min(1.0, float(v))) for v in (look + [0, 0])[:2])
    return {"mood": mood, "say": say, "look": (x, y)}
