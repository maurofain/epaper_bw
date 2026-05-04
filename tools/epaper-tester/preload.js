'use strict';

const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  listPorts:       ()       => ipcRenderer.invoke('list-ports'),
  openPort:        (path)   => ipcRenderer.invoke('open-port', path),
  closePort:       ()       => ipcRenderer.invoke('close-port'),
  portStatus:      ()       => ipcRenderer.invoke('port-status'),

  cmdControl:      (sub)    => ipcRenderer.invoke('cmd-control', sub),
  cmdTextAuto:     (text)   => ipcRenderer.invoke('cmd-text-auto', text),
  cmdTextExtended: (opts)   => ipcRenderer.invoke('cmd-text-extended', opts),
  cmdLed:          (leds)   => ipcRenderer.invoke('cmd-led', leds),
  getFonts:        ()       => ipcRenderer.invoke('get-fonts'),

  onLog:           (cb)     => ipcRenderer.on('log',        (_, m) => cb(m)),
  onRxLine:        (cb)     => ipcRenderer.on('rx-line',    (_, m) => cb(m)),
  onPortError:     (cb)     => ipcRenderer.on('port-error', (_, m) => cb(m)),
  onPortClosed:    (cb)     => ipcRenderer.on('port-closed', ()    => cb()),
});
