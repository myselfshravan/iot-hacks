"""Pet packs: animated pets for Dexi built from GIFs.

Downloads a pet's GIFs (default source: clawd-on-desk's assets/gif; the art
belongs to its owners, so it is cached locally and never committed), fits
them to Dexi's screen, and writes one compact .pak that the firmware plays
per mood (firmware/dexi/src/pet/pet_pack.*).

    uv run dexi-pets build clawd      # download + convert -> .petcache/clawd.pak
    uv run dexi-pets upload clawd     # send to Dexi and switch to it
    uv run dexi-pets list             # what's on Dexi

.pak layout (little-endian):
  "DXP1" u8 anim_count u16 box_w u16 box_h u8 scale u8 mood_map[12]
  (frames are stored box_w x box_h and drawn `scale` times bigger)
  anim_count x { char name[16] u8 frames u16 frame_ms u32 table_offset }
  per anim: frames x u32 frame_offset, then RLE frames.
RLE tokens: 0x00-0x7F: n+1 transparent px; 0x80-0xBF: (n&0x3F)+1 literal px
follow as u16 RGB565; 0xC0-0xFF: (n&0x3F)+1 px of the one u16 colour that follows.
"""

import argparse
import io
import struct
import sys
from pathlib import Path

import requests
from PIL import Image, ImageSequence

from . import config
from .dexi import Dexi

SOURCE = "https://raw.githubusercontent.com/rullerzhou-afk/clawd-on-desk/main/assets/gif/{pet}-{anim}.gif"
CACHE = Path(__file__).resolve().parent.parent / ".petcache"
BOX_W, BOX_H = 120, 96  # max on-screen sprite size (sky above, grass below)
MAX_FPS = 8
MAX_FRAMES = 32         # per animation
BUDGET = 1_300_000      # Dexi's LittleFS holds one pack of about this size

# Mood slots, in the firmware's Mood enum order, then "thinking".
SLOTS = ["neutral", "happy", "excited", "love", "sleepy", "sad", "angry",
         "surprised", "hot", "cold", "curious", "thinking"]

# Which source animation plays for each slot; first available wins.
PETS = {
    "clawd": {"pixel": True, "anims": {
        "neutral": ["idle"], "happy": ["happy"], "excited": ["react-double-jump", "happy"],
        "love": ["headphones-groove", "happy"], "sleepy": ["sleeping"], "sad": ["error"],
        "angry": ["react-annoyed"], "surprised": ["notification"], "hot": ["sweeping"],
        "cold": ["error"], "curious": ["idle-reading", "debugger"], "thinking": ["thinking"]}},
    "calico": {"pixel": False, "box": (110, 76), "anims": {
        "neutral": ["idle"], "happy": ["happy"], "excited": ["juggling", "happy"],
        "love": ["happy"], "sleepy": ["sleeping"], "sad": ["error"], "angry": ["error"],
        "surprised": ["notification"], "hot": ["sweeping"], "cold": ["error"],
        "curious": ["thinking"], "thinking": ["thinking"]}},
    "cloudling": {"pixel": False, "box": (100, 90), "anims": {
        "neutral": ["idle"], "happy": ["juggling"], "excited": ["juggling"],
        "love": ["idle"], "sleepy": ["sleeping"], "sad": ["error"], "angry": ["error"],
        "surprised": ["attention", "notification"], "hot": ["sweeping"], "cold": ["error"],
        "curious": ["idle-reading"], "thinking": ["thinking"]}},
}


def fetch(pet: str, anim: str) -> Image.Image | None:
    path = CACHE / pet / f"{anim}.gif"
    if not path.exists():
        r = requests.get(SOURCE.format(pet=pet, anim=anim), timeout=30)
        if r.status_code != 200:
            return None
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(r.content)
    return Image.open(path)


def cut_background(img: Image.Image) -> Image.Image:
    """Some GIFs have an opaque backdrop: flood-fill it to transparent from the corners."""
    from PIL import ImageDraw
    if img.getpixel((0, 0))[3] < 128:
        return img  # already transparent
    img = img.copy()
    w, h = img.size
    for corner in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)):
        if img.getpixel(corner)[3] >= 128:
            ImageDraw.floodfill(img, corner, (0, 0, 0, 0), thresh=40)
    return img


