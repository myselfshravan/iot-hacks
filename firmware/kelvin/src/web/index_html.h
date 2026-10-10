// GENERATED from index.html by tools/embed_web.py. Do not edit.
#pragma once

#include <pgmspace.h>

const char kIndexHtml[] PROGMEM = R"WEB(<!doctype html>
<!--
  Kelvin phone UI, served at "/".
  Edit this file, then `make build` regenerates index_html.h (tools/embed_web.py).
  Talks to the JSON API in src/net/web_api.cpp.
-->
<html>

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>Kelvin</title>
  <style>
    :root {
      --bg: #0e1116;
      --card: #1a1f27;
      --key: #2a313c;
      --fg: #eef;
      --dim: #8a93a3;
      --acc: #3d9bff;
      --on: #2ecc71;
      --off: #555;
      --bad: #e74c3c;
      --warn: #f1c40f;
    }

    body {
      margin: 0;
      padding: 16px;
      background: var(--bg);
      color: var(--fg);
      font: 16px system-ui;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 14px;
    }

    .card,
    .tabs,
    #offline {
      width: 100%;
      max-width: 380px;
      box-sizing: border-box;
    }

    .card {
      background: var(--card);
      border-radius: 20px;
      padding: 18px;
    }

    button {
      border: 0;
      color: var(--fg);
      background: var(--key);
    }

    #offline {
      display: none;
      background: var(--bad);
      color: #fff;
      border-radius: 12px;
      padding: 10px 14px;
      text-align: center;
    }

    .tabs {
      display: flex;
      gap: 6px;
    }

    .tabs button {
      flex: 1;
      height: 40px;
      border-radius: 12px;
    }

    .tabs button.sel {
      background: var(--acc);
    }

    /* AC */
    #power {
      width: 100%;
      height: 64px;
      border-radius: 16px;
      font-size: 20px;
      color: #fff;
      background: var(--off);
    }

    #power.on {
      background: var(--on);
    }

    .temp {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    .temp b {
      font-size: 56px;
      font-weight: 600;
    }

    .temp button {
      width: 64px;
      height: 64px;
      border-radius: 50%;
      font-size: 28px;
    }

    .label {
      color: var(--dim);
      font-size: 13px;
      margin: 6px 0 8px;
    }

    .seg {
      display: flex;
      gap: 6px;
      flex-wrap: wrap;
    }

    .seg button {
      flex: 1;
      min-width: 56px;
      height: 44px;
      border-radius: 12px;
    }

    .seg button.sel {
      background: var(--acc);
    }

    .badges {
      display: flex;
      gap: 8px;
      flex-wrap: wrap;
    }

    .badge {
      font-size: 13px;
      padding: 5px 10px;
      border-radius: 999px;
      background: var(--key);
      color: var(--dim);
    }

    .badge.ok {
      background: var(--on);
      color: #04210f;
    }

    .badge.bad {
      background: var(--bad);
      color: #fff;
    }

    .badge.wait {
      background: var(--warn);
      color: #2a2000;
    }

    .air {
      display: flex;
      justify-content: space-around;
      font-size: 22px;
    }

    .air small {
      display: block;
      font-size: 12px;
      color: var(--dim);
      text-align: center;
    }

    /* Projector */
    .grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 10px;
    }

    .grid button {
      height: 56px;
      border-radius: 14px;
      font-size: 15px;
    }

    .grid button:active {
      background: var(--acc);
    }

    .grid .gap {
      visibility: hidden;
    }

    .grid .red {
      background: var(--bad);
    }

    .grid .blue {
      background: var(--acc);
    }

    #status {
      color: var(--dim);
      font-size: 12px;
    }
  </style>
</head>

