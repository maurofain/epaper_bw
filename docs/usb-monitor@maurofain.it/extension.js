import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';

export default class UsbMonitorExtension extends Extension {
    enable() {
        this._devDir = Gio.File.new_for_path('/dev');
        this._monitor = this._devDir.monitor_directory(Gio.FileMonitorFlags.NONE, null);
        
        // Colleghiamo il segnale di cambio file
        this._monitor.connect('changed', (monitor, file, otherFile, eventType) => {
            const fileName = file.get_basename();
            
            if (fileName.startsWith('ttyACM') || fileName.startsWith('ttyUSB')) {
                if (eventType === Gio.FileMonitorEvent.CREATED) {
                    console.log(`Dispositivo rilevato: ${fileName}`);
                    // Aggiungi qui la tua logica (es: notifica)
                } else if (eventType === Gio.FileMonitorEvent.DELETED) {
                    console.log(`Dispositivo rimosso: ${fileName}`);
                }
            }
        });
    }

    disable() {
        this._monitor.cancel();
        this._monitor = null;
    }
}
