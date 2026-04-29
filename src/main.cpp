#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <esp_rom_sys.h>
#include <esp_spiffs.h>
#include <driver/spi_master.h>
#include <nvs_flash.h>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string>
#include <vector>

#ifndef ENABLE_DISPLAY_LVGL
#define ENABLE_DISPLAY_LVGL 1
#endif

#include "pin_config.h"
#include "display.h"
#include "GDEY0154D67.h"
#include "led_control.h"
#include "scanner_control.h"
#include "master_protocol.h"
#if ENABLE_DISPLAY_LVGL
#include "lv_conf.h"
#include <lvgl.h>
static lv_obj_t *display_label = nullptr;
#endif

#define LOG_TAG "EPaperQr"
#define _LOGI(...) ESP_LOGI(LOG_TAG, __VA_ARGS__)
#define _LOGD(...) ESP_LOGD(LOG_TAG, __VA_ARGS__)
#define _LOGE(...) ESP_LOGE(LOG_TAG, __VA_ARGS__)
#define _LOGW(...) ESP_LOGW(LOG_TAG, __VA_ARGS__)

#ifndef DISPLAY_UPDATE_INTERVAL_SEC
#define DISPLAY_UPDATE_INTERVAL_SEC 15
#endif

#ifndef SCANNER_SERIAL_SELF_TEST_ENABLE
#define SCANNER_SERIAL_SELF_TEST_ENABLE 0
#endif

#ifndef SCANNER_TX_SQUARE_WAVE_ENABLE
#define SCANNER_TX_SQUARE_WAVE_ENABLE 0
#endif

#ifndef SCANNER_UART_TX_DIAG_BURST_ENABLE
#define SCANNER_UART_TX_DIAG_BURST_ENABLE 0
#endif

#ifndef SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS
#define SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS 20
#endif

#ifndef SCANNER_TX_SQUARE_WAVE_FREQ_HZ
#define SCANNER_TX_SQUARE_WAVE_FREQ_HZ 10000
#endif

#define WIFI_SSID "FWAP02"
#define WIFI_PASSWORD "fwxi56cgo"

#if defined(USE_EPD_GDEY1085F51)
constexpr uint16_t EPD_WIDTH = 1360;
constexpr uint16_t EPD_HEIGHT = 480;
#else
constexpr uint16_t EPD_WIDTH = 200;
constexpr uint16_t EPD_HEIGHT = 200;
#endif

static const char *TAG = "EPaperQr";
static constexpr const char *kAppLastChangeDescription = "Scanner TTL 9600 listen-only: log realtime BYTE in HEX+ASCII senza attesa terminatore";
static constexpr const char *kAppLastChangeTimestamp = __TIMESTAMP__;
static const uart_port_t UART_MASTER = UART_NUM_0;
static bool gMasterUartReady = false;
static spi_device_handle_t epdSpiHandle = nullptr;
static bool epdSpiReady = false;
static bool epdInitialized = false;
static constexpr int EPD_BUSY_ACTIVE_LEVEL = 0;
static const uint32_t EPD_BUSY_TIMEOUT_MS = 15000;

static void feed_watchdog()
{
    vTaskDelay(pdMS_TO_TICKS(1));
}

static void configureOutputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

static void configureInputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << pin;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

static esp_err_t epdTransmit(const uint8_t *buffer, size_t size)
{
    if (epdSpiHandle == nullptr)
    {
        return ESP_ERR_INVALID_STATE;
    }
    spi_transaction_t transaction = {};
    transaction.length = static_cast<int>(size * 8);
    transaction.tx_buffer = buffer;
    return spi_device_transmit(epdSpiHandle, &transaction);
}

static void sendEpdCommand(uint8_t command)
{
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 0);
    epdTransmit(&command, 1);
}

static void sendEpdData(uint8_t data)
{
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 1);
    epdTransmit(&data, 1);
}

static void waitUntilIdle()
{
    if (PIN_EPD_BUSY_CFG >= 0)
    {
        const gpio_num_t busyPin = static_cast<gpio_num_t>(PIN_EPD_BUSY_CFG);
        _LOGD("[M] waitUntilIdle: BUSY pin=%d active_level=%d", busyPin, EPD_BUSY_ACTIVE_LEVEL);
        const TickType_t start = xTaskGetTickCount();
        while (gpio_get_level(busyPin) == EPD_BUSY_ACTIVE_LEVEL)
        {
            if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(EPD_BUSY_TIMEOUT_MS))
            {
                _LOGW("[M] waitUntilIdle: BUSY timeout");
                break;
            }
            _LOGD("[M] waitUntilIdle: BUSY level=%d", gpio_get_level(busyPin));
            feed_watchdog();
        }
        _LOGD("[M] waitUntilIdle: BUSY idle detected level=%d", gpio_get_level(busyPin));
    }
    else
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void epdReset()
{
    if (PIN_EPD_RST_CFG < 0)
    {
        _LOGW("EPD reset pin not configured");
        return;
    }
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_RST_CFG), 1);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_RST_CFG), 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_RST_CFG), 1);
    vTaskDelay(pdMS_TO_TICKS(200));
}

static esp_err_t initEpaperSpi()
{
    if (epdSpiReady)
    {
        return ESP_OK;
    }
    spi_bus_config_t busConfig = {};
    busConfig.mosi_io_num = PIN_SPI_MOSI_CFG;
    busConfig.miso_io_num = PIN_SPI_MISO_CFG >= 0 ? PIN_SPI_MISO_CFG : -1;
    busConfig.sclk_io_num = PIN_SPI_SCK_CFG;
    busConfig.quadhd_io_num = -1;
    busConfig.quadwp_io_num = -1;
    busConfig.max_transfer_sz = 5000;
    esp_err_t err = spi_bus_initialize(SPI2_HOST, &busConfig, SPI_DMA_CH_AUTO);
    if (err != ESP_OK)
    {
        return err;
    }
    spi_device_interface_config_t devConfig = {};
    devConfig.clock_speed_hz = 2000000;
    devConfig.mode = 0;
    devConfig.spics_io_num = PIN_EPD_CS_CFG;
    devConfig.queue_size = 1;
    devConfig.flags = SPI_DEVICE_HALFDUPLEX;
    err = spi_bus_add_device(SPI2_HOST, &devConfig, &epdSpiHandle);
    if (err == ESP_OK)
    {
        epdSpiReady = true;
    }
    return err;
}

