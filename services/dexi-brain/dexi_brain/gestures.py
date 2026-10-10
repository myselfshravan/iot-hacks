"""Turns the raw touch gesture from the firmware into words for the LLM."""


def describe(g: dict) -> str:
    """e.g. "3 quick taps (90, 110, 80 ms, ~150 ms apart)" or "held for 3.2 s"."""
    presses, press_ms, gaps = g.get("presses", 0), g.get("press_ms", []), g.get("gap_ms", [])
    if not presses:
        return ""
    longest = max(press_ms) if press_ms else 0
    if presses == 1:
        if longest >= 1500:
            return f"you were held/stroked for {longest / 1000:.1f} s, a long cuddle"
        if longest >= 400:
            return f"you got one slow, deliberate press ({longest} ms)"
        return f"you got a single quick boop ({longest} ms)"
    avg_gap = sum(gaps) / len(gaps) if gaps else 0
    pace = "frantic" if avg_gap < 150 else "quick" if avg_gap < 300 else "slow"
    desc = (f"you were tapped {presses} times, {pace} (presses {', '.join(map(str, press_ms))} ms, "
            f"~{avg_gap:.0f} ms apart)")
    if longest >= 1200:
        desc += f", ending in a {longest / 1000:.1f} s hold"
    return desc
