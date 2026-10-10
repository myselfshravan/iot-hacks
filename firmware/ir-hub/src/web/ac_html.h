// GENERATED from ac.html by tools/embed_web.py. Do not edit.
#pragma once

#include <pgmspace.h>

const char kAcHtml[] PROGMEM = R"WEB(<!doctype html>
<!--
  Kelvin, the AC remote page, served at "/".
  Edit this file, then `make build` regenerates ac_html.h (tools/embed_web.py).
  Talks to the JSON API in src/net/web_api.cpp. The projector has its own page (projector.html).
  Designed to fit one phone screen without scrolling (min ~360x640).
  No swing control: the indoor multi-split FCU has no louvre, so swing stays as-is.
-->
<html lang="en">

<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
  <meta name="theme-color" content="#0e1116">
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <meta name="apple-mobile-web-app-title" content="Kelvin">
  <title>Kelvin</title>
  <style>
    :root {
      --bg: #0e1116;
      --card: #171c24;
      --key: #242b36;
      --fg: #eef1f6;
      --dim: #8a93a3;
      --acc: #3d9bff;
      --on: #2ecc71;
      --bad: #e74c3c;
      --warn: #f1c40f;
      --r: 16px;
      --gap: clamp(8px, 1.6dvh, 14px);
    }

    * {
      box-sizing: border-box;
      -webkit-tap-highlight-color: transparent;
    }

    html,
    body {
      margin: 0;
      height: 100%;
      background: var(--bg);
      color: var(--fg);
      font: 15px/1.2 system-ui, -apple-system, sans-serif;
    }

    body {
      display: flex;
      justify-content: center;
      padding: max(12px, env(safe-area-inset-top)) 14px max(12px, env(safe-area-inset-bottom));
    }

    main {
      width: 100%;
      max-width: 420px;
      height: 100%;
      display: flex;
      flex-direction: column;
      gap: var(--gap);
    }

    button {
      font: inherit;
      color: inherit;
      border: 0;
      border-radius: 12px;
      background: var(--key);
      min-height: 44px;
      cursor: pointer;
      touch-action: manipulation;
      transition: background .15s, transform .05s;
    }

    button:active {
      transform: scale(.96);
    }

    button:disabled {
      opacity: .5;
    }

    button.sel {
      background: var(--acc);
      color: #fff;
    }

    /* Header: name, link state, room readings */
    header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    h1 {
      margin: 0;
      font-size: 22px;
      display: flex;
      align-items: center;
      gap: 8px;
    }

    .dot {
      width: 9px;
      height: 9px;
      border-radius: 50%;
      background: var(--dim);
    }

    .dot.ok {
      background: var(--on);
      box-shadow: 0 0 8px var(--on);
    }

    .dot.bad {
      background: var(--bad);
    }

    .air {
      display: flex;
      gap: 12px;
      color: var(--dim);
      font-size: 14px;
    }

    .air b {
      color: var(--fg);
      font-weight: 600;
    }

    .card {
      background: var(--card);
      border-radius: var(--r);
      padding: var(--gap);
    }

    /* Hero: temperature + power, takes the spare height */
    .hero {
      flex: 1;
      min-height: 150px;
      display: grid;
      grid-template-columns: 1fr auto 1fr;
      align-items: center;
      justify-items: center;
      position: relative;
    }

    .hero .step {
      width: 64px;
      height: 64px;
      border-radius: 50%;
      font-size: 30px;
    }

    .setpoint {
      text-align: center;
    }

    .setpoint b {
      display: block;
      font-size: clamp(56px, 11dvh, 88px);
      font-weight: 650;
      letter-spacing: -2px;
      font-variant-numeric: tabular-nums;
    }

    .setpoint small {
      color: var(--dim);
      font-size: 13px;
    }

    .off .setpoint b {
      color: var(--dim);
    }

    #power {
      width: 100%;
      height: clamp(52px, 8dvh, 64px);
      font-size: 18px;
      font-weight: 600;
    }

    #power.on {
      background: var(--on);
      color: #04210f;
    }

    /* Option rows */
    .row {
      display: flex;
      flex-direction: column;
      gap: 6px;
    }

    .row+.row {
      margin-top: var(--gap);
    }

    .label {
      color: var(--dim);
      font-size: 12px;
      text-transform: uppercase;
      letter-spacing: .06em;
    }

    .seg {
      display: flex;
      gap: 6px;
    }

    .seg button {
      flex: 1;
      min-width: 0;
      padding: 0 4px;
      font-size: 14px;
    }

    .timer {
      display: grid;
      grid-template-columns: 1fr auto;
      gap: 6px;
    }

    .stepper {
      display: flex;
      align-items: center;
      gap: 4px;
    }

    .stepper button {
      width: 40px;
    }

    .stepper span {
      min-width: 34px;
      text-align: center;
      font-variant-numeric: tabular-nums;
    }

    /* Footer: delivery feedback */
    footer {
      display: flex;
      gap: 6px;
      flex-wrap: wrap;
      align-items: center;
    }

    .badge {
      font-size: 12px;
      padding: 5px 10px;
      border-radius: 999px;
      background: var(--key);
      color: var(--dim);
    }

    .badge.ok {
      background: rgba(46, 204, 113, .18);
      color: var(--on);
    }

    .badge.bad {
      background: rgba(231, 76, 60, .18);
      color: var(--bad);
    }

    .badge.wait {
      background: rgba(241, 196, 15, .18);
      color: var(--warn);
    }

    #status {
      margin-left: auto;
      color: var(--dim);
      font-size: 12px;
    }

    #offline {
      display: none;
      background: var(--bad);
      color: #fff;
      border-radius: 12px;
      padding: 8px 12px;
      text-align: center;
      font-size: 14px;
    }
  </style>