static void epdPowerOn()
{
    if (PIN_EPD_PWR_CFG < 0)
    {
        return;
    }
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_PWR_CFG), 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_PWR_CFG), 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

static bool epdInitSequence()
{
    sendEpdCommand(0x12); // SWRESET
    waitUntilIdle();

    sendEpdCommand(0x4D);
    sendEpdData(0x78);

    sendEpdCommand(0x00);
    sendEpdData(0x0F);
    sendEpdData(0x29);

    sendEpdCommand(0x01);
    sendEpdData(0x07);
    sendEpdData(0x00);

    sendEpdCommand(0x03);
    sendEpdData(0x10);
    sendEpdData(0x54);
    sendEpdData(0x44);

    sendEpdCommand(0x06);
    sendEpdData(0x05);
    sendEpdData(0x00);
    sendEpdData(0x3F);
    sendEpdData(0x0A);
    sendEpdData(0x25);
    sendEpdData(0x12);
    sendEpdData(0x1A);

    sendEpdCommand(0x50);
    sendEpdData(0x37);

    sendEpdCommand(0x60);
    sendEpdData(0x02);
    sendEpdData(0x02);

    sendEpdCommand(0x61);
    sendEpdData(static_cast<uint8_t>(EPD_WIDTH / 256));
    sendEpdData(static_cast<uint8_t>(EPD_WIDTH % 256));
    sendEpdData(static_cast<uint8_t>(EPD_HEIGHT / 256));
    sendEpdData(static_cast<uint8_t>(EPD_HEIGHT % 256));

    sendEpdCommand(0xE7);
    sendEpdData(0x1C);

    sendEpdCommand(0xE3);
    sendEpdData(0x22);

    sendEpdCommand(0xB4);
    sendEpdData(0xD0);
    sendEpdCommand(0xB5);
    sendEpdData(0x03);

    sendEpdCommand(0xE9);
    sendEpdData(0x01);

    sendEpdCommand(0x30);
    sendEpdData(0x08);

    sendEpdCommand(0x04);
    waitUntilIdle();

    sendEpdCommand(0x01); // Driver output control
    sendEpdData(0xC7);
    sendEpdData(0x00);
    sendEpdData(0x01);

    sendEpdCommand(0x11); // Data entry mode
    sendEpdData(0x01);

    sendEpdCommand(0x44); // Ram X address start/end
    sendEpdData(0x00);
    sendEpdData(0x18);

    sendEpdCommand(0x45); // Ram Y address start/end
    sendEpdData(0xC7);
    sendEpdData(0x00);
    sendEpdData(0x00);
    sendEpdData(0x00);

    sendEpdCommand(0x3C); // Border waveform
    sendEpdData(0x05);

    sendEpdCommand(0x18); // Temperature sensor
    sendEpdData(0x80);

    sendEpdCommand(0x4E); // Set RAM X address
    sendEpdData(0x00);

    sendEpdCommand(0x4F); // Set RAM Y address
    sendEpdData(0xC7);
    sendEpdData(0x00);

    waitUntilIdle();
    return true;
}

static void setEpdRamArea()
{
    sendEpdCommand(0x44);
    sendEpdData(0x00);
    sendEpdData(0x18);

    sendEpdCommand(0x45);
    sendEpdData(0xC7);
    sendEpdData(0x00);
    sendEpdData(0x00);
    sendEpdData(0x00);
}

static void setEpdRamPointer(uint8_t x, uint16_t y)
{
    sendEpdCommand(0x4E);
    sendEpdData(x);
    sendEpdCommand(0x4F);
    sendEpdData(static_cast<uint8_t>(y & 0xFF));
    sendEpdData(static_cast<uint8_t>((y >> 8) & 0xFF));
}

static void writeEpdRam(uint8_t command, uint8_t value)
{
    setEpdRamArea();
    setEpdRamPointer(0x00, 0x0000);
    sendEpdCommand(command);
    const size_t length = (EPD_WIDTH * EPD_HEIGHT) / 8;
    for (size_t i = 0; i < length; ++i)
    {
        sendEpdData(value);
    }
}

static void epdRefresh()
{
    sendEpdCommand(0x22);
    sendEpdData(0xF7);
    sendEpdCommand(0x20);
    waitUntilIdle();
}

static void epdClearScreen()
{
    sendEpdCommand(0x11); // data entry mode
    sendEpdData(0x01);
    writeEpdRam(0x24, 0xFF);
    writeEpdRam(0x26, 0x00);
    epdRefresh();
}

static void epdFullRefresh()
{
    GDEY0154D67_refresh();
}

static void setEpdPartialWindow(const lv_area_t *area)
{
    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 >= static_cast<int>(EPD_WIDTH))
        x2 = static_cast<int>(EPD_WIDTH) - 1;
    if (y2 >= static_cast<int>(EPD_HEIGHT))
        y2 = static_cast<int>(EPD_HEIGHT) - 1;
    if (x2 < x1 || y2 < y1)
    {
        return;
    }

    const uint8_t xStart = static_cast<uint8_t>(x1 / 8);
    const uint8_t xEnd = static_cast<uint8_t>((x2 + 8) / 8 - 1);

    sendEpdCommand(0x44);
    sendEpdData(xStart);
    sendEpdData(xEnd);

    sendEpdCommand(0x45);
    sendEpdData(static_cast<uint8_t>(y1 & 0xFF));
    sendEpdData(static_cast<uint8_t>((y1 >> 8) & 0xFF));
    sendEpdData(static_cast<uint8_t>(y2 & 0xFF));
    sendEpdData(static_cast<uint8_t>((y2 >> 8) & 0xFF));

    sendEpdCommand(0x4E);
    sendEpdData(xStart);
    sendEpdCommand(0x4F);
    sendEpdData(static_cast<uint8_t>(y1 & 0xFF));
    sendEpdData(static_cast<uint8_t>((y1 >> 8) & 0xFF));
}

