#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

#include <Arduino.h>

// Threshold ranges below (RANGES object) are injected from config.h at boot
// via __TOKEN__ placeholder substitution in main.cpp - see buildDashboardHtml().

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Environmental Monitor</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Space+Grotesk:wght@500;700&family=IBM+Plex+Mono:wght@500;600&family=Inter:wght@400;500;600&display=swap" rel="stylesheet">
<style>
  :root {
    --bg-deep: #0a1620;
    --bg-panel: #101f2b;
    --bg-panel-2: #0d1a24;
    --text-primary: #eef6fa;
    --text-dim: #6f8a9a;
    --teal: #35c4b0;
    --amber: #e3a640;
    --red: #e2574c;
    --green: #34c77b;
    --hairline: rgba(255,255,255,0.07);
  }
  * { box-sizing: border-box; }
  html, body {
    margin: 0;
    min-height: 100vh;
    background:
      radial-gradient(circle at 15% 8%, rgba(53,196,176,0.07), transparent 40%),
      radial-gradient(circle at 85% 92%, rgba(227,166,64,0.05), transparent 45%),
      var(--bg-deep);
    background-attachment: fixed;
  }
  body {
    font-family: 'Inter', -apple-system, sans-serif;
    color: var(--text-primary);
    padding: 36px 20px 56px;
    position: relative;
  }
  body::before {
    content: "";
    position: fixed;
    inset: 0;
    background-image:
      linear-gradient(rgba(255,255,255,0.025) 1px, transparent 1px),
      linear-gradient(90deg, rgba(255,255,255,0.025) 1px, transparent 1px);
    background-size: 34px 34px;
    pointer-events: none;
    z-index: 0;
  }
  .wrap { position: relative; z-index: 1; max-width: 980px; margin: 0 auto; }

  header { text-align: center; margin-bottom: 40px; }
  .eyebrow {
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.72rem;
    letter-spacing: 3px;
    color: var(--teal);
    text-transform: uppercase;
    margin-bottom: 10px;
  }
  h1 {
    font-family: 'Space Grotesk', sans-serif;
    font-weight: 700;
    font-size: 2.1rem;
    margin: 0 0 12px;
    letter-spacing: -0.3px;
  }
  .status-line {
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.8rem;
    color: var(--text-dim);
    display: inline-flex;
    align-items: center;
    gap: 8px;
  }
  .dot {
    width: 7px; height: 7px; border-radius: 50%;
    background: var(--green);
    box-shadow: 0 0 8px var(--green);
    animation: pulse 2s ease-in-out infinite;
  }
  .dot.offline { background: var(--red); box-shadow: 0 0 8px var(--red); animation: none; }
  .dot.stale { background: var(--amber); box-shadow: 0 0 8px var(--amber); animation: none; }
  @keyframes pulse { 0%,100% { opacity: 1; } 50% { opacity: 0.4; } }

  .device-info {
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.72rem;
    color: var(--text-dim);
    margin-top: 6px;
    letter-spacing: 0.5px;
  }

  .gauges {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
    gap: 18px;
    margin-bottom: 18px;
  }
  .gauge-card {
    background: linear-gradient(180deg, var(--bg-panel), var(--bg-panel-2));
    border: 1px solid var(--hairline);
    border-radius: 18px;
    padding: 24px 20px 20px;
    text-align: center;
  }
  .gauge-label {
    display:flex;
    align-items:center;
    justify-content:center;
    gap:8px;
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.7rem;
    letter-spacing: 2px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 14px;
  }
  
  .label-icon{
    width:18px;
    height:18px;
    color:var(--teal);
    flex-shrink:0;
  }

  .dial {
    position: relative;
    width: 168px;
    height: 90px;
    margin: 0 auto 6px;
    overflow: hidden;
  }
  .dial-ring {
    position: absolute;
    top: 0; left: 0;
    width: 168px; height: 168px;
    border-radius: 50%;
  }
  .dial-mask {
    position: absolute;
    top: 18px; left: 18px;
    width: 132px; height: 132px;
    border-radius: 50%;
    background: var(--bg-panel-2);
  }
  .needle {
    position: absolute;
    left: 50%; bottom: 0;
    margin-left: -1.5px;
    width: 3px; height: 76px;
    background: var(--text-primary);
    border-radius: 2px 2px 0 0;
    transform-origin: bottom center;
    transform: rotate(0deg);
    transition: transform 0.6s cubic-bezier(0.34, 1.2, 0.4, 1);
    box-shadow: 0 0 6px rgba(255,255,255,0.5);
  }
  .pivot {
    position: absolute;
    left: 50%; bottom: -5px;
    width: 11px; height: 11px;
    background: var(--text-primary);
    border-radius: 50%;
    transform: translateX(-50%);
  }
  .readout {
    font-family: 'IBM Plex Mono', monospace;
    font-weight: 600;
    font-size: 1.85rem;
    line-height: 1;
    margin-top: 4px;
  }
  .readout .unit { font-size: 0.95rem; color: var(--text-dim); font-weight: 500; }
  .badge {
    display: inline-block;
    margin-top: 10px;
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.68rem;
    letter-spacing: 1px;
    padding: 3px 10px;
    border-radius: 999px;
    text-transform: uppercase;
    font-weight: 600;
  }
  .badge.green  { background: rgba(52,199,123,0.14); color: var(--green); }
  .badge.yellow { background: rgba(227,166,64,0.14); color: var(--amber); }
  .badge.red    { background: rgba(226,87,76,0.16); color: var(--red); }

  .footer-bar {
    background: linear-gradient(180deg, var(--bg-panel), var(--bg-panel-2));
    border: 1px solid var(--hairline);
    border-radius: 18px;
    padding: 18px 22px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    flex-wrap: wrap;
    gap: 14px;
  }
  .update-label {
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.7rem;
    letter-spacing: 1.5px;
    text-transform: uppercase;
    color: var(--text-dim);
    margin-bottom: 4px;
  }
  .update-value { font-family: 'IBM Plex Mono', monospace; font-size: 0.95rem; }

  .btn-group { display: flex; gap: 10px; }
  button.action {
    font-family: 'Inter', sans-serif;
    font-weight: 600;
    font-size: 0.9rem;
    border: none;
    border-radius: 10px;
    padding: 11px 18px;
    cursor: pointer;
    display: inline-flex;
    align-items: center;
    gap: 7px;
    transition: transform 0.15s ease, filter 0.15s ease;
  }
  button.action:hover { filter: brightness(1.1); transform: translateY(-1px); }
  button.action:active { transform: translateY(0); }
  button.action svg { width: 16px; height: 16px; }

  .btn-reload {
    background: var(--bg-deep);
    color: var(--text-primary);
    border: 1px solid var(--hairline) !important;
  }
  .btn-reload svg { transition: transform 0.5s ease; }
  .btn-reload.spinning svg { transform: rotate(360deg); }

  .btn-download { background: var(--teal); color: #06231e; }

  footer.credit {
    text-align: center;
    color: var(--text-dim);
    font-family: 'IBM Plex Mono', monospace;
    font-size: 0.68rem;
    letter-spacing: 0.5px;
    margin-top: 26px;
  }

  @media (prefers-reduced-motion: reduce) {
    .dot { animation: none; }
    .needle { transition: none; }
  }
</style>
</head>
<body>
<div class="wrap">

  <header>
    <div class="eyebrow">Station 01 &middot; BME280</div>
    <h1>Environmental Monitor</h1>
    <div class="status-line"><span class="dot" id="statusDot"></span><span id="statusText">Connecting</span></div>
    <div class="device-info" id="deviceInfo">&nbsp;</div>
  </header>

  <div class="gauges">

    <div class="gauge-card">
      <div class="gauge-label"><svg class="label-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 14.76V5a2 2 0 10-4 0v9.76a4 4 0 104 0z"/><line x1="12" y1="11" x2="12" y2="17"/></svg><span>Temperature</span></div>
      <div class="dial">
        <div class="dial-ring" id="tempRing"></div>
        <div class="dial-mask"></div>
        <div class="needle" id="tempNeedle"></div>
        <div class="pivot"></div>
      </div>
      <div class="readout" id="tempValue">--<span class="unit"> &deg;C</span></div>
      <span class="badge" id="tempBadge">--</span>
    </div>

    <div class="gauge-card">
      <div class="gauge-label"><svg class="label-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 3C9 7 6 10 6 14a6 6 0 0012 0c0-4-3-7-6-11z"/></svg><span>Relative Humidity</span></div>
      <div class="dial">
        <div class="dial-ring" id="humRing"></div>
        <div class="dial-mask"></div>
        <div class="needle" id="humNeedle"></div>
        <div class="pivot"></div>
      </div>
      <div class="readout" id="humValue">--<span class="unit"> %</span></div>
      <span class="badge" id="humBadge">--</span>
    </div>

    <div class="gauge-card">
      <div class="gauge-label"><svg class="label-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M4 14a8 8 0 1116 0"/><path d="M12 14l4-4"/></svg><span>Barometric Pressure</span></div>
      <div class="dial">
        <div class="dial-ring" id="presRing"></div>
        <div class="dial-mask"></div>
        <div class="needle" id="presNeedle"></div>
        <div class="pivot"></div>
      </div>
      <div class="readout" id="presValue">--<span class="unit"> hPa</span></div>
      <span class="badge" id="presBadge">--</span>
    </div>

  </div>

  <div class="footer-bar">
    <div>
      <div class="update-label">Last Update</div>
      <div class="update-value" id="lastUpdate">--</div>
    </div>
    <div class="btn-group">
      <button class="action btn-reload" id="reloadBtn" onclick="manualReload()">
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M4 4v6h6M20 20v-6h-6"/>
          <path d="M4.5 15a8 8 0 0014.5 3M19.5 9A8 8 0 005 6"/>
        </svg>
        Reload
      </button>
      <button class="action btn-download" onclick="window.location.href='/download'">
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M12 3v12m0 0l-4-4m4 4l4-4M4 19h16"/>
        </svg>
        Download CSV
      </button>
    </div>
  </div>

  <footer class="credit">AUTO-REFRESH 10S &middot; ESP32 + BME280</footer>

</div>

<script>
  // Values come from config.h (see top-of-file note). "min"/"max" define the
  // gauge's full sweep (a bit wider than the yellow band so the
  // needle has room to move before pinning at either end).
  // Sourced directly from config.h (substituted server-side at boot) so
  // there's a single source of truth for thresholds - no manual sync needed.
  // green/yellow are the named zones; red is implicit outside yellow, and
  // min/max (== the RED_MIN/RED_MAX bounds) set the gauge's full sweep.
  const RANGES = {
    temp: { min: __TEMP_RED_MIN__, max: __TEMP_RED_MAX__, yellowMin: __TEMP_YELLOW_MIN__, greenMin: __TEMP_GREEN_MIN__, greenMax: __TEMP_GREEN_MAX__, yellowMax: __TEMP_YELLOW_MAX__ },
    hum:  { min: __HUM_RED_MIN__, max: __HUM_RED_MAX__, yellowMin: __HUM_YELLOW_MIN__, greenMin: __HUM_GREEN_MIN__, greenMax: __HUM_GREEN_MAX__, yellowMax: __HUM_YELLOW_MAX__ },
    pres: { min: __PRES_RED_MIN__, max: __PRES_RED_MAX__, yellowMin: __PRES_YELLOW_MIN__, greenMin: __PRES_GREEN_MIN__, greenMax: __PRES_GREEN_MAX__, yellowMax: __PRES_YELLOW_MAX__ }
  };

  function buildRingGradient(r) {
    const span = r.max - r.min;
    const pct = v => ((v - r.min) / span) * 180;
    const stops = [
      ['var(--red)', 0, pct(r.yellowMin)],
      ['var(--amber)', pct(r.yellowMin), pct(r.greenMin)],
      ['var(--green)', pct(r.greenMin), pct(r.greenMax)],
      ['var(--amber)', pct(r.greenMax), pct(r.yellowMax)],
      ['var(--red)', pct(r.yellowMax), 180]
    ];
    const parts = stops.map(s => `${s[0]} ${s[1]}deg ${s[2]}deg`).join(', ');
    return `conic-gradient(from 270deg at 50% 100%, ${parts}, transparent 180deg 360deg)`;
  }

  function classify(value, r) {
    if (value >= r.greenMin && value <= r.greenMax) return 'green';
    if (value >= r.yellowMin && value <= r.yellowMax) return 'yellow';
    return 'red';
  }

  function needleAngle(value, r) {
    const clamped = Math.max(r.min, Math.min(r.max, value));
    const pct = (clamped - r.min) / (r.max - r.min);
    return -90 + pct * 180;
  }

  function initRings() {
    document.getElementById('tempRing').style.background = buildRingGradient(RANGES.temp);
    document.getElementById('humRing').style.background = buildRingGradient(RANGES.hum);
    document.getElementById('presRing').style.background = buildRingGradient(RANGES.pres);
  }

  function applyGauge(key, value, unit, decimals) {
    const r = RANGES[key];
    const level = classify(value, r);

    document.getElementById(key + 'Needle').style.transform = `rotate(${needleAngle(value, r)}deg)`;
    document.getElementById(key + 'Value').innerHTML = value.toFixed(decimals) + `<span class="unit"> ${unit}</span>`;

    const badge = document.getElementById(key + 'Badge');
    badge.classList.remove('green', 'yellow', 'red');
    badge.classList.add(level);
    badge.textContent = level === 'green' ? 'Normal' : level === 'yellow' ? 'Warning' : 'Critical';
  }

  async function refreshData() {
    const statusDot = document.getElementById('statusDot');
    const statusText = document.getElementById('statusText');
    try {
      const res = await fetch('/data', { cache: 'no-store' });
      const data = await res.json();

      if (!data.ok) {
        statusDot.classList.remove('stale');
        statusDot.classList.add('offline');
        statusText.textContent = 'Connected \u2014 sensor not ready';
        return;
      }

      statusDot.classList.remove('offline');

      applyGauge('temp', data.temperature, '&deg;C', 1);
      applyGauge('hum', data.humidity, '%', 1);
      applyGauge('pres', data.pressure, 'hPa', 1);

      if (data.stale) {
        // Restored from flash after a reboot - not a fresh sensor reading yet.
        statusDot.classList.add('stale');
        statusText.textContent = 'Connected \u2014 showing cached reading';
        document.getElementById('lastUpdate').textContent = data.secondsAgo >= 0
          ? data.timestamp + ' (' + data.secondsAgo + 's ago, before reboot)'
          : data.timestamp + ' (before reboot)';
      } else {
        statusDot.classList.remove('stale');
        statusText.textContent = 'Connected \u2014 live';
        document.getElementById('lastUpdate').textContent = data.timestamp + ' (' + data.secondsAgo + 's ago)';
      }
    } catch (err) {
      statusDot.classList.remove('stale');
      statusDot.classList.add('offline');
      statusText.textContent = 'Connection lost';
      document.getElementById('deviceInfo').innerHTML = '&nbsp;';
    }
  }

  async function refreshStatus() {
    try {
      const res = await fetch('/status', { cache: 'no-store' });
      const s = await res.json();
      const rssiText = (s.rssi !== null && s.rssi !== undefined) ? ` \u00b7 ${s.rssi} dBm` : '';
      document.getElementById('deviceInfo').textContent = `${s.mode} \u00b7 ${s.ssid} \u00b7 ${s.ip}${rssiText}`;
    } catch (err) {
      // /status failing too just reinforces the "connection lost" state from refreshData
    }
  }

  function manualReload() {
    const btn = document.getElementById('reloadBtn');
    btn.classList.add('spinning');
    refreshData().finally(() => setTimeout(() => btn.classList.remove('spinning'), 500));
  }

  initRings();
  refreshData();
  refreshStatus();
  setInterval(refreshData, 10000);
  setInterval(refreshStatus, 10000);
</script>

</body>
</html>
)rawliteral";

#endif
