#include "pin_config.h"
#include "scanner_control.h"

#include <driver/gpio.h>
#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cstdio>
#include <string>
#include <unistd.h>

static const char *TAG = "ScannerControl";
static bool scannerInitialized = false;
static std::string scannerBuffer;

#if defined(SCANNER_CONTROL_USE_SERIAL)
static const uint8_t START_SCAN_CMD[] = {0x01, 'T', 0x04};
static const uint8_t STOP_SCAN_CMD[] = {0x01, 'P', 0x04};
static const uint8_t SCANNER_ENABLE_ILLUM_CMD[] = {'@', 'E', 'I', 'L', 'S', 'C', 'N', '2'};
static const uint8_t SCANNER_DISABLE_ILLUM_CMD[] = {'@', 'E', 'I', 'L', 'S', 'C', 'N', '0'};
static const uint8_t SCANNER_PROTOCOL_PREFIX[] = {0x7E, 0x01, '0', '0', '0', '0'};
static const uint8_t SCANNER_PROTOCOL_SUFFIX[] = {';', 0x03};
static constexpr int SCANNER_TTL_BAUD_RATE = 9600;

#ifndef SCANNER_LISTEN_ONLY_MODE_ENABLE
#define SCANNER_LISTEN_ONLY_MODE_ENABLE 1
#endif
#endif

static void configureOutputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "[C] gpio_config failed pin=%d err=0x%02X", static_cast<int>(pin), err);
    }
    else
    {
        ESP_LOGD(TAG, "[C] GPIO output configured pin=%d", static_cast<int>(pin));
    }
}

static void configureInputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "[C] gpio_config input failed pin=%d err=0x%02X", static_cast<int>(pin), err);
    }
    else
    {
        ESP_LOGD(TAG, "[C] GPIO input configured pin=%d", static_cast<int>(pin));
    }
}

#if defined(SCANNER_CONTROL_USE_TRIGGER)
static void configureScannerControlPinsTriggerMode()
{
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG), 1);
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 1);
}
#endif

void setupScannerPins()
{
    ESP_LOGI(TAG, "[C] setupScannerPins start");
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    configureScannerControlPinsTriggerMode();
#else
    // In serial mode keep scanner out of reset by actively driving RESET high.
    configureOutputPin(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG));
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCRES_CFG), 1);
    configureInputPin(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG));
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

void initializeScanner()
{
    if (scannerInitialized)
    {
        ESP_LOGD(TAG, "[C] initializeScanner skipped: already initialized");
        return;
    }
    ESP_LOGI(TAG, "[C] initializeScanner start");
    scannerInitialized = true;
#if defined(SCANNER_CONTROL_USE_TRIGGER)
    configureScannerControlPinsTriggerMode();
#endif
    vTaskDelay(pdMS_TO_TICKS(50));
    ESP_LOGI(TAG, "[C] Scanner control pins configured");
    ESP_LOGI(TAG, "[C] Scanner initialization phase complete");
}

#if defined(SCANNER_CONTROL_USE_SERIAL)
static std::string bytesToHex(const uint8_t *data, size_t len)
{
    static constexpr char kHex[] = "0123456789ABCDEF";
    std::string out;
    if (data == nullptr || len == 0)
    {
        return out;
    }
    out.reserve((len * 3) - 1);
    for (size_t i = 0; i < len; ++i)
    {
        if (i > 0)
        {
            out.push_back(' ');
        }
        const uint8_t value = data[i];
        out.push_back(kHex[(value >> 4) & 0x0F]);
        out.push_back(kHex[value & 0x0F]);
    }
    return out;
}

static void logRxChunk(const uint8_t *data, size_t len, const char *contextTag)
{
    if (len == 0)
    {
        return;
    }

    std::string ascii;
    ascii.reserve(len);
    for (size_t i = 0; i < len; ++i)
    {
        const uint8_t c = data[i];
        if (c >= 32 && c <= 126)
        {
            ascii.push_back(static_cast<char>(c));
        }
        else if (c == '\r' || c == '\n' || c == '\t')
        {
            ascii.push_back(' ');
        }
        else
        {
            ascii.push_back('.');
        }
    }

    const std::string hex = bytesToHex(data, len);
    ESP_LOGI(TAG, "[C] SCN RX %s len=%u hex=[%s] ascii=[%s]",
             contextTag,
             static_cast<unsigned>(len),
             hex.c_str(),
             ascii.c_str());
}