static void sendEpdPartialImage(const lv_area_t *area, lv_color_t *color_p)
{
    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 >= static_cast<int>(EPD_WIDTH))
        x2 = static_cast<int>(EPD_WIDTH) - 1;
    if (y2 >= static_cast<int>(EPD_HEIGHT))
        y2 = static_cast<int>(EPD_HEIGHT) - 1;
    if (x2 < x1 || y2 < y1)
    {
        return;
    }

    const uint8_t xStart = static_cast<uint8_t>(x1 / 8);
    const uint8_t xEnd = static_cast<uint8_t>((x2 + 8) / 8 - 1);
    const int widthBytes = xEnd - xStart + 1;
    const int widthPixels = x2 - x1 + 1;
    const int height = y2 - y1 + 1;

    setEpdPartialWindow(area);

    sendEpdCommand(0x24);
    for (int row = 0; row < height; ++row)
    {
        for (int byteIndex = 0; byteIndex < widthBytes; ++byteIndex)
        {
            uint8_t data = 0x00;
            for (int bit = 0; bit < 8; ++bit)
            {
                const int pixelX = (xStart + byteIndex) * 8 + bit;
                const int localX = pixelX - x1;
                const uint8_t white = (localX < 0 || localX >= widthPixels) ? 1 : lv_color_to1(color_p[row * widthPixels + localX]);
                if (white)
                {
                    data |= static_cast<uint8_t>(0x80 >> bit);
                }
            }
            sendEpdData(data);
        }
    }

    sendEpdCommand(0x26);
    for (int row = 0; row < height; ++row)
    {
        for (int byteIndex = 0; byteIndex < widthBytes; ++byteIndex)
        {
            sendEpdData(0x00);
        }
    }

    epdRefresh();
}

static void epdBlackScreen()
{
    GDEY0154D67_black_screen();
}

static void initBootButton()
{
    if (PIN_BOOT_BUTTON_CFG < 0)
    {
        return;
    }
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << static_cast<uint8_t>(PIN_BOOT_BUTTON_CFG);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

static void lvglDelayAndRefresh(TickType_t delayTicks)
{
#if ENABLE_DISPLAY_LVGL
    const TickType_t end = xTaskGetTickCount() + delayTicks;
    while (xTaskGetTickCount() < end)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
#else
    vTaskDelay(delayTicks);
#endif
}

static void waitForBootButtonPress()
{
    ESP_LOGI(TAG, "[M] waiting 10 seconds before next display step");
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));
}

static void performDisplayTestSequence()
{
    if (!GDEY0154D67_is_initialized())
    {
        ESP_LOGW(TAG, "[M] display test skipped: epaper not initialized");
        return;
    }
    initBootButton();
    while (1)
    {
        ESP_LOGI(TAG, "[M] display test 1/5: full refresh current display");
        epdFullRefresh();
        ESP_LOGI(TAG, "[M] display test paused, waiting 10 seconds");
        waitForBootButtonPress();

        ESP_LOGI(TAG, "[M] display test 2/5: clean screen");
        clearDisplay();
        ESP_LOGI(TAG, "[M] display test paused, waiting 10 seconds");
        waitForBootButtonPress();

        ESP_LOGI(TAG, "[M] display test 3/5: full black screen");
        epdBlackScreen();
        ESP_LOGI(TAG, "[M] display test paused, waiting 10 seconds");
        waitForBootButtonPress();

        ESP_LOGI(TAG, "[M] display test 4/5: show text '123'");
#if ENABLE_DISPLAY_LVGL
        if (display_label)
        {
            lv_obj_set_style_text_font(display_label, &lv_font_montserrat_28, LV_PART_MAIN);
            lv_label_set_text(display_label, "123");
            lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
        }
#endif
        ESP_LOGI(TAG, "[M] display test paused, waiting 10 seconds");
        waitForBootButtonPress();

        ESP_LOGI(TAG, "[M] display test 5/5: show logo");
#if ENABLE_DISPLAY_LVGL
        displayJpegCentered("/spiffs/logo_n.jpg");
#endif
        ESP_LOGI(TAG, "[M] display test complete");
    }
}

#if defined(SCANNER_CONTROL_USE_SERIAL)
static const uart_port_t UART_SCANNER = UART_NUM_1;
static bool gScannerUartReady = false;
#endif
static httpd_handle_t otaServer = nullptr;

#if defined(SCANNER_CONTROL_USE_SERIAL) && SCANNER_TX_SQUARE_WAVE_ENABLE
static esp_timer_handle_t scannerTxWaveTimer = nullptr;
static bool scannerTxWaveLevel = false;

static void scannerTxWaveCallback(void *arg)
{
    (void)arg;
    scannerTxWaveLevel = !scannerTxWaveLevel;
    gpio_set_level(static_cast<gpio_num_t>(PIN_SCANNER_TX_CFG), scannerTxWaveLevel ? 1 : 0);
}