def frames_of(gif: Image.Image) -> tuple[list[Image.Image], int]:
    """RGBA frames resampled to <= MAX_FPS, and the per-frame duration."""
    raw = [(cut_background(f.convert("RGBA")), f.info.get("duration", 100) or 100)
           for f in ImageSequence.Iterator(gif)]
    total = sum(d for _, d in raw)
    step_ms = max(1000 // MAX_FPS, total // max(1, len(raw)))
    out, t, acc, i = [], 0, 0, 0
    if total / step_ms > MAX_FRAMES:
        step_ms = total // MAX_FRAMES
    while t < total:
        while i < len(raw) - 1 and acc + raw[i][1] <= t:
            acc += raw[i][1]
            i += 1
        out.append(raw[i][0])
        t += step_ms
    return out, step_ms


def rle(img: Image.Image) -> bytes:
    """Token stream (see module doc): transparent runs, literal runs, repeat runs."""
    px = img.load()
    w, h = img.size
    vals = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            vals.append(None if a < 128 else ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))
    out = bytearray()
    i, n = 0, len(vals)
    literal: list[int] = []

    def flush_literal():
        while literal:
            chunk = literal[:64]
            del literal[:64]
            out.append(0x80 | (len(chunk) - 1))
            for v in chunk:
                out.extend(struct.pack("<H", v))

    while i < n:
        v = vals[i]
        j = i
        while j < n and vals[j] == v:
            j += 1
        run = j - i
        if v is None:
            flush_literal()
            while run:
                k = min(run, 128)
                out.append(k - 1)
                run -= k
        elif run >= 3:
            flush_literal()
            while run:
                k = min(run, 64)
                out.append(0xC0 | (k - 1))
                out.extend(struct.pack("<H", v))
                run -= k
        else:
            literal.extend([v] * run)
        i = j
    flush_literal()
    return bytes(out)


def build(pet: str) -> Path:
    spec = PETS[pet]
    chosen: dict[str, tuple[list[Image.Image], int]] = {}
    slot_anim: dict[str, str] = {}
    for slot in SLOTS:
        for name in spec["anims"][slot]:
            if name not in chosen:
                gif = fetch(pet, name)
                if gif is None:
                    continue
                chosen[name] = frames_of(gif)
            slot_anim[slot] = name
            break
    names = list(chosen)

    # One crop box for all animations keeps the pet anchored in place.
    box = None
    for frames, _ in chosen.values():
        for f in frames:
            bb = f.getbbox()
            if bb:
                box = bb if box is None else (min(box[0], bb[0]), min(box[1], bb[1]),
                                              max(box[2], bb[2]), max(box[3], bb[3]))
    bw, bh = box[2] - box[0], box[3] - box[1]
    max_w, max_h = spec.get("box", (BOX_W, BOX_H))
    # Frames are stored at half the on-screen size and drawn 2x: a quarter of
    # the bytes, and it reads as chunky pixel art on a 1.8" screen anyway.
    draw_scale = spec.get("scale", 2)
    fit = min(max_w / bw, max_h / bh) / draw_scale
    size = (max(1, round(bw * fit)), max(1, round(bh * fit)))
    resample = Image.NEAREST if spec["pixel"] else Image.LANCZOS

    header = bytearray(b"DXP1")
    header += struct.pack("<BHHB", len(names), *size, draw_scale)
    header += bytes(names.index(slot_anim.get(s, names[0])) for s in SLOTS)
    table_start = len(header) + len(names) * 23
    body = bytearray()
    entries = []
    for name in names:
        frames, ms = chosen[name]
        encoded = [rle(f.crop(box).resize(size, resample)) for f in frames]
        table_off = table_start + len(body)
        data_off = table_off + 4 * len(encoded)
        offs, cursor = [], data_off
        for e in encoded:
            offs.append(cursor)
            cursor += len(e)
        body += b"".join(struct.pack("<I", o) for o in offs) + b"".join(encoded)
        entries.append(struct.pack("<16sBHI", name.encode()[:16], len(encoded), ms, table_off))
    pak = bytes(header) + b"".join(entries) + bytes(body)
    out = CACHE / f"{pet}.pak"
    out.write_bytes(pak)
    print(f"{pet}: {len(names)} animations, {size[0]}x{size[1]} x{draw_scale}, "
          f"{len(pak) / 1024:.0f} KB -> {out}")
    if len(pak) > BUDGET:
        print(f"  too big for Dexi ({BUDGET / 1024:.0f} KB): shrink its box or MAX_FRAMES")
    return out


def main() -> None:
    ap = argparse.ArgumentParser(prog="dexi-pets", description=__doc__.split("\n\n")[0])
    ap.add_argument("command", choices=["build", "upload", "list", "use"])
    ap.add_argument("pet", nargs="?", help=f"one of {', '.join(PETS)} (or 'builtin' for use)")
    args = ap.parse_args()
    if args.command == "build":
        build(args.pet)
        return
    dexi = Dexi(config.load().dexi_url)
    if args.command == "list":
        print(dexi.pets())
    elif args.command == "use":
        print(dexi.use_pet(args.pet))
    else:
        pak = CACHE / f"{args.pet}.pak"
        if not pak.exists():
            build(args.pet)
        print(dexi.upload_pet(args.pet, pak.read_bytes()))
        print(dexi.use_pet(args.pet))


if __name__ == "__main__":
    sys.exit(main())
