"""What Dexi knows about the world when it thinks: time, weather, the Mac."""

import datetime as dt
import re
import subprocess

import requests

WEATHER_CODES = {
    0: "clear", 1: "mostly clear", 2: "partly cloudy", 3: "overcast", 45: "foggy",
    48: "foggy", 51: "drizzle", 53: "drizzle", 55: "drizzle", 61: "light rain",
    63: "rain", 65: "heavy rain", 80: "showers", 81: "showers", 82: "violent showers",
    95: "thunderstorm", 96: "thunderstorm with hail", 99: "thunderstorm with hail",
}


def time_of_day() -> str:
    now = dt.datetime.now()
    return now.strftime("%A %d %B, %H:%M")


def weather(lat: float, lon: float) -> str | None:
    try:
        r = requests.get(
            "https://api.open-meteo.com/v1/forecast",
            params={"latitude": lat, "longitude": lon,
                    "current": "temperature_2m,relative_humidity_2m,weather_code"},
            timeout=5,
        )
        c = r.json()["current"]
        sky = WEATHER_CODES.get(c["weather_code"], "unknown sky")
        return f"{c['temperature_2m']:.0f}°C, {c['relative_humidity_2m']}% humidity, {sky}"
    except Exception:
        return None


def mac_idle_seconds() -> int | None:
    """Seconds since the last keyboard/mouse input on this Mac."""
    try:
        out = subprocess.run(["ioreg", "-c", "IOHIDSystem"], capture_output=True,
                             text=True, timeout=3).stdout
        m = re.search(r'"HIDIdleTime" = (\d+)', out)
        return int(m.group(1)) // 1_000_000_000 if m else None
    except Exception:
        return None


def active_app() -> str | None:
    try:
        out = subprocess.run(
            ["osascript", "-e", 'tell application "System Events" to get name of '
             "first application process whose frontmost is true"],
            capture_output=True, text=True, timeout=3)
        return out.stdout.strip() or None
    except Exception:
        return None


def gather(lat: float, lon: float, share_app: bool) -> dict:
    ctx = {"time": time_of_day()}
    if w := weather(lat, lon):
        ctx["weather_outside"] = w
    idle = mac_idle_seconds()
    if idle is not None:
        ctx["owner_away_for"] = f"{idle // 60} min" if idle >= 120 else "at the laptop now"
    if share_app and (app := active_app()):
        ctx["owner_is_using"] = app
    return ctx