static void startScannerTxSquareWave()
{
    if (scannerTxWaveTimer != nullptr)
    {
        return;
    }

    gpio_config_t ioConf = {};
    ioConf.intr_type = GPIO_INTR_DISABLE;
    ioConf.mode = GPIO_MODE_OUTPUT;
    ioConf.pin_bit_mask = 1ULL << static_cast<gpio_num_t>(PIN_SCANNER_TX_CFG);
    ioConf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ioConf.pull_up_en = GPIO_PULLUP_DISABLE;
    esp_err_t err = gpio_config(&ioConf);
    if (err != ESP_OK)
    {
        _LOGE("[M] scanner TX wave gpio_config failed: 0x%02X", err);
        return;
    }

    const esp_timer_create_args_t timerArgs = {
        .callback = &scannerTxWaveCallback,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "scn_tx_wave",
        .skip_unhandled_events = false,
    };
    err = esp_timer_create(&timerArgs, &scannerTxWaveTimer);
    if (err != ESP_OK)
    {
        _LOGE("[M] scanner TX wave timer create failed: 0x%02X", err);
        scannerTxWaveTimer = nullptr;
        return;
    }

    const int64_t halfPeriodUs = 1000000LL / (2LL * SCANNER_TX_SQUARE_WAVE_FREQ_HZ);
    err = esp_timer_start_periodic(scannerTxWaveTimer, halfPeriodUs > 0 ? halfPeriodUs : 1);
    if (err != ESP_OK)
    {
        _LOGE("[M] scanner TX wave start failed: 0x%02X", err);
        esp_timer_delete(scannerTxWaveTimer);
        scannerTxWaveTimer = nullptr;
        return;
    }

    _LOGW("[M] scanner TX wave active on pin=%d freq=%dHz", PIN_SCANNER_TX_CFG, SCANNER_TX_SQUARE_WAVE_FREQ_HZ);
}
#endif

static const char *resetReasonToString(esp_reset_reason_t reason)
{
    switch (reason)
    {
    case ESP_RST_UNKNOWN:
        return "UNKNOWN";
    case ESP_RST_POWERON:
        return "POWERON";
    case ESP_RST_EXT:
        return "EXT";
    case ESP_RST_SW:
        return "SW";
    case ESP_RST_PANIC:
        return "PANIC";
    case ESP_RST_INT_WDT:
        return "INT_WDT";
    case ESP_RST_TASK_WDT:
        return "TASK_WDT";
    case ESP_RST_WDT:
        return "WDT";
    case ESP_RST_DEEPSLEEP:
        return "DEEPSLEEP";
    case ESP_RST_BROWNOUT:
        return "BROWNOUT";
    case ESP_RST_SDIO:
        return "SDIO";
    case ESP_RST_USB:
        return "USB";
    case ESP_RST_JTAG:
        return "JTAG";
    case ESP_RST_EFUSE:
        return "EFUSE";
    case ESP_RST_PWR_GLITCH:
        return "PWR_GLITCH";
    case ESP_RST_CPU_LOCKUP:
        return "CPU_LOCKUP";
    default:
        return "UNMAPPED";
    }
}

static const esp_partition_t *selectOtaTargetPartition()
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *boot = esp_ota_get_boot_partition();
    if (running != nullptr)
    {
        _LOGI("[M] OTA running partition: %s subtype=0x%02X addr=0x%08lX size=0x%08lX",
              running->label,
              running->subtype,
              static_cast<unsigned long>(running->address),
              static_cast<unsigned long>(running->size));
    }
    if (boot != nullptr)
    {
        _LOGI("[M] OTA boot partition: %s subtype=0x%02X addr=0x%08lX size=0x%08lX",
              boot->label,
              boot->subtype,
              static_cast<unsigned long>(boot->address),
              static_cast<unsigned long>(boot->size));
    }

    const esp_partition_t *nextUpdate = esp_ota_get_next_update_partition(nullptr);
    if (nextUpdate != nullptr)
    {
        return nextUpdate;
    }

    _LOGW("[M] OTA next update partition unavailable, scanning app partitions");
    const esp_partition_t *fallback = nullptr;
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, nullptr);
    while (it != nullptr)
    {
        const esp_partition_t *part = esp_partition_get(it);
        if (part != nullptr)
        {
            _LOGI("[M] OTA app partition found: %s subtype=0x%02X addr=0x%08lX size=0x%08lX",
                  part->label,
                  part->subtype,
                  static_cast<unsigned long>(part->address),
                  static_cast<unsigned long>(part->size));

            const bool isOta = part->subtype >= ESP_PARTITION_SUBTYPE_APP_OTA_MIN &&
                               part->subtype < ESP_PARTITION_SUBTYPE_APP_OTA_MAX;
            const bool isFactory = part->subtype == ESP_PARTITION_SUBTYPE_APP_FACTORY;
            const bool isDifferentFromRunning = running == nullptr || part->address != running->address;
            if (fallback == nullptr && isDifferentFromRunning && (isOta || isFactory))
            {
                fallback = part;
            }
        }
        it = esp_partition_next(it);
    }
    return fallback;
}

#if defined(SCANNER_CONTROL_USE_SERIAL) && SCANNER_UART_TX_DIAG_BURST_ENABLE
static void runScannerUartTxBurstDiagnostic(uart_port_t scannerPort)
{
    static const uint8_t kPattern[] = {0x55, 0xAA, 0x55, 0xAA, 0x0D, 0x0A};
    ESP_LOGW(TAG, "[M] SCN TX diag burst attivo: uart=%d tx_pin=%d interval=%dms pattern_len=%u",
             static_cast<int>(scannerPort),
             PIN_SCANNER_TX_CFG,
             SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS,
             static_cast<unsigned>(sizeof(kPattern)));

    while (true)
    {
        const int written = uart_write_bytes(scannerPort, reinterpret_cast<const char *>(kPattern), sizeof(kPattern));
        if (written != static_cast<int>(sizeof(kPattern)))
        {
            ESP_LOGW(TAG, "[M] SCN TX diag burst write parziale=%d expected=%u",
                     written,
                     static_cast<unsigned>(sizeof(kPattern)));
        }
        uart_wait_tx_done(scannerPort, pdMS_TO_TICKS(20));
        vTaskDelay(pdMS_TO_TICKS(SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS));
    }
}
#endif

