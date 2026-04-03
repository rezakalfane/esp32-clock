#pragma once
#include <Arduino.h> 

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Réglage de l'horloge</title>
<style>
  *{box-sizing:border-box;margin:0;padding:0}
  body{font-family:-apple-system,sans-serif;background:#0f0f0f;color:#e0e0e0;min-height:100vh;display:flex;align-items:flex-start;justify-content:center;padding:1.5rem}
  .card{background:#1a1a1a;border:1px solid #2a2a2a;border-radius:12px;padding:2rem;width:100%;max-width:480px}
  h1{font-size:1.1rem;font-weight:600;color:#5DCAA5;margin-bottom:0.2rem}
  .sub{font-size:0.75rem;color:#555;margin-bottom:1.5rem}
  .live{background:#111;border:1px solid #222;border-radius:10px;padding:1.25rem;margin-bottom:1.5rem}
  .live-time{font-size:2.8rem;font-weight:700;color:#5DCAA5;letter-spacing:0.05em;font-variant-numeric:tabular-nums;line-height:1;transition:color 0.4s ease}
  .live-date{font-size:0.8rem;color:#555;margin-top:0.3rem;margin-bottom:1rem}
  .stats{display:grid;grid-template-columns:1fr 1fr 1fr;gap:8px;margin-bottom:1rem}
  .stat{background:#0f0f0f;border-radius:8px;padding:0.75rem}
  .stat-label{font-size:0.65rem;color:#555;text-transform:uppercase;letter-spacing:0.08em;margin-bottom:0.3rem}
  .stat-val{font-size:1rem;font-weight:600;color:#e0e0e0}
  .stat-val.ok{color:#5DCAA5}
  .stat-val.off{color:#555}
  .led-section{margin-top:0.25rem}
  .led-label{font-size:0.65rem;color:#555;text-transform:uppercase;letter-spacing:0.08em;margin-bottom:0.5rem;display:flex;justify-content:space-between;align-items:center}
  .led-pct{font-size:0.85rem;font-weight:600;color:#5DCAA5}
  .led-track{background:#0f0f0f;border-radius:6px;height:8px;overflow:hidden}
  .led-fill{height:100%;border-radius:6px;background:#5DCAA5;transition:width 0.9s ease}
  .mercury-badge{display:inline-block;font-size:0.7rem;font-weight:600;padding:0.2rem 0.55rem;border-radius:20px;margin-left:0.5rem;vertical-align:middle;transition:all 0.4s ease}
  .mercury-on{background:#1a3330;color:#5DCAA5;border:1px solid #2a5548}
  .mercury-off{background:#2a1a0a;color:#FF8C00;border:1px solid #5a3a0a}
  .ntp-badge{display:none;font-size:0.7rem;font-weight:600;padding:0.15rem 0.4rem;border-radius:4px;background:#1a2a3a;color:#5DCAA5;border:1px solid #2a4a6a;animation:blink 0.6s step-start infinite}
  @keyframes blink{50%{opacity:0}}
  hr{border:none;border-top:1px solid #222;margin:1.25rem 0}
  .section-title{font-size:0.65rem;text-transform:uppercase;letter-spacing:0.1em;color:#5DCAA5;margin-bottom:0.75rem}
  label{display:block;font-size:0.85rem;color:#aaa;margin-bottom:0.25rem}
  input[type=text],input[type=password],input[type=number]{width:100%;background:#111;border:1px solid #333;border-radius:6px;color:#e0e0e0;padding:0.5rem 0.75rem;font-size:0.9rem;margin-bottom:0.75rem}
  input:focus{outline:none;border-color:#5DCAA5}
  .row{display:flex;gap:0.75rem}
  .row>div{flex:1}
  .time-row{display:flex;align-items:center;gap:0.5rem;margin-bottom:0.75rem}
  .time-row label{margin:0;min-width:130px;font-size:0.85rem;color:#aaa}
  .time-row input{width:57px;text-align:center;margin:0}
  .sep{color:#5DCAA5;font-weight:bold}
  .days{display:flex;gap:0.4rem;flex-wrap:wrap;margin-bottom:1rem}
  .day-btn input[type=checkbox]{display:none}
  .day-btn label{display:block;padding:0.4rem 0.65rem;border-radius:6px;border:1px solid #333;cursor:pointer;font-size:0.8rem;color:#555;background:#111;margin:0;transition:all 0.15s;user-select:none}
  .day-btn input:checked+label{background:#5DCAA5;color:#0f0f0f;border-color:#5DCAA5;font-weight:600}
  button{width:100%;background:#5DCAA5;color:#0f0f0f;border:none;border-radius:8px;padding:0.75rem;font-size:1rem;font-weight:600;cursor:pointer;margin-top:0.5rem;transition:opacity 0.3s}
  button:active{opacity:0.8}
  button:disabled{opacity:0.3;cursor:not-allowed}
  .toast{display:none;background:#5DCAA5;color:#0f0f0f;border-radius:6px;padding:0.6rem 1rem;text-align:center;margin-top:1rem;font-size:0.9rem;font-weight:500}
  .conn{display:inline-block;width:7px;height:7px;border-radius:50%;margin-right:5px;vertical-align:middle}
  .conn.on{background:#5DCAA5}
  .conn.off{background:#333}
  .conn.red{background:#E24B4A}
</style>
</head>
<body>
<div class="card">
  <h1>&#9200; R&eacute;glage de l'horloge</h1>
  <p class="sub">ESP32-C3 &middot; La M&eacute;zi&egrave;re &middot; %IP%</p>

  <div class="live">
    <div style="display:flex;align-items:baseline;gap:0.5rem;margin-bottom:0.3rem">
      <div class="live-time" id="ltime">--:--:--</div>
      <span class="mercury-badge mercury-off" id="mbadge">&#9679; Veille</span>
      <span class="ntp-badge" id="ntpbadge">T</span>
    </div>
    <div class="live-date" id="ldate">&nbsp;</div>
    <div class="stats">
      <div class="stat">
        <div class="stat-label">Temp&eacute;rature</div>
        <div class="stat-val" id="ltemp">--&deg;C</div>
      </div>
      <div class="stat">
        <div class="stat-label">WiFi</div>
        <div class="stat-val" id="lwifi"><span class="conn off"></span>--</div>
      </div>
      <div class="stat">
        <div class="stat-label">M&eacute;t&eacute;o</div>
        <div class="stat-val" id="lweather">--</div>
      </div>
    </div>
    <div class="led-section">
      <div class="led-label">
        <span>Intensit&eacute; LED</span>
        <span class="led-pct" id="lpct">0%</span>
      </div>
      <div class="led-track">
        <div class="led-fill" id="lfill" style="width:0%"></div>
      </div>
    </div>
  </div>

  <form id="f">
    <div class="section-title">Jours actifs</div>
    <div class="days">
      <div class="day-btn"><input type="checkbox" name="day1" id="d1" %DAY1%><label for="d1">LUN</label></div>
      <div class="day-btn"><input type="checkbox" name="day2" id="d2" %DAY2%><label for="d2">MAR</label></div>
      <div class="day-btn"><input type="checkbox" name="day3" id="d3" %DAY3%><label for="d3">MER</label></div>
      <div class="day-btn"><input type="checkbox" name="day4" id="d4" %DAY4%><label for="d4">JEU</label></div>
      <div class="day-btn"><input type="checkbox" name="day5" id="d5" %DAY5%><label for="d5">VEN</label></div>
      <div class="day-btn"><input type="checkbox" name="day6" id="d6" %DAY6%><label for="d6">SAM</label></div>
      <div class="day-btn"><input type="checkbox" name="day0" id="d0" %DAY0%><label for="d0">DIM</label></div>
    </div>
    <hr>
    <div class="section-title">Programme LED</div>
    <div class="time-row">
      <label>D&eacute;but rampe</label>
      <input type="number" name="rampStartH" value="%RAMP_START_H%" min="0" max="23">
      <span class="sep">:</span>
      <input type="number" name="rampStartM" value="%RAMP_START_M%" min="0" max="59">
    </div>
    <div class="time-row">
      <label>Pleine intensit&eacute;</label>
      <input type="number" name="rampEndH" value="%RAMP_END_H%" min="0" max="23">
      <span class="sep">:</span>
      <input type="number" name="rampEndM" value="%RAMP_END_M%" min="0" max="59">
    </div>
    <div class="time-row">
      <label>Extinction</label>
      <input type="number" name="ledOffH" value="%LED_OFF_H%" min="0" max="23">
      <span class="sep">:</span>
      <input type="number" name="ledOffM" value="%LED_OFF_M%" min="0" max="59">
    </div>
    <hr>
    <div class="section-title">Localisation</div>
    <div class="row">
      <div><label>Latitude</label><input type="text" name="lat" value="%LAT%"></div>
      <div><label>Longitude</label><input type="text" name="lon" value="%LON%"></div>
    </div>
    <hr>
    <div class="section-title">WiFi</div>
    <label>SSID</label>
    <input type="text" name="ssid" value="%SSID%" autocomplete="off">
    <label>Mot de passe</label>
    <input type="password" name="pass" value="" placeholder="Laisser vide pour conserver" autocomplete="off">
    <button type="submit" id="savebtn">Enregistrer</button>
  </form>
  <div class="toast" id="toast">&#10003; Param&egrave;tres sauvegard&eacute;s !</div>
</div>

<script>
const DAYS = ['DIM','LUN','MAR','MER','JEU','VEN','SAM'];
let epochOffset = null;
let tickTimer   = null;
let heartbeat   = null;
let evts        = null;

function wLabel(c) {
  if (c == 0)  return '\u2600\uFE0F Soleil';
  if (c <= 2)  return '\u26C5 Nuageux';
  if (c == 3)  return '\u2601\uFE0F Couvert';
  if (c <= 67) return '\ud83c\udf27\uFE0F Pluie';
  if (c <= 77) return '\u2744\uFE0F Neige';
  if (c <= 82) return '\ud83c\udf26\uFE0F Averses';
  if (c <= 99) return '\u26A1 Orage';
  return '--';
}

function pad(n) { return String(n).padStart(2,'0'); }

function tickClock() {
  if (epochOffset === null) return;
  const now = new Date(Date.now() + epochOffset);
  document.getElementById('ltime').textContent =
    pad(now.getHours()) + ':' + pad(now.getMinutes()) + ':' + pad(now.getSeconds());
  const d = pad(now.getDate()), m = pad(now.getMonth()+1), y = now.getFullYear();
  document.getElementById('ldate').textContent =
    DAYS[now.getDay()] + ' ' + d + '/' + m + '/' + y;
}

function startTicking() {
  if (tickTimer) clearInterval(tickTimer);
  const msToNext = 1000 - (Date.now() % 1000);
  setTimeout(() => {
    tickClock();
    tickTimer = setInterval(tickClock, 1000);
  }, msToNext);
}

function setDisconnected() {
  document.getElementById('ltime').style.opacity = '0.4';
  document.getElementById('lwifi').innerHTML =
    '<span class="conn red"></span>D&eacute;connect&eacute;';
  document.getElementById('lwifi').className = 'stat-val off';
  document.getElementById('savebtn').disabled = true;
}

function resetHeartbeat() {
  clearTimeout(heartbeat);
  heartbeat = setTimeout(() => {
    setDisconnected();
    startSSE();
  }, 4000);
}

function handleState(d) {
  document.getElementById('ltime').style.opacity = '1';
  document.getElementById('savebtn').disabled = false;
  epochOffset = d.epoch * 1000 - Date.now();
  tickClock();
  startTicking();

  const ltime  = document.getElementById('ltime');
  const mbadge = document.getElementById('mbadge');
  if (d.mercury) {
    ltime.style.color  = '#5DCAA5';
    mbadge.textContent = '\u25CF Actif';
    mbadge.className   = 'mercury-badge mercury-on';
  } else {
    ltime.style.color  = '#FF8C00';
    mbadge.textContent = '\u25CF Veille';
    mbadge.className   = 'mercury-badge mercury-off';
  }

  document.getElementById('ntpbadge').style.display = d.ntp ? 'inline-block' : 'none';
  document.getElementById('ltemp').textContent = d.temp + '\u00b0C';

  document.getElementById('lwifi').innerHTML =
    '<span class="conn ' + (d.wifi ? 'on' : 'off') + '"></span>' +
    (d.wifi ? 'Connect\u00e9' : 'Hors ligne');
  document.getElementById('lwifi').className = 'stat-val' + (d.wifi ? ' ok' : '');

  const lw = document.getElementById('lweather');
  lw.textContent = d.wcode >= 0 ? wLabel(d.wcode) : (d.wifi ? 'Chargement...' : '--');
  lw.className   = 'stat-val' + (d.wcode >= 0 ? ' ok' : '');

  const pct = Math.round(d.led);
  document.getElementById('lpct').textContent  = pct + '%';
  document.getElementById('lfill').style.width = pct + '%';
}

function startSSE() {
  if (evts) { evts.close(); evts = null; }
  evts = new EventSource('/events');
  evts.addEventListener('state', e => {
    resetHeartbeat();
    try { handleState(JSON.parse(e.data)); } catch(err) {}
  });
  evts.onerror = () => {};
}

resetHeartbeat();
startSSE();

document.getElementById('f').addEventListener('submit', async function(e) {
  e.preventDefault();
  const data = new URLSearchParams(new FormData(this));
  const res = await fetch('/save', { method: 'POST', body: data });
  if (res.ok) {
    const t = document.getElementById('toast');
    t.style.display = 'block';
    setTimeout(() => t.style.display = 'none', 3000);
  }
});
</script>
</body>
</html>
)rawliteral";