static bool writeScannerCommand(uart_port_t scannerPort, const uint8_t *cmd, size_t cmdLen, const char *label)
{
    if (cmd == nullptr || cmdLen == 0)
    {
        ESP_LOGW(TAG, "[C] SCN TX %s skipped: empty command", label);
        return false;
    }

    const std::string hex = bytesToHex(cmd, cmdLen);
    ESP_LOGI(TAG, "[C] SCN TX %s uart=%d len=%u hex=[%s]",
             label,
             static_cast<int>(scannerPort),
             static_cast<unsigned>(cmdLen),
             hex.c_str());

    const int written = uart_write_bytes(scannerPort, reinterpret_cast<const char *>(cmd), cmdLen);
    if (written != static_cast<int>(cmdLen))
    {
        ESP_LOGW(TAG, "[C] SCN TX %s partial write=%d expected=%u", label, written, static_cast<unsigned>(cmdLen));
        return false;
    }
    uart_wait_tx_done(scannerPort, pdMS_TO_TICKS(100));
    return true;
}

static bool writeScannerProtocolCommand(uart_port_t scannerPort, const uint8_t *payload, size_t payloadLen, const char *label)
{
    if (payload == nullptr || payloadLen == 0)
    {
        ESP_LOGW(TAG, "[C] SCN TX %s skipped: empty payload", label);
        return false;
    }

    constexpr size_t kMaxFrameLen = 64;
    uint8_t frame[kMaxFrameLen] = {};
    const size_t frameLen = sizeof(SCANNER_PROTOCOL_PREFIX) + payloadLen + sizeof(SCANNER_PROTOCOL_SUFFIX);
    if (frameLen > kMaxFrameLen)
    {
        ESP_LOGW(TAG, "[C] SCN TX %s skipped: frame too long=%u", label, static_cast<unsigned>(frameLen));
        return false;
    }

    size_t idx = 0;
    memcpy(frame + idx, SCANNER_PROTOCOL_PREFIX, sizeof(SCANNER_PROTOCOL_PREFIX));
    idx += sizeof(SCANNER_PROTOCOL_PREFIX);
    memcpy(frame + idx, payload, payloadLen);
    idx += payloadLen;
    memcpy(frame + idx, SCANNER_PROTOCOL_SUFFIX, sizeof(SCANNER_PROTOCOL_SUFFIX));
    idx += sizeof(SCANNER_PROTOCOL_SUFFIX);

    return writeScannerCommand(scannerPort, frame, idx, label);
}

static size_t readScannerResponse(uart_port_t scannerPort, TickType_t timeoutTicks, const char *contextTag)
{
    uint8_t rxBuf[128];
    size_t total = 0;
    TickType_t deadline = xTaskGetTickCount() + timeoutTicks;
    while (xTaskGetTickCount() < deadline)
    {
        const TickType_t now = xTaskGetTickCount();
        const TickType_t remaining = deadline > now ? (deadline - now) : 0;
        const TickType_t readTimeout = remaining > pdMS_TO_TICKS(50) ? pdMS_TO_TICKS(50) : remaining;
        int readLen = uart_read_bytes(scannerPort, rxBuf, sizeof(rxBuf), readTimeout);
        if (readLen > 0)
        {
            total += static_cast<size_t>(readLen);
            logRxChunk(rxBuf, static_cast<size_t>(readLen), contextTag);
        }
    }
    return total;
}

void scannerOn(uart_port_t scannerPort)
{
    initializeScanner();
    writeScannerCommand(scannerPort, START_SCAN_CMD, sizeof(START_SCAN_CMD), "START_SCAN");
}

void scannerOff(uart_port_t scannerPort)
{
    initializeScanner();
    writeScannerCommand(scannerPort, STOP_SCAN_CMD, sizeof(STOP_SCAN_CMD), "STOP_SCAN");
}