static void otaRebootTask(void *arg)
{
    (void)arg;
    _LOGW("[M] OTA success: reboot in 1s");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}

static esp_err_t otaUploadHandler(httpd_req_t *req)
{
    const esp_partition_t *updatePartition = selectOtaTargetPartition();
    if (updatePartition == nullptr)
    {
        _LOGE("[M] OTA failed: no update partition available (check partition table and flash size)");
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "NO_UPDATE_PARTITION");
        return ESP_FAIL;
    }

    esp_ota_handle_t otaHandle = 0;
    esp_err_t err = esp_ota_begin(updatePartition, OTA_SIZE_UNKNOWN, &otaHandle);
    if (err != ESP_OK)
    {
        _LOGE("[M] OTA begin failed: 0x%02X", err);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "OTA_BEGIN_FAILED");
        return ESP_FAIL;
    }

    _LOGI("[M] OTA upload started: size=%d target=%s", req->content_len, updatePartition->label);

    int remaining = req->content_len;
    static uint8_t otaBuffer[1024];
    while (remaining > 0)
    {
        const int toRead = remaining > static_cast<int>(sizeof(otaBuffer)) ? static_cast<int>(sizeof(otaBuffer)) : remaining;
        const int received = httpd_req_recv(req, reinterpret_cast<char *>(otaBuffer), toRead);
        if (received <= 0)
        {
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                continue;
            }
            _LOGE("[M] OTA recv failed: %d", received);
            esp_ota_abort(otaHandle);
            httpd_resp_set_status(req, "500 Internal Server Error");
            httpd_resp_sendstr(req, "OTA_RECV_FAILED");
            return ESP_FAIL;
        }

        err = esp_ota_write(otaHandle, otaBuffer, received);
        if (err != ESP_OK)
        {
            _LOGE("[M] OTA write failed: 0x%02X", err);
            esp_ota_abort(otaHandle);
            httpd_resp_set_status(req, "500 Internal Server Error");
            httpd_resp_sendstr(req, "OTA_WRITE_FAILED");
            return ESP_FAIL;
        }
        remaining -= received;
    }

    err = esp_ota_end(otaHandle);
    if (err != ESP_OK)
    {
        _LOGE("[M] OTA end failed: 0x%02X", err);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "OTA_END_FAILED");
        return ESP_FAIL;
    }

    err = esp_ota_set_boot_partition(updatePartition);
    if (err != ESP_OK)
    {
        _LOGE("[M] OTA set boot partition failed: 0x%02X", err);
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_sendstr(req, "OTA_SET_BOOT_FAILED");
        return ESP_FAIL;
    }

    _LOGI("[M] OTA upload complete: next boot partition=%s", updatePartition->label);
    httpd_resp_set_status(req, "200 OK");
    httpd_resp_sendstr(req, "OK");
    xTaskCreate(otaRebootTask, "ota_reboot", 2048, nullptr, 5, nullptr);
    return ESP_OK;
}

static void startOtaHttpServer()
{
    if (otaServer != nullptr)
    {
        return;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    esp_err_t err = httpd_start(&otaServer, &config);
    if (err != ESP_OK)
    {
        _LOGE("[M] OTA server start failed: 0x%02X", err);
        otaServer = nullptr;
        return;
    }

    httpd_uri_t uploadUri = {};
    uploadUri.uri = "/ota/upload";
    uploadUri.method = HTTP_POST;
    uploadUri.handler = otaUploadHandler;
    uploadUri.user_ctx = nullptr;

    err = httpd_register_uri_handler(otaServer, &uploadUri);
    if (err != ESP_OK)
    {
        _LOGE("[M] OTA URI register failed: 0x%02X", err);
        httpd_stop(otaServer);
        otaServer = nullptr;
        return;
    }

    _LOGI("[M] OTA HTTP server ready on port %d endpoint %s", config.server_port, uploadUri.uri);
}

// -----------------------------------------------------------------------------
// Sezione 1: Logica generale con FreeRTOS
// -----------------------------------------------------------------------------

#if ENABLE_DISPLAY_LVGL
struct FontDefinition
{
    const lv_font_t *font;
    uint8_t maxCharsPerLine;
    uint8_t maxLines;
    const char *name;
};

static const FontDefinition fontDefinitions[] = {
    {&lv_font_montserrat_48, 2, 2, "montserrat_48"},
    {&lv_font_montserrat_48, 3, 3, "montserrat_48"},
    {&lv_font_montserrat_28, 5, 4, "montserrat_28"},
    {&lv_font_montserrat_28, 10, 8, "montserrat_28"},
    {&lv_font_montserrat_14, 12, 12, "montserrat_14"},
    {&lv_font_montserrat_14, 18, 15, "montserrat_14"},
};

static const uint8_t kFontCount = sizeof(fontDefinitions) / sizeof(fontDefinitions[0]);

#endif

static void wifiEventHandler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        _LOGI("WiFi station started, connecting to %s", WIFI_SSID);
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *event = static_cast<wifi_event_sta_disconnected_t *>(event_data);
        _LOGW("WiFi disconnected, reason=%d. Reconnecting...", event->reason);
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
        char ipstr[16];
        esp_ip4addr_ntoa(&event->ip_info.ip, ipstr, sizeof(ipstr));
        _LOGI("WiFi connected, IP=%s", ipstr);
        startOtaHttpServer();
    }
}

static bool initNvsStorage()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        _LOGW("[M] NVS init issue (0x%02X), erasing NVS partition", err);
        err = nvs_flash_erase();
        if (err != ESP_OK)
        {
            _LOGE("[M] nvs_flash_erase failed: 0x%02X", err);
            return false;
        }
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        _LOGE("[M] nvs_flash_init failed: 0x%02X", err);
        return false;
    }

    return true;
}