</head>

<body>
  <main>
    <header>
      <h1><span class="dot" id="dot"></span>Kelvin</h1>
      <div class="air"><span>🌡 <b id="airC">--</b>°</span><span>💧 <b id="airRh">--</b>%</span></div>
    </header>

    <div id="offline">Kelvin is offline: retrying…</div>

    <section class="card hero" id="hero">
      <button class="step" data-dt="-1" aria-label="Cooler">&minus;</button>
      <div class="setpoint"><b id="temp">--</b><small id="modeLabel">set temperature</small></div>
      <button class="step" data-dt="1" aria-label="Warmer">+</button>
    </section>

    <button id="power">Power</button>

    <section class="card">
      <div class="row">
        <span class="label">Mode</span>
        <div class="seg" id="mode"></div>
      </div>
      <div class="row">
        <span class="label">Fan</span>
        <div class="seg" id="fan"></div>
      </div>
      <div class="row">
        <span class="label">Timer</span>
        <div class="timer">
          <div class="seg" id="timer"></div>
          <div class="stepper">
            <button data-dh="-1" aria-label="Fewer hours">&minus;</button>
            <span id="hours">-</span>
            <button data-dh="1" aria-label="More hours">+</button>
          </div>
        </div>
      </div>
      <div class="row">
        <span class="label">Sleep</span>
        <div class="seg" id="sleep"></div>
      </div>
    </section>

    <footer>
      <span class="badge" id="echo">IR</span>
      <span class="badge" id="confirm">AC reaction</span>
      <span id="status">loading…</span>
    </footer>
  </main>

  <script>
    const $ = (id) => document.getElementById(id);
    const MODES = [
      { v: 'cool', label: '❄️ Cool' }, { v: 'dry', label: '💧 Dry' }, { v: 'fan', label: '🌀 Fan' },
      { v: 'heat', label: '☀️ Heat' }, { v: 'auto', label: 'Auto' },
    ];
    const FANS = [
      { v: 'auto', label: 'Auto' }, { v: 'low', label: 'Low' },
      { v: 'medium', label: 'Med' }, { v: 'high', label: 'High' },
    ];
    const TIMERS = [
      { v: 'none', label: 'None' }, { v: 'off', label: 'Off in' }, { v: 'on', label: 'On in' },
    ];
    const ON_OFF = [{ v: false, label: 'Off' }, { v: true, label: 'On' }];
    const CONFIRM = {
      pending: ['wait', 'AC reacting…'],
      confirmed: ['ok', 'AC reacted ✓'],
      unconfirmed: ['bad', 'No AC reaction'],
      none: ['', 'Reaction: n/a'],
    };

    let state = null;
    let misses = 0;
    let busy = false;

    // ---- networking ----

    async function get(url, timeoutMs) {
      const ctl = new AbortController();
      const timer = setTimeout(() => ctl.abort(), timeoutMs);
      const res = await fetch(url, { signal: ctl.signal }).catch(() => null);
      clearTimeout(timer);
      const ok = !!(res && res.ok);
      misses = ok ? 0 : misses + 1;
      $('offline').style.display = misses >= 2 ? 'block' : 'none';
      $('dot').className = 'dot ' + (misses >= 2 ? 'bad' : ok ? 'ok' : '');
      return ok ? res : null;
    }

    async function refresh() {
      if (busy) return;
      const res = await get('/state', 1500);
      if (res && !busy) render(await res.json());
    }

    // One command at a time; the hub resends until it hears its own echo (~1 s worst case).
    async function setAc(fields) {
      if (busy || !state) return;
      busy = true;
      document.body.style.cursor = 'progress';
      document.querySelectorAll('button').forEach((b) => (b.disabled = true));
      $('status').textContent = 'sending…';
      const q = new URLSearchParams(Object.entries(fields).map(([k, v]) => [k, String(v === true ? 1 : v === false ? 0 : v)]));
      const res = await get('/set?' + q, 4000);
      busy = false;
      document.body.style.cursor = '';
      document.querySelectorAll('button').forEach((b) => (b.disabled = false));
      if (res) render(await res.json());
      else $('status').textContent = 'send failed';
      if (navigator.vibrate) navigator.vibrate(10);
    }

    // ---- rendering ----

    function segment(id, options, key) {
      const el = $(id);
      el.replaceChildren(...options.map((o) => {
        const b = document.createElement('button');
        b.textContent = o.label;
        b.className = String(state[key]) === String(o.v) ? 'sel' : '';
        b.onclick = () => setAc({ [key]: o.v });
        return b;
      }));
    }

    function badge(id, cls, text) {
      $(id).className = 'badge ' + cls;
      $(id).textContent = text;
    }

    function render(s) {
      state = s;
      $('hero').classList.toggle('off', !s.power);
      $('power').className = s.power ? 'on' : '';
      $('power').textContent = s.power ? 'On' : 'Off';
      $('temp').textContent = s.temp + '°';
      $('modeLabel').textContent = s.power ? s.mode + ' · fan ' + s.fan : 'AC is off';
      segment('mode', MODES, 'mode');
      segment('fan', FANS, 'fan');
      segment('timer', TIMERS, 'timer');
      segment('sleep', ON_OFF, 'sleep');
      $('hours').textContent = s.hours + 'h';

      if (s.source === 'remote') badge('echo', 'ok', 'From remote');
      else if (s.echo_ok === true) badge('echo', 'ok', 'IR sent ✓');
      else if (s.echo_ok === false) badge('echo', 'bad', 'IR not heard');
      else badge('echo', '', 'IR');
      badge('confirm', ...(CONFIRM[s.confirm] || CONFIRM.none));

      $('airC').textContent = s.air_c == null ? '--' : s.air_c.toFixed(1);
      $('airRh').textContent = s.air_rh == null ? '--' : Math.round(s.air_rh);
      $('status').textContent = 'via ' + s.source;
    }

    // ---- wiring ----

    $('power').onclick = () => setAc({ power: !state.power });
    document.querySelectorAll('[data-dt]').forEach((b) => {
      b.onclick = () => setAc({ temp: Math.min(30, Math.max(16, state.temp + Number(b.dataset.dt))) });
    });
    document.querySelectorAll('[data-dh]').forEach((b) => {
      b.onclick = () => setAc({ hours: Math.min(8, Math.max(1, state.hours + Number(b.dataset.dh))) });
    });
    document.addEventListener('visibilitychange', () => { if (!document.hidden) refresh(); });

    refresh();
    setInterval(refresh, 2000);
  </script>
</body>

</html>
)WEB";
