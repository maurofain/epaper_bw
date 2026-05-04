'use strict';

// ─── Helpers ──────────────────────────────────────────────────────────────────

const MAX_LOG = 500;

function appendLog(message) {
  const box = document.getElementById('log-box');
  const line = document.createElement('div');
  line.className = 'log-line ' + classifyLog(message);
  line.textContent = message;
  box.appendChild(line);
  if (box.children.length > MAX_LOG) box.removeChild(box.firstChild);
  box.scrollTop = box.scrollHeight;
}

function classifyLog(msg) {
  if (msg.includes(' TX ')) return 'tx';
  if (msg.includes(' RX:')) return 'rx';
  if (msg.toLowerCase().includes('errore')) return 'err';
  return 'info';
}

function setPortStatus(connected) {
  const dot   = document.getElementById('status-dot');
  const label = document.getElementById('status-label');
  dot.className = 'dot ' + (connected ? 'connected' : 'disconnected');
  label.textContent = connected ? 'Connesso' : 'Disconnesso';
  document.getElementById('btn-connect').textContent = connected ? 'Disconnetti' : 'Connetti';
}

// ─── Font select population ───────────────────────────────────────────────────

async function populateFontSelects() {
  const fonts = await window.api.getFonts();
  const selects = ['ext-font', 'watch-font', 'cd-font'];
  for (const id of selects) {
    const sel = document.getElementById(id);
    sel.innerHTML = '';
    for (const f of fonts) {
      const opt = document.createElement('option');
      opt.value = f.id;
      opt.textContent = `${f.id} – ${f.name} (max ${f.maxChars})`;
      if (f.id === 7) opt.selected = true;
      sel.appendChild(opt);
    }
  }
}

// ─── Port management ──────────────────────────────────────────────────────────

async function refreshPorts() {
  const ports = await window.api.listPorts();
  const sel = document.getElementById('port-select');
  const current = sel.value;
  sel.innerHTML = '<option value="">-- seleziona porta --</option>';
  for (const p of ports) {
    const opt = document.createElement('option');
    opt.value = p;
    opt.textContent = p;
    if (p === current) opt.selected = true;
    sel.appendChild(opt);
  }
}

let connected = false;

async function toggleConnect() {
  if (connected) {
    await window.api.closePort();
    connected = false;
    setPortStatus(false);
  } else {
    const port = document.getElementById('port-select').value;
    if (!port) { appendLog('[WARN] Nessuna porta selezionata'); return; }
    const res = await window.api.openPort(port);
    if (res.ok) {
      connected = true;
      setPortStatus(true);
    } else {
      appendLog(`[ERR] Impossibile aprire ${port}: ${res.error}`);
    }
  }
}

// ─── Quick command buttons ────────────────────────────────────────────────────

document.querySelectorAll('.cmd-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    const val = parseInt(btn.dataset.cmd, 16);
    window.api.cmdControl(val);
  });
});

// ─── Text AUTO ───────────────────────────────────────────────────────────────

document.getElementById('btn-text-auto').addEventListener('click', () => {
  const text = document.getElementById('text-auto-input').value;
  if (!text) return;
  window.api.cmdTextAuto(text);
});

document.getElementById('text-auto-input').addEventListener('keydown', e => {
  if (e.key === 'Enter') document.getElementById('btn-text-auto').click();
});

// ─── Text EXTENDED ───────────────────────────────────────────────────────────

document.getElementById('btn-ext-send').addEventListener('click', () => {
  const font = parseInt(document.getElementById('ext-font').value);
  const posX = parseInt(document.getElementById('ext-x').value);
  const posY = parseInt(document.getElementById('ext-y').value);
  const text = document.getElementById('ext-text').value;
  window.api.cmdTextExtended({ font, posX, posY, text });
});

document.getElementById('ext-text').addEventListener('keydown', e => {
  if (e.key === 'Enter') document.getElementById('btn-ext-send').click();
});

// ─── LED control ─────────────────────────────────────────────────────────────

// Sync color picker → RGB fields
document.querySelectorAll('.led-row').forEach(row => {
  const picker = row.querySelector('.led-color');
  const rIn = row.querySelector('.led-r');
  const gIn = row.querySelector('.led-g');
  const bIn = row.querySelector('.led-b');

  picker.addEventListener('input', () => {
    const hex = picker.value;
    rIn.value = parseInt(hex.slice(1,3), 16);
    gIn.value = parseInt(hex.slice(3,5), 16);
    bIn.value = parseInt(hex.slice(5,7), 16);
  });

  function syncPicker() {
    const r = parseInt(rIn.value) || 0;
    const g = parseInt(gIn.value) || 0;
    const b = parseInt(bIn.value) || 0;
    picker.value = '#' + [r,g,b].map(v => v.toString(16).padStart(2,'0')).join('');
  }
  rIn.addEventListener('input', syncPicker);
  gIn.addEventListener('input', syncPicker);
  bIn.addEventListener('input', syncPicker);
});

document.getElementById('btn-led-send').addEventListener('click', () => {
  const leds = [];
  document.querySelectorAll('.led-row').forEach(row => {
    leds.push({
      r: Math.min(255, Math.max(0, parseInt(row.querySelector('.led-r').value) || 0)),
      g: Math.min(255, Math.max(0, parseInt(row.querySelector('.led-g').value) || 0)),
      b: Math.min(255, Math.max(0, parseInt(row.querySelector('.led-b').value) || 0)),
    });
  });
  window.api.cmdLed(leds);
});

// ─── Watch mode ───────────────────────────────────────────────────────────────

let watchTimer = null;

