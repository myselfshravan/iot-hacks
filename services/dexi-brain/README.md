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

**Laptop compute:** `Dexi.show_image(pil_image, secs)` renders anything on the Mac and pushes it full-screen. That covers weather art, album covers, charts and generated pixel art. The face comes back after `secs` seconds.

**Privacy:** with `SHARE_ACTIVE_APP=1`, the name of the frontmost app (not its contents) is sent to the LLM provider. Set it to 0 to turn this off.
