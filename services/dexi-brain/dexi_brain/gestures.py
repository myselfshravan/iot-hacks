"""Turns the raw touch gesture from the firmware into words for the LLM."""


def describe(g: dict) -> str:
    """Plain words only (numbers here get parroted back by the LLM)."""
    presses, press_ms, gaps = g.get("presses", 0), g.get("press_ms", []), g.get("gap_ms", [])
    if not presses:
        return ""
    longest = max(press_ms) if press_ms else 0
    if presses == 1:
        if longest >= 3000:
            return "a long, slow cuddle"
        if longest >= 1200:
            return "being gently held for a moment"
        if longest >= 400:
            return "one slow, deliberate press"
        return "a single quick boop"
    avg_gap = sum(gaps) / len(gaps) if gaps else 0
    pace = "frantic drumming" if avg_gap < 150 else "quick taps" if avg_gap < 300 else "slow taps"
    count = "a couple of" if presses == 2 else "a few" if presses <= 4 else "lots of"
    desc = f"{count} {pace}"
    if longest >= 1200:
        desc += ", ending in a hold"
    return desc