function watchTick() {
  const font   = parseInt(document.getElementById('watch-font').value);
  const bold   = document.getElementById('watch-bold').checked;
  const text   = document.getElementById('watch-text').value;
  const sec    = new Date().getSeconds();
  const prefix = bold ? 'ç§' : '§';
  const full   = text ? `${text}\n${String(sec).padStart(2,'0')}s` : String(sec);
  window.api.cmdTextExtended({ font, posX: 100, posY: 100, text: prefix + full });
  document.getElementById('watch-status').textContent = `secondi: ${String(sec).padStart(2,'0')}`;
}

document.getElementById('btn-watch-start').addEventListener('click', () => {
  if (watchTimer) return;
  // partial refresh before start
  window.api.cmdControl(0x00);
  watchTick();
  watchTimer = setInterval(watchTick, 1000);
  document.getElementById('btn-watch-start').disabled = true;
  document.getElementById('btn-watch-stop').disabled = false;
});

document.getElementById('btn-watch-stop').addEventListener('click', () => {
  clearInterval(watchTimer);
  watchTimer = null;
  document.getElementById('btn-watch-start').disabled = false;
  document.getElementById('btn-watch-stop').disabled = true;
  document.getElementById('watch-status').textContent = '';
});

// ─── Countdown mode ───────────────────────────────────────────────────────────

let cdTimer  = null;
let cdCounter = 60;
let cdInverted = false;

function cdTick() {
  const font   = parseInt(document.getElementById('cd-font').value);
  const bold   = document.getElementById('cd-bold').checked;
  const text   = document.getElementById('cd-text').value;
  const prefix = bold ? 'ç§' : '§';
  const countStr = String(cdCounter).padStart(2, '0');
  const full   = text ? `${text}\n${countStr}` : countStr;
  window.api.cmdTextExtended({ font, posX: 100, posY: 100, text: prefix + full });
  document.getElementById('cd-status').textContent = `contatore: ${countStr}`;

  if (cdCounter === 15 && !cdInverted) {
    window.api.cmdControl(0x05); // tema invertito
    cdInverted = true;
  }
  if (cdCounter === 0) {
    window.api.cmdControl(0x04); // tema normale
    cdInverted = false;
    cdCounter = 60;
  } else {
    cdCounter--;
  }
}

document.getElementById('btn-cd-start').addEventListener('click', () => {
  if (cdTimer) return;
  cdCounter = 60;
  cdInverted = false;
  window.api.cmdControl(0x00);
  cdTick();
  cdTimer = setInterval(cdTick, 1000);
  document.getElementById('btn-cd-start').disabled = true;
  document.getElementById('btn-cd-stop').disabled = false;
});

document.getElementById('btn-cd-stop').addEventListener('click', () => {
  clearInterval(cdTimer);
  cdTimer = null;
  document.getElementById('btn-cd-start').disabled = false;
  document.getElementById('btn-cd-stop').disabled = true;
  document.getElementById('cd-status').textContent = '';
});

// ─── Font demo ────────────────────────────────────────────────────────────────

let demoTimer   = null;
let demoFonts   = [];
let demoIndex   = 0;

async function demoTick() {
  if (demoIndex >= demoFonts.length) {
    stopDemo();
    document.getElementById('demo-status').textContent = 'Demo completata';
    return;
  }
  const f = demoFonts[demoIndex++];
  // clear
  await window.api.cmdControl(0x00);
  await delay(500);
  // font name at top
  await window.api.cmdTextExtended({ font: 1, posX: 10, posY: 20, text: `(${f.id}) ${f.name}` });
  await delay(100);
  // digit with that font
  await window.api.cmdTextExtended({ font: f.id, posX: 100, posY: 100, text: '19' });
  document.getElementById('demo-status').textContent = `font ${f.id}/15: ${f.name}`;
}

function delay(ms) { return new Promise(r => setTimeout(r, ms)); }

function stopDemo() {
  clearInterval(demoTimer);
  demoTimer = null;
  document.getElementById('btn-demo-start').disabled = false;
  document.getElementById('btn-demo-stop').disabled = true;
}

document.getElementById('btn-demo-start').addEventListener('click', async () => {
  if (demoTimer) return;
  demoFonts = await window.api.getFonts();
  demoIndex = 0;
  document.getElementById('btn-demo-start').disabled = true;
  document.getElementById('btn-demo-stop').disabled = false;
  demoTick();
  demoTimer = setInterval(demoTick, 4000);
});

document.getElementById('btn-demo-stop').addEventListener('click', stopDemo);

// ─── IPC event listeners ──────────────────────────────────────────────────────

window.api.onLog(appendLog);
window.api.onRxLine(line => {
  // auto-init on boot detection
  if (line.includes('[M] init completed')) {
    window.api.cmdControl(0xAA);
    appendLog('[AUTO] Init inviato automaticamente');
  }
});
window.api.onPortError(msg => {
  appendLog(`[ERR] ${msg}`);
  connected = false;
  setPortStatus(false);
});
window.api.onPortClosed(() => {
  connected = false;
  setPortStatus(false);
  // stop any active timers
  if (watchTimer) { clearInterval(watchTimer); watchTimer = null; }
  if (cdTimer)    { clearInterval(cdTimer);    cdTimer    = null; }
  if (demoTimer)  { stopDemo(); }
});

// ─── Init ─────────────────────────────────────────────────────────────────────

document.getElementById('btn-connect').addEventListener('click', toggleConnect);
document.getElementById('btn-refresh-ports').addEventListener('click', refreshPorts);
document.getElementById('btn-clear-log').addEventListener('click', () => {
  document.getElementById('log-box').innerHTML = '';
});

(async () => {
  await populateFontSelects();
  await refreshPorts();
})();