<body>
  <div id="offline">Kelvin offline: not responding</div>

  <div class="tabs">
    <button id="tabAc" class="sel">AC</button>
    <button id="tabProjector">Projector</button>
  </div>

  <div id="ac" style="display: contents">
    <div class="card"><button id="power">Power</button></div>
    <div class="card badges">
      <span class="badge" id="echo">IR</span>
      <span class="badge" id="confirm">AC reaction</span>
    </div>
    <div class="card temp">
      <button data-dt="-1">&minus;</button><b id="temp">--</b><button data-dt="1">+</button>
    </div>
    <div class="card">
      <div class="label">Mode</div>
      <div class="seg" id="mode"></div>
      <div class="label">Fan</div>
      <div class="seg" id="fan"></div>
      <div class="label">Swing</div>
      <div class="seg" id="swing"></div>
    </div>
    <div class="card air">
      <div><span id="airC">--</span><small>air °C</small></div>
      <div><span id="airRh">--</span><small>humidity %</small></div>
    </div>
  </div>

  <div id="projector" class="card" style="display: none">
    <div class="grid">
      <button class="red" data-b="power">Power</button><button class="gap"></button><button data-b="settings">Settings</button>
      <button class="gap"></button><button data-b="up">&#9650;</button><button class="gap"></button>
      <button data-b="left">&#9664;</button><button class="blue" data-b="ok">OK</button><button data-b="right">&#9654;</button>
      <button data-b="back">Back</button><button data-b="down">&#9660;</button><button data-b="home">Home</button>
      <button data-b="voldown">Vol &minus;</button><button data-b="s">S</button><button data-b="volup">Vol +</button>
    </div>
  </div>

  <div id="status">loading…</div>

  <script>
    const $ = (id) => document.getElementById(id);
    const MODES = ['cool', 'dry', 'fan', 'heat', 'auto'];
    const FANS = ['auto', 'low', 'medium', 'high'];
    const CONFIRM = {
      pending: ['wait', 'AC reacting…'],
      confirmed: ['ok', 'AC reacted ✓'],
      unconfirmed: ['bad', 'No AC reaction ✗'],
      none: ['', 'AC reaction: n/a'],
    };

    let state = {};
    let misses = 0;

    // ---- networking ----

    async function get(url, timeoutMs = 3000) {
      const ctl = new AbortController();
      setTimeout(() => ctl.abort(), timeoutMs);
      const res = await fetch(url, {
        signal: ctl.signal
      }).catch(() => null);
      misses = res && res.ok ? 0 : misses + 1;
      $('offline').style.display = misses >= 2 ? 'block' : 'none';
      return res && res.ok ? res : null;
    }

    async function refresh() {
      const res = await get('/state', 1500);
      if (res) render(await res.json());
    }

    async function setAc(fields) {
      const q = new URLSearchParams(
        Object.entries(fields).map(([k, v]) => [k, v === true ? 1 : v === false ? 0 : v]));
      $('status').textContent = 'sending…';
      const res = await get('/set?' + q); // waits for the IR echo, so allow 3 s
      if (res) render(await res.json());
      else $('status').textContent = 'failed';
    }

    async function pressProjector(button) {
      const res = await get('/projector?b=' + button);
      $('status').textContent = res ? 'projector: ' + button : 'failed';
      if (navigator.vibrate) navigator.vibrate(15);
    }

    // ---- rendering ----

    function segment(id, options, key) {
      const el = $(id);
      el.innerHTML = '';
      for (let o of options) {
        if (typeof o !== 'object') o = {
          v: o,
          label: o
        };
        const b = document.createElement('button');
        b.textContent = o.label;
        if (String(state[key]) === String(o.v)) b.className = 'sel';
        b.onclick = () => setAc({
          [key]: o.v
        });
        el.appendChild(b);
      }
    }

    function badge(id, cls, text) {
      $(id).className = 'badge ' + cls;
      $(id).textContent = text;
    }

    function render(s) {
      state = s;
      $('power').className = s.power ? 'on' : '';
      $('power').textContent = s.power ? 'ON' : 'OFF';
      $('temp').textContent = s.temp + '°';
      segment('mode', MODES, 'mode');
      segment('fan', FANS, 'fan');
      segment('swing', [{
        v: true,
        label: 'on'
      }, {
        v: false,
        label: 'off'
      }], 'swing');

      if (s.source === 'remote') badge('echo', 'ok', 'IR: from remote');
      else if (s.echo_ok === true) badge('echo', 'ok', 'IR sent ✓');
      else if (s.echo_ok === false) badge('echo', 'bad', 'IR not seen ✗');
      else badge('echo', '', 'IR');
      badge('confirm', ...(CONFIRM[s.confirm] || CONFIRM.none));

      $('airC').textContent = s.air_c == null ? '--' : s.air_c.toFixed(1);
      $('airRh').textContent = s.air_rh == null ? '--' : Math.round(s.air_rh);
      $('status').textContent = 'last change: ' + s.source + ' · Wi-Fi ' + s.rssi + ' dBm';
    }

    function showTab(ac) {
      $('ac').style.display = ac ? 'contents' : 'none';
      $('projector').style.display = ac ? 'none' : 'block';
      $('tabAc').className = ac ? 'sel' : '';
      $('tabProjector').className = ac ? '' : 'sel';
    }

    // ---- wiring ----

    $('power').onclick = () => setAc({
      power: !state.power
    });
    document.querySelectorAll('[data-dt]').forEach((b) => {
      b.onclick = () => setAc({
        temp: Math.min(30, Math.max(16, state.temp + Number(b.dataset.dt)))
      });
    });
    document.querySelectorAll('[data-b]').forEach((b) => {
      b.onclick = () => pressProjector(b.dataset.b);
    });
    $('tabAc').onclick = () => showTab(true);
    $('tabProjector').onclick = () => showTab(false);

    refresh();
    setInterval(refresh, 2000);
  </script>
</body>

</html>)WEB";