static void initWifi()
{
    if (!initNvsStorage())
    {
        _LOGE("[M] WiFi init aborted: NVS unavailable");
        return;
    }

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_err_t err = esp_wifi_init(&cfg);
    if (err != ESP_OK)
    {
        _LOGE("esp_wifi_init failed: 0x%02X", err);
        return;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, nullptr, nullptr);
    if (err != ESP_OK)
    {
        _LOGE("WiFi event handler register failed: 0x%02X", err);
        return;
    }

    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, nullptr, nullptr);
    if (err != ESP_OK)
    {
        _LOGE("IP event handler register failed: 0x%02X", err);
        return;
    }

    wifi_config_t wifi_config = {};
    std::memcpy(wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    std::memcpy(wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK)
    {
        _LOGE("esp_wifi_set_mode failed: 0x%02X", err);
        return;
    }

    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK)
    {
        _LOGE("esp_wifi_set_config failed: 0x%02X", err);
        return;
    }

    err = esp_wifi_start();
    if (err != ESP_OK)
    {
        _LOGE("esp_wifi_start failed: 0x%02X", err);
    }
    else
    {
        _LOGI("WiFi initialization complete, waiting for connection...");
    }
}

static bool initUart(uart_port_t uart_num, int baud_rate, int tx_pin, int rx_pin)
{
    _LOGI("Initializing UART %d @ %d baud TX=%d RX=%d", static_cast<int>(uart_num), baud_rate, tx_pin, rx_pin);
    uart_config_t uart_config = {};
    uart_config.baud_rate = baud_rate;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    uart_config.rx_flow_ctrl_thresh = 0;
    uart_config.source_clk = UART_SCLK_APB;
    esp_err_t err = uart_param_config(uart_num, &uart_config);
    if (err != ESP_OK)
    {
        _LOGE("uart_param_config failed for UART %d: 0x%02X", static_cast<int>(uart_num), err);
        return false;
    }
    err = uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
    {
        _LOGE("uart_set_pin failed for UART %d: 0x%02X", static_cast<int>(uart_num), err);
        return false;
    }
    err = uart_driver_install(uart_num, 1024, 0, 0, nullptr, 0);
    if (err != ESP_OK)
    {
        _LOGE("uart_driver_install failed for UART %d: 0x%02X", static_cast<int>(uart_num), err);
        return false;
    }
    _LOGI("UART %d initialized successfully", static_cast<int>(uart_num));
    return true;
}

static bool mountSpiffs()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = "spiffs",
        .max_files = 10,
        .format_if_mount_failed = false};
    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if (err == ESP_OK)
    {
        _LOGI("[M] SPIFFS mounted at /spiffs");
        return true;
    }
    if (err == ESP_ERR_NOT_FOUND)
    {
        _LOGW("[M] SPIFFS partition not found (label=spiffs)");
    }
    else if (err == ESP_ERR_INVALID_STATE)
    {
        _LOGW("[M] SPIFFS already mounted");
        return true;
    }
    else if (err == ESP_ERR_INVALID_SIZE)
    {
        _LOGW("[M] SPIFFS partition size invalid");
    }
    else if (err == ESP_ERR_NOT_SUPPORTED)
    {
        _LOGW("[M] SPIFFS mount not supported");
    }
    else
    {
        _LOGW("[M] SPIFFS mount failed: 0x%02X", err);
    }
    return false;
}

static void logSpiffsFiles()
{
    DIR *dir = opendir("/spiffs");
    if (dir == nullptr)
    {
        _LOGW("[M] SPIFFS directory /spiffs not available");
        return;
    }
    struct dirent *entry;
    unsigned count = 0;
    _LOGI("[M] SPIFFS contents:");
    while ((entry = readdir(dir)) != nullptr)
    {
        _LOGI("[M]   %s", entry->d_name);
        ++count;
    }
    closedir(dir);
    _LOGI("[M] SPIFFS files count: %u", count);
}

static void initEpaperDriver()
{
    _LOGI("[M] Initializing Epaper hardware driver");
    GDEY0154D67_init();
    epdInitialized = GDEY0154D67_is_initialized();
}

#if ENABLE_DISPLAY_LVGL

static bool isNumericOnly(const std::string &text)
{
    for (char c : text)
    {
        if (c == '\r' || c == '\n' || c == ' ')
        {
            continue;
        }
        if (c < '0' || c > '9')
        {
            return false;
        }
    }
    return true;
}

static std::string normalizeText(const std::string &text)
{
    std::string normalized;
    normalized.reserve(text.size());
    for (char c : text)
    {
        if (c == '\r')
        {
            normalized.push_back('\n');
        }
        else if (c == '\t')
        {
            normalized.push_back(' ');
        }
        else
        {
            normalized.push_back(c);
        }
    }
    return normalized;
}

static std::vector<std::string> wrapText(const std::string &text, const FontDefinition &fontDef)
{
    std::vector<std::string> lines;
    std::string currentLine;
    std::string token;
    for (size_t i = 0; i <= text.length(); ++i)
    {
        char c = i < text.length() ? text[i] : ' ';
        if (c == '\n')
        {
            if (!token.empty())
            {
                if (!currentLine.empty())
                {
                    if (currentLine.length() + 1 + token.length() <= fontDef.maxCharsPerLine)
                    {
                        currentLine += ' ';
                        currentLine += token;
                    }
                    else
                    {
                        lines.push_back(currentLine);
                        currentLine = token;
                    }
                }
                else
                {
                    currentLine = token;
                }
                token.clear();
            }
            lines.push_back(currentLine);
            currentLine.clear();
        }
        else if (c == ' ' || i == text.length())
        {
            if (!token.empty())
            {
                if (!currentLine.empty())
                {
                    if (currentLine.length() + 1 + token.length() <= fontDef.maxCharsPerLine)
                    {
                        currentLine += ' ';
                        currentLine += token;
                    }
                    else
                    {
                        lines.push_back(currentLine);
                        currentLine = token;
                    }
                }
                else
                {
                    currentLine = token;
                }
                token.clear();
            }
            if (i == text.length())
            {
                break;
            }
        }
        else
        {
            token.push_back(c);
            if (token.length() > fontDef.maxCharsPerLine)
            {
                if (!currentLine.empty())
                {
                    lines.push_back(currentLine);
                    currentLine.clear();
                }
                while (token.length() > fontDef.maxCharsPerLine)
                {
                    lines.push_back(token.substr(0, fontDef.maxCharsPerLine));
                    token.erase(0, fontDef.maxCharsPerLine);
                }
                if (!token.empty())
                {
                    currentLine = token;
                    token.clear();
                }
            }
        }
        if (currentLine.length() > fontDef.maxCharsPerLine)
        {
            lines.push_back(currentLine.substr(0, fontDef.maxCharsPerLine));
            currentLine.erase(0, fontDef.maxCharsPerLine);
        }
    }
    if (!currentLine.empty() || lines.empty())
    {
        lines.push_back(currentLine);
    }
    return lines;
}

