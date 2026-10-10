"""Main loop: think every THINK_EVERY_S, or immediately when Dexi is petted."""

import logging
import time

from . import config, context
from .dexi import Dexi
from .llm import Brain

POLL_S = 2

log = logging.getLogger("dexi-brain")


def act(dexi: Dexi, decision: dict) -> None:
    dexi.mood(decision["mood"])
    x, y = decision["look"]
    dexi.look(x, y, secs=4)
    if decision["say"]:
        dexi.say(decision["say"], secs=10)


def main() -> None:
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(message)s", datefmt="%H:%M:%S")
    cfg = config.load()
    dexi = Dexi(cfg.dexi_url)
    brain = Brain(cfg.llm_base_url, cfg.llm_api_key, cfg.llm_model, cfg.owner)
    log.info("brain up: %s via %s, Dexi at %s", cfg.llm_model, cfg.llm_base_url, cfg.dexi_url)

    pets = None
    next_think = 0.0
    while True:
        try:
            st = dexi.state()
            total = st["taps"] + st["long_presses"]
            event = None
            if pets is not None and total > pets:
                event = "you were just petted" + (" (a long cuddle)" if st["long_presses"] else "")
            pets = total

            if event or time.time() >= next_think:
                ctx = context.gather(cfg.latitude, cfg.longitude, cfg.share_active_app)
                decision = brain.think(ctx, event)
                act(dexi, decision)
                log.info("%s -> %s %r", event or "idle", decision["mood"], decision["say"])
                next_think = time.time() + cfg.think_every_s
        except Exception as e:  # keep living through Wi-Fi blips and API errors
            log.warning("skipped a beat: %s", e)
            next_think = time.time() + 30
        time.sleep(POLL_S)


if __name__ == "__main__":
    main()
