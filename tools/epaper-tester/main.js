'use strict';

const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const fs = require('fs');
const { SerialPort } = require('serialport');

let mainWindow;
let serialPort = null;
let rxBuffer = Buffer.alloc(0);
let execLog = null;

// ─── Protocol constants ──────────────────────────────────────────────────────

const DEFAULT_BAUD = 9600;
const READ_TIMEOUT_MS = 50;

const FONT_DATABASE = [
  { id: 1,  name: 'montserrat_14',     maxChars: 12 },
  { id: 2,  name: 'montserrat_28',     maxChars: 5  },
  { id: 3,  name: 'montserrat_48',     maxChars: 2  },
  { id: 4,  name: 'GoogleSans10',      maxChars: 18 },
  { id: 5,  name: 'GoogleSans15',      maxChars: 15 },
  { id: 6,  name: 'GoogleSans20',      maxChars: 12 },
  { id: 7,  name: 'GoogleSans35',      maxChars: 6  },
  { id: 8,  name: 'GoogleSans50',      maxChars: 5  },
  { id: 9,  name: 'GoogleSans60',      maxChars: 4  },
  { id: 10, name: 'GoogleSans100',     maxChars: 3  },
  { id: 11, name: 'GoogleSans140',     maxChars: 2  },
  { id: 12, name: 'GoogleSansBold40',  maxChars: 5  },
  { id: 13, name: 'GoogleSansBold60',  maxChars: 4  },
  { id: 14, name: 'GoogleSansBold100', maxChars: 3  },
  { id: 15, name: 'GoogleSansBold140', maxChars: 2  },
];

// ─── Helpers ─────────────────────────────────────────────────────────────────

function toHex(buf) {
  return Array.from(buf).map(b => b.toString(16).padStart(2, '0').toUpperCase()).join(' ');
}

function toAscii(buf) {
  return Array.from(buf).map(b => (b >= 32 && b <= 126) ? String.fromCharCode(b) : '.').join('');
}

function normalizeText(text) {
  // replace literal \n with CR (0x0D), encode as latin-1
  const replaced = text.replace(/\\n/g, '\r');
  const buf = Buffer.alloc(replaced.length);
  for (let i = 0; i < replaced.length; i++) {
    buf[i] = replaced.charCodeAt(i) & 0xFF;
  }
  return buf;
}

function selectFont(text, useBold = false, useMontserrat = false) {
  const len = text.length;
  let candidates;
  if (useMontserrat) candidates = FONT_DATABASE.filter(f => f.id >= 1 && f.id <= 3);
  else if (useBold)  candidates = FONT_DATABASE.filter(f => f.id >= 12 && f.id <= 15);
  else               candidates = FONT_DATABASE.filter(f => f.id >= 4 && f.id <= 11);

  for (let i = candidates.length - 1; i >= 0; i--) {
    if (len <= candidates[i].maxChars) return candidates[i];
  }
  return candidates[0];
}

function openExecLog() {
  try {
    execLog = fs.createWriteStream('exec.log', { flags: 'a' });
  } catch (e) {
    execLog = null;
  }
}

function appendExecLog(data) {
  if (execLog) execLog.write(data);
}

function sendLog(message) {
  const ts = new Date().toLocaleTimeString('it-IT');
  mainWindow && mainWindow.webContents.send('log', `[${ts}] ${message}`);
}

// ─── Serial port management ───────────────────────────────────────────────────

async function listPorts() {
  const ports = await SerialPort.list();
  return ports.map(p => p.path);
}

function openPort(portPath) {
  return new Promise((resolve, reject) => {
    if (serialPort && serialPort.isOpen) {
      serialPort.close();
    }
    serialPort = new SerialPort({
      path: portPath,
      baudRate: DEFAULT_BAUD,
      dataBits: 8,
      parity: 'none',
      stopBits: 1,
      autoOpen: false,
    });

    serialPort.open(err => {
      if (err) return reject(err.message);

      serialPort.on('data', data => {
        appendExecLog(data);
        rxBuffer = Buffer.concat([rxBuffer, data]);
        // split on CR/LF
        let idx;
        while ((idx = rxBuffer.findIndex(b => b === 0x0D || b === 0x0A)) !== -1) {
          const line = rxBuffer.slice(0, idx);
          rxBuffer = rxBuffer.slice(idx + 1);
          if (line.length === 0) continue;
          const text = toAscii(line);
          sendLog(`RX: ${text}`);
          mainWindow && mainWindow.webContents.send('rx-line', text);
        }
      });

      serialPort.on('error', err => {
        sendLog(`Errore seriale: ${err.message}`);
        mainWindow && mainWindow.webContents.send('port-error', err.message);
      });

      serialPort.on('close', () => {
        sendLog('Porta seriale chiusa');
        mainWindow && mainWindow.webContents.send('port-closed');
      });

      sendLog(`Porta ${portPath} aperta`);
      resolve();
    });
  });
}