static std::string joinLines(const std::vector<std::string> &lines)
{
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        result += lines[i];
        if (i + 1 < lines.size())
        {
            result.push_back('\n');
        }
    }
    return result;
}

static bool fitsInFont(const std::string &text, const FontDefinition &fontDef)
{
    auto lines = wrapText(text, fontDef);
    return lines.size() <= fontDef.maxLines;
}

static std::string truncateText(const std::string &text, const FontDefinition &fontDef)
{
    auto lines = wrapText(text, fontDef);
    if (lines.size() <= fontDef.maxLines)
    {
        return joinLines(lines);
    }
    lines.resize(fontDef.maxLines);
    std::string &last = lines.back();
    if (last.length() > 4)
    {
        last = last.substr(0, last.length() - 4);
        last += " ...";
    }
    else
    {
        last = " ...";
    }
    return joinLines(lines);
}

static uint8_t selectFontIndex(const std::string &text)
{
    const std::string normalized = normalizeText(text);
    const bool numeric = isNumericOnly(normalized);
    std::vector<uint8_t> candidates;
    if (numeric)
    {
        candidates = {0, 1, 2, 3, 4, 5};
    }
    else
    {
        candidates = {3, 4, 5};
    }
    for (uint8_t index : candidates)
    {
        if (fitsInFont(normalized, fontDefinitions[index]))
        {
            return index;
        }
    }
    return candidates.back();
}

static uint8_t resolveExtendedFontIndex(uint8_t fontNumber)
{
    switch (fontNumber)
    {
    case 1:
        return 4;
    case 2:
        return 5;
    case 3:
        return 2;
    case 4:
        return 3;
    case 5:
        return 1;
    case 6:
        return 0;
    default:
        return 4;
    }
}

void clearDisplay()
{
    if (GDEY0154D67_is_initialized())
    {
        GDEY0154D67_clear_screen();
    }
    if (display_label)
    {
        lv_label_set_text(display_label, "");
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    }
}

void setDisplayOrientation(DisplayOrientation orientation)
{
    switch (orientation)
    {
        case DisplayOrientation::ORIENTATION_0:
            GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_0);
            break;
        case DisplayOrientation::ORIENTATION_90:
            GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_90);
            break;
        case DisplayOrientation::ORIENTATION_180:
            GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_180);
            break;
        case DisplayOrientation::ORIENTATION_270:
            GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_270);
            break;
        default:
            GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_0);
            break;
    }
}