void scannerSerialSelfTest(uart_port_t scannerPort)
{
    initializeScanner();
    ESP_LOGI(TAG, "[C] SCN self-test start (uart=%d, %d 8N1)", static_cast<int>(scannerPort), SCANNER_TTL_BAUD_RATE);

    const esp_err_t baudErr = uart_set_baudrate(scannerPort, SCANNER_TTL_BAUD_RATE);
    if (baudErr != ESP_OK)
    {
        ESP_LOGW(TAG, "[C] SCN baud set failed uart=%d baud=%d err=0x%02X",
                 static_cast<int>(scannerPort),
                 SCANNER_TTL_BAUD_RATE,
                 baudErr);
    }

#if SCANNER_LISTEN_ONLY_MODE_ENABLE
    ESP_LOGW(TAG, "[C] SCN listen-only mode enabled: no TX commands, log HEX realtime (no CR wait)");
    while (true)
    {
        uint8_t c = 0;
        const int readLen = uart_read_bytes(scannerPort, &c, 1, pdMS_TO_TICKS(100));
        if (readLen <= 0)
        {
            continue;
        }

        char ascii = '.';
        if (c >= 32 && c <= 126)
        {
            ascii = static_cast<char>(c);
        }
        else if (c == '\r' || c == '\n' || c == '\t')
        {
            ascii = ' ';
        }

        const std::string hex = bytesToHex(&c, 1);
        ESP_LOGI(TAG, "[C] SCN RX BYTE hex=[%s] ascii=[%c] dec=%u",
                 hex.c_str(),
                 ascii,
                 static_cast<unsigned>(c));
    }
#else
    while (true)
    {
        uart_flush_input(scannerPort);
        writeScannerProtocolCommand(scannerPort, SCANNER_ENABLE_ILLUM_CMD, sizeof(SCANNER_ENABLE_ILLUM_CMD), "EILSCN2");
        size_t responseBytes = readScannerResponse(scannerPort, pdMS_TO_TICKS(200), "EILSCN2");
        if (responseBytes == 0)
        {
            ESP_LOGW(TAG, "[C] SCN RX EILSCN2 no response");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));

        uart_flush_input(scannerPort);
        writeScannerProtocolCommand(scannerPort, SCANNER_DISABLE_ILLUM_CMD, sizeof(SCANNER_DISABLE_ILLUM_CMD), "EILSCN0");
        responseBytes = readScannerResponse(scannerPort, pdMS_TO_TICKS(200), "EILSCN0");
        if (responseBytes == 0)
        {
            ESP_LOGW(TAG, "[C] SCN RX EILSCN0 no response");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
#endif
}
#else
void scannerOn()
{
    initializeScanner();
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 0);
    ESP_LOGI(TAG, "[C] Scanner ON via trigger (pin=%d level=0)", PIN_QR_BCTRIG_CFG);
}

void scannerOff()
{
    initializeScanner();
    gpio_set_level(static_cast<gpio_num_t>(PIN_QR_BCTRIG_CFG), 1);
    ESP_LOGI(TAG, "[C] Scanner OFF via trigger (pin=%d level=1)", PIN_QR_BCTRIG_CFG);
}
#endif

void forwardScannerData(uart_port_t source, uart_port_t destination)
{
    size_t bufferedLen = 0;
    esp_err_t err = uart_get_buffered_data_len(source, &bufferedLen);
    if (err != ESP_OK) {
        static bool uart_error_reported = false;
        if (!uart_error_reported) {
            ESP_LOGW(TAG, "UART scanner forward skipped: uart_get_buffered_data_len failed (err=0x%02X)", err);
            uart_error_reported = true;
        }
        return;
    }
    if (bufferedLen == 0)
    {
        return;
    }
    ESP_LOGD(TAG, "[C] Forwarding scanner data start (src=%d dst=%d buffered=%u)",
             static_cast<int>(source),
             static_cast<int>(destination),
             static_cast<unsigned>(bufferedLen));

    size_t forwardedBytes = 0;
    size_t forwardedLines = 0;
    while (true)
    {
        uint8_t c;
        int len = uart_read_bytes(source, &c, 1, 0);
        if (len <= 0)
        {
            break;
        }
        if (c == '\r' || c == '\n')
        {
            if (!scannerBuffer.empty())
            {
                ESP_LOGI(TAG, "[C] Scanner line complete (len=%u): %s",
                         static_cast<unsigned>(scannerBuffer.size()),
                         scannerBuffer.c_str());
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
                const int writtenPayload = static_cast<int>(write(STDOUT_FILENO, scannerBuffer.c_str(), scannerBuffer.size()));
                const int writtenCrLf = static_cast<int>(write(STDOUT_FILENO, "\r\n", 2));
#else
                const int writtenPayload = uart_write_bytes(destination, scannerBuffer.c_str(), scannerBuffer.size());
                const int writtenCrLf = uart_write_bytes(destination, "\r\n", 2);
#endif
                if (writtenPayload > 0)
                {
                    forwardedBytes += static_cast<size_t>(writtenPayload);
                }
                if (writtenCrLf > 0)
                {
                    forwardedBytes += static_cast<size_t>(writtenCrLf);
                }
                forwardedLines++;
                scannerBuffer.clear();
            }
        }
        else
        {
            scannerBuffer.push_back(static_cast<char>(c));
            if (scannerBuffer.length() > 240)
            {
                ESP_LOGW(TAG, "[C] Scanner chunk flush (len=%u)", static_cast<unsigned>(scannerBuffer.size()));
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
                const int writtenPayload = static_cast<int>(write(STDOUT_FILENO, scannerBuffer.c_str(), scannerBuffer.size()));
                const int writtenCrLf = static_cast<int>(write(STDOUT_FILENO, "\r\n", 2));
#else
                const int writtenPayload = uart_write_bytes(destination, scannerBuffer.c_str(), scannerBuffer.size());
                const int writtenCrLf = uart_write_bytes(destination, "\r\n", 2);
#endif
                if (writtenPayload > 0)
                {
                    forwardedBytes += static_cast<size_t>(writtenPayload);
                }
                if (writtenCrLf > 0)
                {
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
