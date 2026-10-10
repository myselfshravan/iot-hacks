"""Main loop: watch Dexi's touch pad, think on gestures and every THINK_EVERY_S.

Touch is read as whole gestures (press durations + gaps, recorded by the
firmware); gestures.describe() turns them into words and the LLM decides how
Dexi responds. A double-tap-like pair of quick presses also shows an info card.
"""

import logging
import time

from . import cards, config, context, gestures
from .dexi import Dexi
from .llm import Brain
from .memory import Memory
from .server import serve
from .state import Shared

POLL_S = 1
CARD_SECS = 8

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


def is_double_tap(g: dict) -> bool:
    return g.get("presses") == 2 and max(g.get("press_ms") or [0]) < 300 and \
        (g.get("gap_ms") or [999])[0] < 300


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s", datefmt="%H:%M:%S")
    for noisy in ("httpx", "openai", "httpcore"):  # one line per LLM call is noise
        logging.getLogger(noisy).setLevel(logging.WARNING)
    cfg = config.load()
    dexi = Dexi(cfg.dexi_url)
    brain = Brain(cfg.llm_base_url, cfg.llm_api_key, cfg.llm_model, cfg.owner)
    memory = Memory()
    shared = Shared()
    serve(cfg.console_host, cfg.console_port, dexi, memory, shared, cfg.llm_model)
    log.info("brain up: %s via %s, Dexi at %s (%s)", cfg.llm_model, cfg.llm_base_url,
             cfg.dexi_url, dexi.base)

    last_gesture = None
    while True:
        try:
            st = dexi.state()
            g = st.get("gesture", {})
            event = None
            if last_gesture is not None and g.get("id", 0) > last_gesture:
                memory.petted()
                event = gestures.describe(g)
                if is_double_tap(g):
                    show_card(dexi, cfg)
            last_gesture = g.get("id", 0)
            with shared.lock:
                shared.dexi_state, shared.status = st, "online"

            forced = shared.think_now.is_set()
            if event or forced or time.time() >= shared.next_think:
                shared.think_now.clear()
                ctx = context.gather(cfg.latitude, cfg.longitude, cfg.share_active_app)
                with shared.lock:
                    shared.status = "thinking"
                    if ctx.get("owner_away_for") == "at the laptop now":
                        memory.saw_owner()
                    summary, recent = memory.summary(), memory.recent_lines()
                dexi.think(secs=8)  # pet packs show their "thinking" animation
                decision = brain.think(ctx, summary, recent, event)
                act(dexi, decision)
                trigger = "touch: " + event if event else "console" if forced else "idle"
                with shared.lock:
                    memory.said(decision["say"])
                    if decision["remember"]:
                        memory.note(decision["remember"])
                    memory.log(trigger, ctx, decision)
                    memory.save()
                    shared.last_context = ctx
                    shared.next_think = time.time() + cfg.think_every_s
                    shared.status = "online"
                log.info("%s -> %s %r%s", trigger, decision["mood"], decision["say"],
                         f" [remembered: {decision['remember']}]" if decision["remember"] else "")
        except Exception as e:  # keep living through Wi-Fi blips and API errors
            log.warning("skipped a beat: %s", e)
            with shared.lock:
                shared.status = f"error: {e}"[:200]
                shared.next_think = time.time() + 30
            time.sleep(4)
        shared.think_now.wait(POLL_S)


if __name__ == "__main__":
    main()
