#include "pin_config.h"
#include "scanner_control.h"

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>

static const char* TAG = "ScannerControl";
static bool scannerInitialized = false;
static std::string scannerBuffer;

#if defined(SCANNER_CONTROL_USE_SERIAL)
static const uint8_t START_SCAN_CMD[] = {0x01, 'T', 0x04};
static const uint8_t STOP_SCAN_CMD[] = {0x01, 'P', 0x04};
#endif

static void configureOutputPin(gpio_num_t pin) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "[C] gpio_config failed pin=%d err=0x%02X", static_cast<int>(pin), err);
    } else {
        ESP_LOGD(TAG, "[C] GPIO output configured pin=%d", static_cast<int>(pin));
    }
}

void setupScannerPins() {
    ESP_LOGI(TAG, "[C] setupScannerPins start");
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG), 1);
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 1);
#endif
    ESP_LOGI(TAG, "[C] Scanner control pins configured (RESET=%d%s)",
             PIN_QR_BCRES_CFG,
#if defined(SCANNER_CONTROL_USE_TRIGGER)
             ", TRIGGER="
#else
             ""
#endif
    );
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    ESP_LOGI(TAG, "[C] Scanner trigger pin configured (TRIGGER=%d)", PIN_QR_BCTRIG_CFG);
#endif
}

void initializeScanner() {
    if (scannerInitialized) {
        ESP_LOGD(TAG, "[C] initializeScanner skipped: already initialized");
        return;
    }
    ESP_LOGI(TAG, "[C] initializeScanner start");
    scannerInitialized = true;
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG), 1);
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 1);
#endif
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "[C] Scanner control pins configured");
    ESP_LOGI(TAG, "[C] Scanner initialization phase complete");
}

#if defined(SCANNER_CONTROL_USE_SERIAL)
void scannerOn(uart_port_t scannerPort) {
    initializeScanner();
    const int written = uart_write_bytes(scannerPort, reinterpret_cast<const char*>(START_SCAN_CMD), sizeof(START_SCAN_CMD));
    ESP_LOGI(TAG, "[C] Scanner ON via serial (uart=%d, bytes=%d)", static_cast<int>(scannerPort), written);
}

void scannerOff(uart_port_t scannerPort) {
    initializeScanner();
    const int written = uart_write_bytes(scannerPort, reinterpret_cast<const char*>(STOP_SCAN_CMD), sizeof(STOP_SCAN_CMD));
    ESP_LOGI(TAG, "[C] Scanner OFF via serial (uart=%d, bytes=%d)", static_cast<int>(scannerPort), written);
}
#else
void scannerOn() {
    initializeScanner();
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 0);
    ESP_LOGI(TAG, "[C] Scanner ON via trigger (pin=%d level=0)", PIN_QR_BCTRIG_CFG);
}

void scannerOff() {
    initializeScanner();
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 1);
    ESP_LOGI(TAG, "[C] Scanner OFF via trigger (pin=%d level=1)", PIN_QR_BCTRIG_CFG);
}
#endif

void forwardScannerData(uart_port_t source, uart_port_t destination) {
    size_t bufferedLen = 0;
    uart_get_buffered_data_len(source, &bufferedLen);
    if (bufferedLen == 0) {
        return;
    }
    ESP_LOGD(TAG, "[C] Forwarding scanner data start (src=%d dst=%d buffered=%u)",
             static_cast<int>(source),
             static_cast<int>(destination),
             static_cast<unsigned>(bufferedLen));

    size_t forwardedBytes = 0;
    size_t forwardedLines = 0;
    while (true) {
        uint8_t c;
        int len = uart_read_bytes(source, &c, 1, 0);
        if (len <= 0) {
            break;
        }
        if (c == '\r' || c == '\n') {
            if (!scannerBuffer.empty()) {
                ESP_LOGI(TAG, "[C] Scanner line complete (len=%u): %s",
                         static_cast<unsigned>(scannerBuffer.size()),
                         scannerBuffer.c_str());
                const int writtenPayload = uart_write_bytes(destination, scannerBuffer.c_str(), scannerBuffer.size());
                const int writtenCrLf = uart_write_bytes(destination, "\r\n", 2);
                if (writtenPayload > 0) {
                    forwardedBytes += static_cast<size_t>(writtenPayload);
                }
                if (writtenCrLf > 0) {
                    forwardedBytes += static_cast<size_t>(writtenCrLf);
                }
                forwardedLines++;
                scannerBuffer.clear();
            }
        } else {
            scannerBuffer.push_back(static_cast<char>(c));
            if (scannerBuffer.length() > 240) {
                ESP_LOGW(TAG, "[C] Scanner chunk flush (len=%u)", static_cast<unsigned>(scannerBuffer.size()));
                const int writtenPayload = uart_write_bytes(destination, scannerBuffer.c_str(), scannerBuffer.size());
                const int writtenCrLf = uart_write_bytes(destination, "\r\n", 2);
                if (writtenPayload > 0) {
                    forwardedBytes += static_cast<size_t>(writtenPayload);
                }
                if (writtenCrLf > 0) {
                    forwardedBytes += static_cast<size_t>(writtenCrLf);
                }
                forwardedLines++;
                scannerBuffer.clear();
            }
        }
    }

    ESP_LOGD(TAG, "[C] Forwarding scanner data end (lines=%u bytes=%u pending=%u)",
             static_cast<unsigned>(forwardedLines),
             static_cast<unsigned>(forwardedBytes),
             static_cast<unsigned>(scannerBuffer.size()));
}
