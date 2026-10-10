"""Main loop: watch Dexi's touch pad, think on events and every THINK_EVERY_S.

Touch gestures (counted by the firmware, read from /state):
  tap         -> Dexi says something clever or fun (LLM)
  double tap  -> info card: now playing if music is on, else time + weather
  long press  -> a cuddle; Dexi responds warmly (LLM)
"""

import logging
import time

from . import cards, config, context
from .dexi import Dexi
from .llm import Brain
from .memory import Memory

POLL_S = 1
CARD_SECS = 8
TAP_EVENT = ("you were poked once: say something clever, curious or funny that fits "
             "the moment (a thought, a fact, a joke, a callback to memory)")
CUDDLE_EVENT = "you are getting a long cuddle: respond warmly"

log = logging.getLogger("dexi-brain")


def act(dexi: Dexi, decision: dict) -> None:
    dexi.mood(decision["mood"])
    x, y = decision["look"]
    dexi.look(x, y, secs=4)
    if decision["say"]:
        dexi.say(decision["say"], secs=10)


def show_card(dexi: Dexi, cfg: config.Config) -> None:
    song = cards.now_playing()
    img = cards.music_card(song) if song else cards.weather_card(
        context.weather_data(cfg.latitude, cfg.longitude))
    dexi.show_image(img, secs=CARD_SECS)
    log.info("card: %s", f"{song['track']} - {song['artist']}" if song else "time + weather")


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s", datefmt="%H:%M:%S")
    logging.getLogger("httpx").setLevel(logging.WARNING)  # one line per LLM call is noise
    cfg = config.load()
    dexi = Dexi(cfg.dexi_url)
    brain = Brain(cfg.llm_base_url, cfg.llm_api_key, cfg.llm_model, cfg.owner)
    memory = Memory()
    log.info("brain up: %s via %s, Dexi at %s (%s)", cfg.llm_model, cfg.llm_base_url,
             cfg.dexi_url, dexi.base)

    seen = None  # last (taps, double_taps, long_presses) from the firmware
    next_think = 0.0
    while True:
        try:
            st = dexi.state()
            counts = (st.get("taps", 0), st.get("double_taps", 0), st.get("long_presses", 0))
            taps, doubles, longs = (max(0, a - b) for a, b in zip(counts, seen)) if seen else (0, 0, 0)
            seen = counts
            if taps or longs:
                memory.petted(taps + longs)

            event = CUDDLE_EVENT if longs else TAP_EVENT if taps else None
            if doubles:
                show_card(dexi, cfg)

            if event or time.time() >= next_think:
                ctx = context.gather(cfg.latitude, cfg.longitude, cfg.share_active_app)
                if ctx.get("owner_away_for") == "at the laptop now":
                    memory.saw_owner()
                decision = brain.think(ctx, memory.summary(), memory.recent_lines(), event)
                act(dexi, decision)
                memory.said(decision["say"])
                if decision["remember"]:
                    memory.note(decision["remember"])
                memory.save()
                log.info("%s -> %s %r%s", "tap" if taps else "cuddle" if longs else "idle",
                         decision["mood"], decision["say"],
                         f" [remembered: {decision['remember']}]" if decision["remember"] else "")
                next_think = time.time() + cfg.think_every_s
        except Exception as e:  # keep living through Wi-Fi blips and API errors
            log.warning("skipped a beat: %s", e)
            next_think = time.time() + 30
            time.sleep(5)
        time.sleep(POLL_S)


if __name__ == "__main__":
    main()
