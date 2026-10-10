// GENERATED from projector.html by tools/embed_web.py. Do not edit.
#pragma once

#include <pgmspace.h>

const char kProjectorHtml[] PROGMEM = R"WEB(<!doctype html>
<!--
  Projector remote (side extra), served at "/projector".
  Edit this file, then `make build` regenerates projector_html.h.
  Sends buttons via GET /api/projector?b=<id> (src/net/web_api.cpp).
-->
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Projector</title>
  <style>
    :root {
      --bg: #111;
      --fg: #eee;
      --key: #2a2a2a;
      --red: #c0392b;
      --acc: #3d7eff;
    }

    body {
      margin: 0;
      padding: 24px 16px;
      background: var(--bg);
      color: var(--fg);
      font: 16px system-ui;
      display: flex;
      flex-direction: column;
      align-items: center;
    }

    h1 {
      font-size: 20px;
      margin: 0 0 20px;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(3, 84px);
      gap: 12px;
    }

    button {
      height: 64px;
      border: 0;
      border-radius: 14px;
      background: var(--key);
      color: var(--fg);
      font-size: 16px;
      touch-action: manipulation;
    }

    button:active {
      background: var(--acc);
    }

    .gap {
      visibility: hidden;
    }

    .red {
      background: var(--red);
    }

    .blue {
      background: var(--acc);
    }

    #status {
      margin-top: 24px;
      font-size: 13px;
      opacity: .7;
    }
  </style>
</head>

<body>
  <h1>Projector</h1>
  <div class="grid">
    <button class="red" data-b="power">Power</button><button class="gap"></button><button data-b="settings">Settings</button>
    <button class="gap"></button><button data-b="up">&#9650;</button><button class="gap"></button>
    <button data-b="left">&#9664;</button><button class="blue" data-b="ok">OK</button><button data-b="right">&#9654;</button>
    <button data-b="back">Back</button><button data-b="down">&#9660;</button><button data-b="home">Home</button>
    <button data-b="voldown">Vol &minus;</button><button data-b="s">S</button><button data-b="volup">Vol +</button>
  </div>
  <div id="status">ready</div>

  <script>
    const status = document.getElementById('status');
    document.querySelectorAll('[data-b]').forEach((b) => {
      b.onclick = async () => {
        const res = await fetch('/api/projector?b=' + b.dataset.b).catch(() => null);
        status.textContent = res && res.ok ? 'sent ' + b.dataset.b : 'failed';
        if (navigator.vibrate) navigator.vibrate(15);
      };
    });
  </script>
</body>

</html>
)WEB";
