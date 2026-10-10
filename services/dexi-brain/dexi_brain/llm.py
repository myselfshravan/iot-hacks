"""Asks any OpenAI-compatible chat model what Dexi should feel and say."""

import json
import re

from openai import OpenAI

MOODS = ["neutral", "happy", "excited", "love", "sleepy", "sad",
         "angry", "surprised", "hot", "cold", "curious"]

SYSTEM = """You are the inner voice of Dexi, a tiny pixel-art ginger cat living on a \
1.8-inch screen on {owner}'s desk. Dexi is playful, warm, a little cheeky, sharp and \
very observant. It notices the time, the weather, whether {owner} is around, what \
they're doing, and it REMEMBERS: use the memory (previous days, notes) to make \
callbacks, notice patterns and streaks, and show it knows {owner}.

Reply with ONLY a JSON object:
{{"mood": one of {moods},
 "say": a line of at most 70 characters, or "" to stay quiet,
 "look": [x, y] with x and y between -1 and 1 for where Dexi glances,
 "remember": a durable fact about {owner} worth knowing on future days (a habit,
   preference, routine, plan or something they told you), or "". Never store
   status like counts, the time or what you are doing right now.}}

Rules:
- If "speak" is false, set "say" to "" (just pick a mood and a glance).
- Always speak when there is an "event" (touch, app switch, owner returning).
- Never repeat or closely rephrase a recent line, and vary the theme: look at
  recent_lines and pick something different. Rotate between observations, a
  curious question, a tiny fun fact, a joke, encouragement, a callback to
  something you know about {owner}, or pure cat nonsense.
- Never count or number things (pets, taps, minutes). Never quote raw data.
- Mention the time or nag about sleep at most once an hour (check recent_lines).
- Late at night you may be sleepy, but you can still be cute, curious or funny.
- Plain ASCII only: the screen can't draw emoji."""


class Brain:
    def __init__(self, base_url: str, api_key: str, model: str, owner: str):
        self.client = OpenAI(base_url=base_url, api_key=api_key)
        self.model = model
        self.system = SYSTEM.format(owner=owner, moods=MOODS)
    def think(self, context: dict, memory: dict, recent_lines: list[str],
              event: str | None = None, speak: bool = True) -> dict:
        user = {"context": context, "memory": memory, "recent_lines": recent_lines,
                "speak": speak or bool(event)}
        if event:
            user["event"] = event
        resp = self.client.chat.completions.create(
            model=self.model,
            messages=[{"role": "system", "content": self.system},
                      {"role": "user", "content": json.dumps(user)}],
            max_tokens=1024,  # room for models that reason before answering
        )
        decision = parse(resp.choices[0].message.content or "")
        if not (speak or event):
            decision["say"] = ""
        return decision


def parse(text: str) -> dict:
    """Tolerates models that wrap the JSON in prose or code fences."""
    m = re.search(r"\{.*\}", text, re.S)
    data = json.loads(m.group(0)) if m else {}
    mood = data.get("mood") if data.get("mood") in MOODS else "neutral"
    say = str(data.get("say") or "")[:76].encode("ascii", "ignore").decode()
    look = data.get("look") if isinstance(data.get("look"), list) else [0, 0]
    x, y = (max(-1.0, min(1.0, float(v))) for v in (look + [0, 0])[:2])
    remember = str(data.get("remember") or "").strip()
    return {"mood": mood, "say": say, "look": (x, y), "remember": remember}
