"""HTTP client for the Dexi firmware (firmware/dexi/src/net/web_api.h)."""

import io
import socket
from urllib.parse import urlsplit, urlunsplit

import requests

WIDTH, HEIGHT = 128, 160


class Dexi:
    def __init__(self, base_url: str, timeout: float = 4.0):
        self.url = base_url
        self.timeout = timeout
        self.base = resolve(base_url)

    def _get(self, path: str, **params) -> dict:
        try:
            r = requests.get(self.base + path, params=params, timeout=self.timeout)
        except requests.ConnectionError:
            self.base = resolve(self.url)  # Dexi may have a new DHCP address
            raise
        r.raise_for_status()
        return r.json()

    def state(self) -> dict:
        return self._get("/state")

    def mood(self, mood: str, secs: int = 0) -> dict:
        return self._get("/mood", m=mood, secs=secs)

    def say(self, text: str, secs: int = 8) -> dict:
        return self._get("/say", text=text, secs=secs)

    def look(self, x: float, y: float, secs: int = 3) -> dict:
        return self._get("/look", x=f"{x:.2f}", y=f"{y:.2f}", secs=secs)

    def think(self, secs: int = 4) -> dict:
        return self._get("/think", secs=secs)

    def pets(self) -> dict:
        return self._get("/pets")

    def use_pet(self, name: str) -> dict:
        return self._get("/pets/use", name=name)

    def upload_pet(self, name: str, pak: bytes) -> dict:
        r = requests.post(self.base + "/pets/upload", params={"name": name},
                          files={"pak": (f"{name}.pak", pak)}, timeout=120)
        r.raise_for_status()
        return r.json()

    def show_image(self, image, secs: int = 10) -> dict:
        """Pushes a PIL image (any size; letterboxed to 128x160) to the screen."""
        from PIL import Image

        canvas = Image.new("RGB", (WIDTH, HEIGHT))
        img = image.convert("RGB")
        img.thumbnail((WIDTH, HEIGHT))
        canvas.paste(img, ((WIDTH - img.width) // 2, (HEIGHT - img.height) // 2))
        raw = io.BytesIO()
        for r, g, b in canvas.getdata():
            v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
            raw.write(v.to_bytes(2, "little"))
        r = requests.post(self.base + "/frame", params={"secs": secs},
                          files={"img": ("frame.raw", raw.getvalue())}, timeout=10)
        r.raise_for_status()
        return r.json()


def resolve(url: str) -> str:
    """Swaps the hostname for its IPv4 address. macOS takes ~5 s per request to
    resolve .local names (it tries IPv6 first), longer than our timeouts."""
    parts = urlsplit(url)
    try:
        ip = socket.getaddrinfo(parts.hostname, None, socket.AF_INET)[0][4][0]
    except OSError:
        return url
    netloc = ip + (f":{parts.port}" if parts.port else "")
    return urlunsplit(parts._replace(netloc=netloc))
