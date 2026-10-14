"""Keep monitor screen sync alive: restart HyperHDR when Dexi stops getting colours.

HyperHDR streams the LG's edge colours to Dexi over UDP. Sometimes its macOS
capture goes stale (after Wi-Fi drops, sleep, or the monitor waking up). Dexi's
/light counts the packets it receives, so this watchdog checks every 10 s:

- HyperHDR not running          -> launch it
- Dexi's packet count not rising for 20 s while HyperHDR runs -> restart HyperHDR
- Dexi unreachable               -> do nothing (Dexi or Wi-Fi is down, not HyperHDR)

Restarts back off (1, 2, 4 ... 10 min) so a monitor that is off doesn't cause a loop.

    python3 tools/lights_watchdog.py            # run in the foreground
    make lights-watchdog                        # install as a login service (launchd)
"""

from __future__ import annotations

import json
import socket
import subprocess
import time
import urllib.request

DEXI_HOST = "dexi.local"
APP = "hyperhdr"
CHECK_S = 10
STALE_S = 20
MIN_BACKOFF_S, MAX_BACKOFF_S = 60, 600


def log(msg: str) -> None:
    print(time.strftime("%H:%M:%S"), msg, flush=True)


def resolve(cache: dict) -> str | None:
    # macOS takes seconds per .local lookup, so keep the last good address.
    try:
        cache["ip"] = socket.gethostbyname(DEXI_HOST)
    except OSError:
        pass
    return cache.get("ip")


def packets(ip: str) -> int | None:
    try:
        with urllib.request.urlopen(f"http://{ip}/light", timeout=3) as r:
            return json.load(r)["packets"]
    except (OSError, ValueError, KeyError):
        return None


def running() -> bool:
    return subprocess.run(["pgrep", "-f", f"{APP}.app"], capture_output=True).returncode == 0


def start() -> None:
    subprocess.run(["open", "-a", APP])


def restart() -> None:
    subprocess.run(["pkill", "-f", f"{APP}.app"])
    time.sleep(3)
    start()


def main() -> None:
    cache: dict = {}
    last_count, last_change = None, time.time()
    backoff, last_restart = MIN_BACKOFF_S, 0.0
    log("lights watchdog started")
    while True:
        time.sleep(CHECK_S)
        now = time.time()
        if not running():
            log("HyperHDR not running: launching")
            start()
            last_change = now
            continue
        ip = resolve(cache)
        count = packets(ip) if ip else None
        if count is None:
            continue  # Dexi is offline or rebooting; not HyperHDR's fault
        if count != last_count:
            if last_count is not None and count < last_count:
                log("Dexi rebooted")
            last_count, last_change = count, now
            if now - last_restart > MAX_BACKOFF_S:
                backoff = MIN_BACKOFF_S  # healthy for a while: reset the backoff
            continue
        if now - last_change >= STALE_S and now - last_restart >= backoff:
            log(f"no packets for {now - last_change:.0f} s: restarting HyperHDR")
            restart()
            last_restart, last_change = now, now
            backoff = min(backoff * 2, MAX_BACKOFF_S)


if __name__ == "__main__":
    main()
