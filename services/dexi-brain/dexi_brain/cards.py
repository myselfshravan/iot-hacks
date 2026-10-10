"""Info cards rendered on the Mac and pushed full-screen to Dexi (128x160)."""

import datetime as dt
import io
import subprocess

import requests
from PIL import Image, ImageDraw, ImageFont

W, H = 128, 160
BG, FG, DIM, ACC = (14, 17, 22), (238, 241, 246), (138, 147, 163), (255, 160, 70)


def font(size: int) -> ImageFont.ImageFont:
    return ImageFont.load_default(size=size)


def _centered(d: ImageDraw.ImageDraw, y: int, text: str, size: int, fill) -> None:
    f = font(size)
    w = d.textlength(text, font=f)
    d.text(((W - w) / 2, y), text, font=f, fill=fill)


def _sky_icon(d: ImageDraw.ImageDraw, sky: str, cx: int, cy: int) -> None:
    night = not 6 <= dt.datetime.now().hour < 19
    if "clear" in sky:
        if night:  # crescent moon
            d.ellipse([cx - 16, cy - 16, cx + 16, cy + 16], fill=(235, 235, 210))
            d.ellipse([cx - 6, cy - 22, cx + 22, cy + 8], fill=BG)
        else:
            d.ellipse([cx - 16, cy - 16, cx + 16, cy + 16], fill=(255, 210, 60))
        return
    if "cloud" in sky or "overcast" in sky or "fog" in sky:
        d.ellipse([cx - 4, cy - 18, cx + 18, cy + 4], fill=(255, 210, 60)) if "partly" in sky else None
    d.ellipse([cx - 22, cy - 6, cx + 2, cy + 14], fill=(200, 210, 225))
    d.ellipse([cx - 10, cy - 14, cx + 16, cy + 12], fill=(220, 228, 240))
    d.rectangle([cx - 12, cy + 2, cx + 14, cy + 14], fill=(220, 228, 240))
    if "rain" in sky or "drizzle" in sky or "shower" in sky or "thunder" in sky:
        for x in (-12, -2, 8):
            d.line([cx + x, cy + 18, cx + x - 3, cy + 26], fill=(90, 160, 255), width=2)


def weather_card(weather: dict | None) -> Image.Image:
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    now = dt.datetime.now()
    _centered(d, 6, now.strftime("%H:%M"), 30, FG)
    _centered(d, 40, now.strftime("%a %d %b"), 12, DIM)
    if weather:
        _sky_icon(d, weather["sky"], W // 2, 82)
        _centered(d, 112, f"{weather['temp']:.0f}°C", 22, ACC)
        _centered(d, 140, f"{weather['sky']} · {weather['humidity']}%", 10, DIM)
    else:
        _centered(d, 90, "no weather", 12, DIM)
    return img


def now_playing() -> dict | None:
    """Current track from Spotify or Apple Music, if one is playing."""
    for app in ("Spotify", "Music"):
        script = (f'if application "{app}" is running then\n'
                  f'tell application "{app}" to if player state is playing then '
                  f'return (name of current track) & "\\n" & (artist of current track)'
                  + (' & "\\n" & (artwork url of current track)' if app == "Spotify" else "")
                  + "\nend if")
        try:
            out = subprocess.run(["osascript", "-e", script], capture_output=True, text=True,
                                 timeout=3).stdout.strip()
        except Exception:
            continue
        if out:
            parts = out.split("\n")
            return {"track": parts[0], "artist": parts[1] if len(parts) > 1 else "",
                    "art": parts[2] if len(parts) > 2 else None}
    return None


def music_card(song: dict) -> Image.Image:
    img = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(img)
    art = None
    if song.get("art"):
        try:
            art = Image.open(io.BytesIO(requests.get(song["art"], timeout=5).content)).convert("RGB")
        except Exception:
            art = None
    if art:
        img.paste(art.resize((104, 104)), (12, 6))
    else:
        d.rounded_rectangle([12, 6, 116, 110], 10, fill=(40, 46, 58))
        _centered(d, 40, "♪", 40, ACC)
    _centered(d, 116, song["track"][:20], 12, FG)
    _centered(d, 134, song["artist"][:22], 10, DIM)
    return img