void displayText(const std::string &raw_text)
{
    const std::string normalized = normalizeText(raw_text);
    const uint8_t fontIndex = selectFontIndex(normalized);
    const auto &fontDef = fontDefinitions[fontIndex];
    std::string output = fitsInFont(normalized, fontDef) ? joinLines(wrapText(normalized, fontDef)) : truncateText(normalized, fontDef);

    lv_obj_set_style_text_font(display_label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(display_label, output.c_str());
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    ESP_LOGI(TAG, "Display text using %s", fontDef.name);
}

void displayText(const std::string &raw_text, uint8_t fontNumber, uint8_t x, uint8_t y)
{
    const std::string normalized = normalizeText(raw_text);
    const uint8_t fontIndex = resolveExtendedFontIndex(fontNumber);
    const auto &fontDef = fontDefinitions[fontIndex];
    const std::string output = normalized;
    const int32_t width = static_cast<int32_t>(EPD_WIDTH) - x;
    lv_obj_set_style_text_font(display_label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(display_label, output.c_str());
    lv_obj_set_width(display_label, width > 0 ? width : 0);
    lv_obj_set_pos(display_label, x, y);
    ESP_LOGI(TAG, "Extended display text font#%u pos=(%u,%u) len=%u", fontNumber, x, y, static_cast<unsigned>(output.length()));
}

void displayJpegCentered(const char *path)
{
    if (path == nullptr)
    {
        ESP_LOGW(TAG, "displayJpegCentered called with null path");
        return;
    }

    lv_obj_t *screen = lv_scr_act();
    if (screen == nullptr)
    {
        ESP_LOGW(TAG, "displayJpegCentered failed: no active screen");
        return;
    }

    lv_obj_t *image = lv_img_create(screen);
    lv_img_set_src(image, path);
    lv_obj_center(image);
    ESP_LOGI(TAG, "displayJpegCentered loaded image: %s", path);
}

void setupDisplay()
{
    initEpaperDriver();
    GDEY0154D67_set_orientation(GDEY0154D67_Orientation::ORIENTATION_0);
    _LOGI("[M] Epaper driver enabled");
}

static void epd_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    if (GDEY0154D67_is_initialized() && area != nullptr && color_p != nullptr)
    {
        GDEY0154D67_draw_partial(area, color_p);
    }
    lv_disp_flush_ready(drv);
}

void setupLvgl()
{
    lv_init();
    static lv_color_t *lv_framebuffer = nullptr;
    static lv_disp_draw_buf_t *draw_buf = nullptr;
    static lv_disp_drv_t *disp_drv = nullptr;
    if (lv_framebuffer == nullptr)
    {
        lv_framebuffer = new lv_color_t[EPD_WIDTH * 40];
        if (lv_framebuffer == nullptr)
        {
            _LOGE("Failed to allocate LVGL framebuffer");
            return;
        }
    }
    if (draw_buf == nullptr)
    {
        draw_buf = new lv_disp_draw_buf_t;
        if (draw_buf == nullptr)
        {
            _LOGE("Failed to allocate LVGL draw buffer object");
            return;
        }
    }
    if (disp_drv == nullptr)
    {
        disp_drv = new lv_disp_drv_t;
        if (disp_drv == nullptr)
        {
            _LOGE("Failed to allocate LVGL display driver object");
            return;
        }
    }
    lv_disp_draw_buf_init(draw_buf, lv_framebuffer, nullptr, EPD_WIDTH * 40);
    lv_disp_drv_init(disp_drv);
    disp_drv->hor_res = EPD_WIDTH;
    disp_drv->ver_res = EPD_HEIGHT;
    disp_drv->flush_cb = epd_flush_cb;
    disp_drv->draw_buf = draw_buf;
    disp_drv->full_refresh = 0;
    lv_disp_drv_register(disp_drv);
}

void buildUi()
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    display_label = lv_label_create(screen);
    lv_obj_set_width(display_label, EPD_WIDTH);
    lv_label_set_long_mode(display_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(display_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(display_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_bg_color(display_label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(display_label, LV_OPA_COVER, LV_PART_MAIN);
    lv_label_set_text(display_label, "Ready");
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
}

#else

void clearDisplay()
{
    if (epdInitialized)
    {
        epdClearScreen();
    }
}

void displayText(const std::string &raw_text)
{
    (void)raw_text;
}

void displayText(const std::string &raw_text, uint8_t fontNumber, uint8_t x, uint8_t y)
{
    (void)raw_text;
    (void)fontNumber;
    (void)x;
    (void)y;
}

void setupDisplay()
{
    _LOGW("[M] Display/LVGL disabilitato da ENABLE_DISPLAY_LVGL");
}

void setupLvgl()
{
    _LOGW("[M] setupLvgl skipped: ENABLE_DISPLAY_LVGL=0");
}

void buildUi()
{
    _LOGW("[M] buildUi skipped: ENABLE_DISPLAY_LVGL=0");
}

#endif

extern "C" void app_main()
{
    const esp_reset_reason_t resetReason = esp_reset_reason();
    esp_rom_printf("\n[M] app_main entry, reset_reason=%d (%s)\n", static_cast<int>(resetReason), resetReasonToString(resetReason));
    ESP_LOGI(TAG, "[M] Last change: %s", kAppLastChangeDescription);
    ESP_LOGI(TAG, "[M] Last change timestamp: %s", kAppLastChangeTimestamp);
    ESP_LOGI(TAG, "[M] Starting EPaperQr FreeRTOS application");
    vTaskDelay(pdMS_TO_TICKS(300));

    ESP_LOGI(TAG, "[M] init step: SPIFFS");
    if (mountSpiffs())
    {
        logSpiffsFiles();
    }
#if ENABLE_DISPLAY_LVGL
    ESP_LOGI(TAG, "[M] init step: display");
    setupDisplay();
    ESP_LOGI(TAG, "[M] init step: LVGL");
    setupLvgl();
    ESP_LOGI(TAG, "[M] init step: UI");
    buildUi();
    ESP_LOGI(TAG, "[M] init step: display test sequence");
    performDisplayTestSequence();
#else
    ESP_LOGI(TAG, "[M] init step: display/LVGL skipped (ENABLE_DISPLAY_LVGL=0)");
#endif

    ESP_LOGI(TAG, "[M] init step: WiFi");
    initWifi();
    ESP_LOGI(TAG, "[M] init step: UART master");
    gMasterUartReady = initUart(UART_MASTER, 9600, PIN_TX_ESP_CFG, PIN_RX_ESP_CFG);
#if defined(SCANNER_CONTROL_USE_SERIAL)
    ESP_LOGI(TAG, "[M] init step: UART scanner");
    gScannerUartReady = initUart(UART_SCANNER, 9600, PIN_SCANNER_TX_CFG, PIN_SCANNER_RX_CFG);
#if SCANNER_SERIAL_SELF_TEST_ENABLE
    ESP_LOGI(TAG, "[M] init step: scanner serial self-test");
#if SCANNER_UART_TX_DIAG_BURST_ENABLE
    ESP_LOGW(TAG, "[M] init step: scanner TX diagnostic burst mode enabled");
    runScannerUartTxBurstDiagnostic(UART_SCANNER);
#else
    scannerSerialSelfTest(UART_SCANNER);
#endif
#else
    ESP_LOGI(TAG, "[M] init step: scanner initialization (serial mode, self-test disabled)");
    initializeScanner();
#endif
#else
    initializeScanner();
#endif
    ESP_LOGI(TAG, "[M] init step: RGB LED");
    setupRgbLed();
    ESP_LOGI(TAG, "[M] init completed");

    uint64_t lastLoopLog = esp_timer_get_time() / 1000;
    while (true)
    {
        const uint64_t now = esp_timer_get_time() / 1000;
        if (now - lastLoopLog > 10000)
        {
            ESP_LOGI(TAG, "Main loop heartbeat");
            lastLoopLog = now;
        }

        if (gMasterUartReady)
        {
            handleMasterSerial(UART_MASTER
#if defined(SCANNER_CONTROL_USE_SERIAL)
                               ,
                               UART_SCANNER
#endif
            );
        }
#if defined(SCANNER_CONTROL_USE_SERIAL)
        if (gScannerUartReady)
        {
            forwardScannerData(UART_SCANNER, UART_MASTER);
        }
#endif
#if ENABLE_DISPLAY_LVGL
        lv_timer_handler();
#endif
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
