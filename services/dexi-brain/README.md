# dexi-brain

The laptop side of Dexi. Every few minutes, or immediately when Dexi is petted, it does three things:
1. Gathers context: the time, the weather outside, whether you're at the Mac, and which app is in front.
2. Asks an LLM what Dexi feels and says.
3. Drives the board over HTTP (`firmware/dexi`).

Any OpenAI-compatible provider works, including OpenAI, Anthropic, xAI Grok and local Ollama. You switch provider by changing `LLM_BASE_URL`, `LLM_MODEL` and `LLM_API_KEY` in `.env`.

```sh
cp .env.example .env      # then paste your API key into .env
uv run dexi-brain
```

**Console:** http://localhost:8765 is a terminal-style dashboard with:
- live status and what Dexi sees;
- a filterable thought log, where ▶ replays any past line on Dexi;
- memory notes you can view, add or delete;
- per-day stats;
- mood and say controls, plus "think now".

Set `CONSOLE_HOST=0.0.0.0` to open it from your phone.

**Touch gestures:**
- The firmware records each gesture in full: every press duration and every gap between presses.
- It reacts instantly by the gesture's shape: a boop, quick taps, drumming (it gets annoyed), or a long hold (purrs).
- The brain describes the gesture to the LLM, which responds in context.
- Two quick taps also show an info card: now playing with album art if Spotify or Music is playing, otherwise the time and weather.

**Memory:** `memory.json` (git-ignored) holds:
- which days you were around, and when;
- how often Dexi was petted;
- recent lines, so it doesn't repeat itself;
- notes the LLM chooses to remember.

The LLM sees the last week of this, so it can make callbacks.

**Pet packs:** animated pets built from clawd-on-desk's GIFs (Clawd, Calico, Cloudling).
- `uv run dexi-pets upload clawd` downloads the GIFs, converts them and uploads to Dexi, all in one step. A pack is about 140 to 320 KB, and all three fit at once.
- Switch pets from the console.
- The art belongs to its owners: it is cached in `.petcache/` (git-ignored) and never committed.

**Laptop compute:** `Dexi.show_image(pil_image, secs)` renders anything on the Mac and pushes it full-screen. That covers weather art, album covers, charts and generated pixel art. The face comes back after `secs` seconds.

**Privacy:** with `SHARE_ACTIVE_APP=1`, the name of the frontmost app (not its contents) is sent to the LLM provider. Set it to 0 to turn this off.