function closePort() {
  return new Promise((resolve) => {
    if (serialPort && serialPort.isOpen) {
      serialPort.close(() => resolve());
    } else {
      resolve();
    }
  });
}

function sendPacket(packet, label) {
  return new Promise((resolve) => {
    if (!serialPort || !serialPort.isOpen) {
      const msg = 'Porta non aperta';
      sendLog(`ERRORE TX ${label}: ${msg}`);
      resolve({ ok: false, error: msg });
      return;
    }
    sendLog(`TX ${label}: len=${packet.length} hex=[${toHex(packet)}]`);
    appendExecLog(packet);
    serialPort.write(packet, err => {
      if (err) {
        sendLog(`Errore scrittura: ${err.message}`);
        resolve({ ok: false, error: err.message });
        return;
      }
      serialPort.drain(() => resolve({ ok: true }));
    });
  });
}

// ─── IPC handlers ────────────────────────────────────────────────────────────

ipcMain.handle('list-ports', async () => {
  return await listPorts();
});

ipcMain.handle('open-port', async (_, portPath) => {
  try {
    await openPort(portPath);
    return { ok: true };
  } catch (e) {
    return { ok: false, error: e };
  }
});

ipcMain.handle('close-port', async () => {
  await closePort();
  return { ok: true };
});

ipcMain.handle('port-status', () => {
  return { open: !!(serialPort && serialPort.isOpen) };
});

// ── CMD 0x00 ──────────────────────────────────────────────────────────────────
ipcMain.handle('cmd-control', async (_, subCmd) => {
  // subCmd: 0x00..0x05, 0xAA (init), 0xBB (reboot), 0xEE (disconnect), 0xFF (logo)
  const packet = Buffer.from([0x00, subCmd]);
  const labels = {
    0x00: 'CMD refresh totale',
    0x01: 'CMD scanner ON + refresh',
    0x02: 'CMD scanner ON',
    0x03: 'CMD scanner OFF',
    0x04: 'CMD tema normale',
    0x05: 'CMD tema invertito',
    0xAA: 'CMD init',
    0xBB: 'CMD reboot',
    0xEE: 'CMD disconnect',
    0xFF: 'CMD logo',
  };
  return sendPacket(packet, labels[subCmd] || `CMD 0x${subCmd.toString(16)}`);
});

// ── TEXT AUTO 0x01 ───────────────────────────────────────────────────────────
ipcMain.handle('cmd-text-auto', async (_, text) => {
  const useBold = text.startsWith('ç') || text.includes('\xe7');
  const useMontserrat = text.startsWith('£') || text.includes('\xa3');
  const font = selectFont(text, useBold, useMontserrat);
  const payload = normalizeText(text);
  const truncated = payload.slice(0, 0xFE);
  const packet = Buffer.concat([Buffer.from([0x01, truncated.length]), truncated]);
  return sendPacket(packet, `TEXT AUTO (font ${font.id}: ${font.name})`);
});

// ── TEXT EXTENDED 0x01 0xFF ──────────────────────────────────────────────────
ipcMain.handle('cmd-text-extended', async (_, { font, posX, posY, text }) => {
  const payload = normalizeText(text);
  const truncated = payload.slice(0, 250);
  const header = Buffer.from([0x01, 0xFF, font, posX, posY]);
  const packet = Buffer.concat([header, truncated, Buffer.from([0x00])]);
  return sendPacket(packet, `TEXT EXTENDED (font ${font})`);
});

// ── LED 0x02 ─────────────────────────────────────────────────────────────────
ipcMain.handle('cmd-led', async (_, leds) => {
  // leds: [{r,g,b}, {r,g,b}, {r,g,b}, {r,g,b}]
  const values = leds.flatMap(l => [l.r, l.g, l.b]);
  const packet = Buffer.from([0x02, ...values]);
  return sendPacket(packet, 'LED VALUES');
});

// ── FONT DATABASE ─────────────────────────────────────────────────────────────
ipcMain.handle('get-fonts', () => FONT_DATABASE);

// ─── App lifecycle ────────────────────────────────────────────────────────────

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1100,
    height: 760,
    minWidth: 800,
    minHeight: 600,
    title: 'EpaperQr RS232 Tester',
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });

  mainWindow.loadFile('index.html');
  openExecLog();
}

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  closePort().then(() => {
    if (execLog) execLog.end();
    app.quit();
  });
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
