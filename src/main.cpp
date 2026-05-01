#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <esp_log.h>
#if USE_WIFI == 1
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_http_server.h>
#include <esp_ota_ops.h>
#endif
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

#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
#include <fcntl.h>
#include <unistd.h>
#endif

#ifndef ENABLE_DISPLAY_LVGL
#define ENABLE_DISPLAY_LVGL 1
#endif


//#define ENABLE_DISPLAY_TEST


#include "pin_config.h"
#include "display.h"
#if ENABLE_DISPLAY_LVGL
#include <lvgl.h>
#include "ui/fonts/user_fonts.h"
extern "C" const lv_img_dsc_t logo_img;
#endif
#include "GDEY0154D67.h"
#include "led_control.h"
#include "scanner_control.h"
#include "master_protocol.h"
#if ENABLE_DISPLAY_LVGL
static lv_obj_t *display_label = nullptr;
static bool g_display_inverted = false; // false = white bg / black text (default)
static inline lv_color_t themeBg() { return g_display_inverted ? lv_color_black() : lv_color_white(); }
static inline lv_color_t themeText() { return g_display_inverted ? lv_color_white() : lv_color_black(); }
static lv_obj_t *fontLoopButton = nullptr;
static lv_obj_t *fontLoopLabel = nullptr;
static TaskHandle_t displayTestTaskHandle = nullptr;
static TaskHandle_t fontLoopTaskHandle = nullptr;
static bool fontLoopRunning = false;
static void drawCheckerboard();
static void drawSplitScreen(bool leftWhite);
static void showTextInFonts();
static void showLogoTest();
static const lv_img_dsc_t *getInvertedLogoImg();
static void fontLoopTask(void *pvParameter);
static void performDisplayTestSequence();
void displayLogo();
#endif

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

#if USE_WIFI == 1
#define WIFI_SSID "FWAP02"
#define WIFI_PASSWORD "fwxi56cgo"
#endif

#if defined(USE_EPD_GDEY1085F51)
constexpr uint16_t EPD_WIDTH = 1360;
constexpr uint16_t EPD_HEIGHT = 480;
#else
constexpr uint16_t EPD_WIDTH = 200;
constexpr uint16_t EPD_HEIGHT = 200;
#endif

static const char *TAG = "EPaperQr";

#include "conditional_log.h"
static constexpr const char *kAppLastChangeDescription = "Prefisso \u00a7: clear display prima del testo";
static constexpr const char *kAppLastChangeTimestamp = __TIMESTAMP__;
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
static const uart_port_t UART_MASTER = UART_NUM_0; // USB console used for master commands, UART init skipped.
#else
static const uart_port_t UART_MASTER = UART_NUM_0;
#endif
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
        lv_tick_inc(10);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
#else
    vTaskDelay(delayTicks);
#endif
}

static void waitForBootButtonPress()
{
    _LOGI("[M] waiting 5 seconds before next display step");
    lvglDelayAndRefresh(pdMS_TO_TICKS(5000));
}

static void fontLoopBtnEventCb(lv_event_t *e)
{
    LV_UNUSED(e);
    fontLoopRunning = !fontLoopRunning;
    if (fontLoopRunning && fontLoopTaskHandle == nullptr)
    {
        xTaskCreate(fontLoopTask, "fontLoop", 4096, nullptr, 5, &fontLoopTaskHandle);
    }
    if (fontLoopButton)
    {
        lv_obj_t *label = lv_obj_get_child(fontLoopButton, 0);
        if (label)
        {
            lv_label_set_text(label, fontLoopRunning ? "Stop menu 4 loop" : "Start menu 4 loop");
        }
    }
}

static void fontLoopTask(void *pvParameter)
{
    (void)pvParameter;
    static const struct FontEntry { const char *name; const lv_font_t *font; } fonts[] = {
        {"Montserrat 14", &lv_font_montserrat_14},
        {"Montserrat 28", &lv_font_montserrat_28},
        {"Montserrat 48", &lv_font_montserrat_48},
        {"GoogleSans 10", &GoogleSans10},
        {"GoogleSans 15", &GoogleSans15},
        {"GoogleSans 20", &GoogleSans20},
        {"GoogleSans 35", &GoogleSans35},
        {"GoogleSans 50", &GoogleSans50},
        {"GoogleSans 60", &GoogleSans60},
        {"GoogleSans 100", &GoogleSans100},
        {"GoogleSans 140", &GoogleSans140},
    };
    size_t index = 0;
    while (fontLoopRunning)
    {
        if (fontLoopLabel)
        {
            lv_obj_del(fontLoopLabel);
            fontLoopLabel = nullptr;
        }
        lv_obj_t *screen = lv_scr_act();
        fontLoopLabel = lv_label_create(screen);
        lv_obj_set_style_text_color(fontLoopLabel, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_text_font(fontLoopLabel, fonts[index].font, LV_PART_MAIN);
        lv_label_set_text_fmt(fontLoopLabel, "%s 123", fonts[index].name);
        lv_obj_center(fontLoopLabel);
        index = (index + 1) % (sizeof(fonts) / sizeof(fonts[0]));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    fontLoopTaskHandle = nullptr;
    vTaskDelete(NULL);
}

static void displayTestTask(void *pvParameter)
{
    (void)pvParameter;
    performDisplayTestSequence();
    displayTestTaskHandle = nullptr;
    vTaskDelete(NULL);
}

static void startDisplayTestTask()
{
    if (displayTestTaskHandle == nullptr)
    {
        xTaskCreate(displayTestTask, "displayTest", 8192, nullptr, 5, &displayTestTaskHandle);
    }
}

static void performDisplayTestSequence()
{
    if (!GDEY0154D67_is_initialized())
    {
        _LOGW("[M] display test skipped: epaper not initialized");
        return;
    }

    _LOGI("[M] display test 1/5: full refresh current display");
    epdFullRefresh();
    _LOGI("[M] display test 1/5: draw checkerboard");
    drawCheckerboard();
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));

    _LOGI("[M] display test 2/5: draw half screen white/black");
    drawSplitScreen(true);
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));

    _LOGI("[M] display test 3/5: draw half screen black/white");
    drawSplitScreen(false);
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));

    _LOGI("[M] display test 4/5: show text '123' in three fonts");
    showTextInFonts();
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));

    _LOGI("[M] display test 5/5: show logo");
    showLogoTest();
    lvglDelayAndRefresh(pdMS_TO_TICKS(10000));

    _LOGI("[M] display test complete");
}

#if defined(SCANNER_CONTROL_USE_SERIAL)
static const uart_port_t UART_SCANNER = UART_NUM_1;
static bool gScannerUartReady = false;
#endif
#if USE_WIFI == 1
static httpd_handle_t otaServer = nullptr;
#endif

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

#if USE_WIFI == 1
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
#endif

#if defined(SCANNER_CONTROL_USE_SERIAL) && SCANNER_UART_TX_DIAG_BURST_ENABLE
static void runScannerUartTxBurstDiagnostic(uart_port_t scannerPort)
{
    static const uint8_t kPattern[] = {0x55, 0xAA, 0x55, 0xAA, 0x0D, 0x0A};
    _LOGW("[M] SCN TX diag burst attivo: uart=%d tx_pin=%d interval=%dms pattern_len=%u",
             static_cast<int>(scannerPort),
             PIN_SCANNER_TX_CFG,
             SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS,
             static_cast<unsigned>(sizeof(kPattern)));

    while (true)
    {
        const int written = uart_write_bytes(scannerPort, reinterpret_cast<const char *>(kPattern), sizeof(kPattern));
        if (written != static_cast<int>(sizeof(kPattern)))
        {
            _LOGW("[M] SCN TX diag burst write parziale=%d expected=%u",
                     written,
                     static_cast<unsigned>(sizeof(kPattern)));
        }
        uart_wait_tx_done(scannerPort, pdMS_TO_TICKS(20));
        vTaskDelay(pdMS_TO_TICKS(SCANNER_UART_TX_DIAG_BURST_INTERVAL_MS));
    }
}
#endif

#if USE_WIFI == 1
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
#endif

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
    // MONTSERRAT (indices 0-2, font# 1-3)
    {&lv_font_montserrat_14, 12, 12, "montserrat_14"},       // index 0: font# 1
    {&lv_font_montserrat_28, 5, 4, "montserrat_28"},         // index 1: font# 2
    {&lv_font_montserrat_48, 2, 2, "montserrat_48"},         // index 2: font# 3
    // GOOGLE SANS (indices 3-10, font# 4-11)
    {&GoogleSans10, 18, 15, "GoogleSans10"},                 // index 3: font# 4
    {&GoogleSans15, 15, 12, "GoogleSans15"},                 // index 4: font# 5
    {&GoogleSans20, 12, 10, "GoogleSans20"},                 // index 5: font# 6
    {&GoogleSans35, 6, 5, "GoogleSans35"},                   // index 6: font# 7
    {&GoogleSans50, 5, 3, "GoogleSans50"},                   // index 7: font# 8
    {&GoogleSans60, 4, 2, "GoogleSans60"},                   // index 8: font# 9
    {&GoogleSans100, 3, 2, "GoogleSans100"},                 // index 9: font# 10
    {&GoogleSans140, 2, 1, "GoogleSans140"},                 // index 10: font# 11
    // GOOGLE SANS BOLD (indices 11-14, font# 12-15)
    {&GoogleSansBold40, 5, 3, "GoogleSansBold40"},           // index 11: font# 12 (bold)
    {&GoogleSansBold60, 4, 2, "GoogleSansBold60"},           // index 12: font# 13 (bold)
    {&GoogleSansBold100, 3, 2, "GoogleSansBold100"},         // index 13: font# 14 (bold)
    {&GoogleSansBold140, 2, 1, "GoogleSansBold140"},         // index 14: font# 15 (bold)
};

static const uint8_t kFontCount = sizeof(fontDefinitions) / sizeof(fontDefinitions[0]);

#endif

#if USE_WIFI == 1
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
#endif

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

#if USE_WIFI == 1
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
#endif

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
    // Tokenize: alphanumeric sequences (words) and individual non-alnum non-whitespace chars (punct).
    // Words are never split. Punct chars are natural split points (no space added around them).
    struct Token { std::string str; bool isWord; };
    std::vector<Token> tokens;
    std::string cur;

    auto isAlnum = [](unsigned char c) -> bool {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    };
    auto flushWord = [&]() {
        if (!cur.empty()) { tokens.push_back({cur, true}); cur.clear(); }
    };

    for (char ch : text) {
        if (ch == '\n') {
            flushWord();
            tokens.push_back({"", false}); // newline sentinel
        } else if (ch == ' ') {
            flushWord();
        } else if (isAlnum((unsigned char)ch)) {
            cur.push_back(ch);
        } else {
            // punct: flush current word, emit punct as its own token
            flushWord();
            tokens.push_back({{ch}, false});
        }
    }
    flushWord();

    // Build lines: a space is inserted before a word only when the last written char is alphanumeric.
    std::vector<std::string> lines;
    std::string line;

    for (const auto &tok : tokens) {
        if (!tok.isWord && tok.str.empty()) {
            // Newline sentinel
            lines.push_back(line);
            line.clear();
            continue;
        }
        bool needSpace = tok.isWord && !line.empty() && isAlnum((unsigned char)line.back());
        size_t addLen = (needSpace ? 1 : 0) + tok.str.length();
        if (!line.empty() && line.length() + addLen > fontDef.maxCharsPerLine) {
            lines.push_back(line);
            line = tok.str;
        } else {
            if (needSpace) line += ' ';
            line += tok.str;
        }
    }
    if (!line.empty() || lines.empty()) {
        lines.push_back(line);
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
    if (lines.size() > fontDef.maxLines)
        return false;
    // A word longer than maxCharsPerLine means font is too large
    for (const auto &line : lines)
        if (line.length() > fontDef.maxCharsPerLine)
            return false;
    return true;
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
        // Largest to smallest: GoogleSans140(7), GoogleSans100(6), then Montserrat descending
        candidates = {7, 6, 0, 1, 2, 3, 4, 5};
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
    // Font# (1-15) → array index (0-14)
    // MONTSERRAT (1-3): indices 0-2
    // GOOGLE SANS (4-11): indices 3-10
    // GOOGLE SANS BOLD (12-15): indices 11-14
    if (fontNumber >= 1 && fontNumber <= 15) {
        return fontNumber - 1;
    }
    return 0; // default to Montserrat 14
}

// Check if a font is a bold variant by checking its name
static bool isBoldFont(const FontDefinition &fontDef)
{
    return fontDef.name && strstr(fontDef.name, "bold") != nullptr;
}

void clearActiveScreen()
{
#if ENABLE_DISPLAY_LVGL
    lv_obj_t *screen = lv_scr_act();
    if (screen != nullptr)
    {
        lv_obj_clean(screen);
        lv_obj_set_style_bg_color(screen, themeBg(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    }
    display_label = nullptr; // lv_obj_clean deleted all children including display_label
#endif
}

void clearDisplay()
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
#endif
    if (GDEY0154D67_is_initialized())
    {
        GDEY0154D67_clear_screen_partial();
        _LOGI("clearDisplay: LVGL buffer cleared + e-paper partial refresh applied");
    }
}

void fillDisplayWithDots()
{
    // Fill e-paper display with dots pattern based on current theme
    if (GDEY0154D67_is_initialized())
    {
        if (g_display_inverted) {
            _LOGI("fillDisplayWithDots: inverted mode -> black dots");
            GDEY0154D67_fill_with_black_dots();
        } else {
            _LOGI("fillDisplayWithDots: normal mode -> white dots");
            GDEY0154D67_fill_with_white_dots();
        }
    }
}

void setDisplayTheme(bool inverted)
{
#if ENABLE_DISPLAY_LVGL
    g_display_inverted = inverted;
    lv_obj_t *screen = lv_scr_act();
    if (screen != nullptr)
    {
        lv_obj_set_style_bg_color(screen, themeBg(), LV_PART_MAIN);
        if (display_label != nullptr)
        {
            lv_obj_set_style_bg_color(display_label, themeBg(), LV_PART_MAIN);
            lv_obj_set_style_text_color(display_label, themeText(), LV_PART_MAIN);
        }
        // No lv_obj_invalidate: theme takes effect on next displayText() call.
        // Forcing a flush here risks triggering an EPD refresh while the panel may be busy.
    }
    _LOGI("Display theme: %s", inverted ? "inverted (black bg/white text)" : "normal (white bg/black text)");
#endif
}

static void drawCheckerboard()
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
    lv_obj_t *screen = lv_scr_act();
    const int width = EPD_WIDTH;
    const int height = EPD_HEIGHT;
    static lv_color_t *canvas_buf = nullptr;
    if (canvas_buf == nullptr)
    {
        canvas_buf = static_cast<lv_color_t *>(malloc(sizeof(lv_color_t) * width * height));
        if (canvas_buf == nullptr)
        {
            _LOGW("Failed to allocate checkerboard canvas buffer");
            return;
        }
    }

    lv_obj_t *canvas = lv_canvas_create(screen);
    lv_canvas_set_buffer(canvas, canvas_buf, width, height, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_opa = LV_OPA_COVER;
    const int squareSize = 20;
    for (int row = 0; row < height / squareSize; ++row)
    {
        for (int col = 0; col < width / squareSize; ++col)
        {
            rect_dsc.bg_color = ((row + col) & 1) ? lv_color_black() : lv_color_white();
            lv_canvas_draw_rect(canvas, col * squareSize, row * squareSize, squareSize, squareSize, &rect_dsc);
        }
    }
    lv_obj_center(canvas);
#endif
}

static void drawSplitScreen(bool leftWhite)
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
    lv_obj_t *screen = lv_scr_act();

    lv_obj_t *left = lv_obj_create(screen);
    lv_obj_set_size(left, EPD_WIDTH / 2, EPD_HEIGHT);
    lv_obj_set_style_border_width(left, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(left, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(left, leftWhite ? lv_color_white() : lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(left, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_pos(left, 0, 0);

    lv_obj_t *right = lv_obj_create(screen);
    lv_obj_set_size(right, EPD_WIDTH - EPD_WIDTH / 2, EPD_HEIGHT);
    lv_obj_set_style_border_width(right, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(right, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(right, leftWhite ? lv_color_black() : lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(right, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_pos(right, EPD_WIDTH / 2, 0);
#endif
}

static void showTextInFonts()
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
    lv_obj_t *screen = lv_scr_act();
    struct FontLine { const lv_font_t *font; int32_t y; } lines[] = {
        {LV_FONT_DEFAULT, 8},
        {&lv_font_montserrat_14, 40},
        {&lv_font_montserrat_28, 80},
        {&lv_font_montserrat_48, 125},
    };
    for (const FontLine &line : lines)
    {
        lv_obj_t *label = lv_label_create(screen);
        lv_obj_set_style_text_font(label, line.font, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
        lv_label_set_text(label, "123");
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, line.y);
    }
#endif
}

static const lv_img_dsc_t *getInvertedLogoImg()
{
#if ENABLE_DISPLAY_LVGL
    static bool initialized = false;
    static lv_img_dsc_t inverted_logo = {};
    static std::vector<uint8_t> inverted_data;

    if(!initialized) {
        const uint8_t *src_data = reinterpret_cast<const uint8_t *>(logo_img.data);
        const size_t len = logo_img.data_size;
        inverted_data.resize(len);
        for(size_t i = 0; i < len; ++i) {
            inverted_data[i] = static_cast<uint8_t>(~src_data[i]);
        }
        inverted_logo = logo_img;
        inverted_logo.data = inverted_data.data();
        initialized = true;
    }
    // Return XOR-ed logo if in inverted mode (white on black), normal logo otherwise (black on white)
    if(g_display_inverted) {
        _LOGI("Logo: inverted mode -> returning normal (black) logo");
        return &logo_img;
    } else {
        _LOGI("Logo: normal mode -> returning inverted (white) logo");
        return &inverted_logo;
    }
#else
    return nullptr;
#endif
}

static void showLogoTest()
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
    lv_obj_t *screen = lv_scr_act();
    if(screen != nullptr) {
        const lv_img_dsc_t *img = getInvertedLogoImg();
        if(img != nullptr) {
            lv_obj_t *image = lv_img_create(screen);
            lv_img_set_src(image, img);
            lv_obj_center(image);
        }
    }
#endif
}

void displayLogo()
{
#if ENABLE_DISPLAY_LVGL
    clearActiveScreen();
    showLogoTest();
#else
    _LOGW("displayLogo called but LVGL display is disabled");
#endif
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
    if (display_label == nullptr)
    {
        lv_obj_t *screen = lv_scr_act();
        if (screen == nullptr) { _LOGW("displayText: no active screen"); return; }
        display_label = lv_label_create(screen);
        lv_obj_set_width(display_label, EPD_WIDTH);
        lv_label_set_long_mode(display_label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(display_label, themeText(), LV_PART_MAIN);
        lv_obj_set_style_text_align(display_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(display_label, themeBg(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(display_label, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    }
    const std::string normalized = normalizeText(raw_text);
    const uint8_t fontIndex = selectFontIndex(normalized);
    const auto &fontDef = fontDefinitions[fontIndex];
    std::string output = fitsInFont(normalized, fontDef) ? joinLines(wrapText(normalized, fontDef)) : truncateText(normalized, fontDef);

    lv_obj_set_style_text_font(display_label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(display_label, output.c_str());
    lv_obj_align(display_label, LV_ALIGN_CENTER, 0, 0);
    _LOGI("Display text using %s", fontDef.name);
}

void displayText(const std::string &raw_text, uint8_t fontNumber, uint8_t x, uint8_t y)
{
    lv_obj_t *screen = lv_scr_act();
    if (screen == nullptr) { _LOGW("displayText ext: no active screen"); return; }

    // Check if we need clean display (§ prefix handling):
    // If § is detected during stripSpecialPrefixes in master_protocol.cpp, we'll use a single reusable label
    // with opaque background (like auto mode) instead of creating a new transparent label.
    // This approach is more fluid because it automatically clears previous content.
    // For now, we create a new label and let the caller handle the clearing via partial refresh.
    
    lv_obj_t *label = lv_label_create(screen);
    lv_obj_set_style_text_color(label, themeText(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(label, themeBg(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, LV_PART_MAIN); // transparent bg: don't overwrite existing content

    const std::string normalized = normalizeText(raw_text);
    const uint8_t fontIndex = resolveExtendedFontIndex(fontNumber);
    const auto &fontDef = fontDefinitions[fontIndex];
    lv_obj_set_width(label, EPD_WIDTH);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(label, normalized.c_str());
    lv_obj_align(label, LV_ALIGN_CENTER,
                 0,
                 static_cast<int32_t>(y) - EPD_HEIGHT / 2);

    display_label = label;
    _LOGI("Extended display text font#%u center=(%u,%u) len=%u", fontNumber, x, y, static_cast<unsigned>(normalized.length()));
}

void displayTextClean(const std::string &raw_text, uint8_t fontNumber, uint8_t x, uint8_t y)
{
    // Clean display version: reuse the same label with opaque background (like auto mode).
    // This provides smooth transitions by automatically clearing previous content.
    // Called when § prefix is detected to provide fluid display updates.
    
    if (display_label == nullptr)
    {
        lv_obj_t *screen = lv_scr_act();
        if (screen == nullptr) { _LOGW("displayTextClean: no active screen"); return; }
        display_label = lv_label_create(screen);
        lv_obj_set_style_text_color(display_label, themeText(), LV_PART_MAIN);
        lv_obj_set_style_text_align(display_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_bg_color(display_label, themeBg(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(display_label, LV_OPA_COVER, LV_PART_MAIN);  // opaque background clears previous content
    }

    const std::string normalized = normalizeText(raw_text);
    const uint8_t fontIndex = resolveExtendedFontIndex(fontNumber);
    const auto &fontDef = fontDefinitions[fontIndex];
    
    lv_obj_set_style_text_font(display_label, fontDef.font, LV_PART_MAIN);
    lv_label_set_text(display_label, normalized.c_str());
    lv_obj_align(display_label, LV_ALIGN_CENTER,
                 0,
                 static_cast<int32_t>(y) - EPD_HEIGHT / 2);

    _LOGI("Extended display text CLEAN font#%u center=(%u,%u) len=%u", fontNumber, x, y, static_cast<unsigned>(normalized.length()));
}

void displayJpegCentered(const char *path)
{
    if (path == nullptr)
    {
        _LOGW("displayJpegCentered called with null path");
        return;
    }

    lv_obj_t *screen = lv_scr_act();
    if (screen == nullptr)
    {
        _LOGW("displayJpegCentered failed: no active screen");
        return;
    }

    lv_img_header_t info;
    lv_res_t info_res = lv_img_decoder_get_info(path, &info);
    if (info_res != LV_RES_OK)
    {
        _LOGW("displayJpegCentered failed to decode image: %s", path);
        return;
    }

    lv_obj_t *image = lv_img_create(screen);
    lv_img_set_src(image, path);
    lv_obj_center(image);
    _LOGI("displayJpegCentered loaded image: %s (%ux%u)", path, info.w, info.h);
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
        lv_framebuffer = new lv_color_t[EPD_WIDTH * EPD_HEIGHT];
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
    lv_disp_draw_buf_init(draw_buf, lv_framebuffer, nullptr, EPD_WIDTH * EPD_HEIGHT);
    lv_disp_drv_init(disp_drv);
    disp_drv->hor_res = EPD_WIDTH;
    disp_drv->ver_res = EPD_HEIGHT;
    disp_drv->flush_cb = epd_flush_cb;
    disp_drv->draw_buf = draw_buf;
    disp_drv->full_refresh = 1; // use full-screen refresh to ensure EPD receives a complete update
    lv_disp_drv_register(disp_drv);
}

void buildUi()
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);

    showLogoTest();
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

void displayTextClean(const std::string &raw_text, uint8_t fontNumber, uint8_t x, uint8_t y)
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
    _LOGI("[M] Last change: %s", kAppLastChangeDescription);
    _LOGI("[M] Last change timestamp: %s", kAppLastChangeTimestamp);
    _LOGI("[M] Starting EPaperQr FreeRTOS application");
    vTaskDelay(pdMS_TO_TICKS(300));

    _LOGI("[M] init step: SPIFFS");
    if (mountSpiffs())
    {
        logSpiffsFiles();
    }
#if ENABLE_DISPLAY_LVGL
    _LOGI("[M] init step: display");
    setupDisplay();
    _LOGI("[M] init step: LVGL");
    setupLvgl();
    _LOGI("[M] init step: UI");
    buildUi();
#if ENABLE_DISPLAY_TEST == 1 
    _LOGI("[M] init step: display test sequence");
    startDisplayTestTask();
#endif    
#else
    _LOGI("[M] init step: display/LVGL skipped (ENABLE_DISPLAY_LVGL=0)");
#endif

    _LOGI("[M] init step: WiFi");
#if USE_WIFI == 1
    initWifi();
#endif
#if defined(MASTER_PROTOCOL_USE_USB_CONSOLE)
    _LOGI("[M] init step: USB console master commands");
    gMasterUartReady = true;
    int stdinFlags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (stdinFlags >= 0)
    {
        fcntl(STDIN_FILENO, F_SETFL, stdinFlags | O_NONBLOCK);
    }
    else
    {
        _LOGW("[M] failed to set stdin non-blocking");
    }
#else
    _LOGI("[M] init step: UART master");
    gMasterUartReady = initUart(UART_MASTER, 9600, PIN_TX_ESP_CFG, PIN_RX_ESP_CFG);
#endif
#if defined(SCANNER_CONTROL_USE_SERIAL)
    _LOGI("[M] init step: UART scanner");
    gScannerUartReady = initUart(UART_SCANNER, 9600, PIN_SCANNER_TX_CFG, PIN_SCANNER_RX_CFG);
#if SCANNER_SERIAL_SELF_TEST_ENABLE
    _LOGI("[M] init step: scanner serial self-test");
#if SCANNER_UART_TX_DIAG_BURST_ENABLE
    _LOGW("[M] init step: scanner TX diagnostic burst mode enabled");
    runScannerUartTxBurstDiagnostic(UART_SCANNER);
#else
    scannerSerialSelfTest(UART_SCANNER);
#endif
#else
    _LOGI("[M] init step: scanner initialization (serial mode, self-test disabled)");
    initializeScanner();
#endif
#else
    initializeScanner();
#endif
    _LOGI("[M] init step: RGB LED");
    setupRgbLed();
    _LOGI("[M] init completed");

    uint64_t lastLoopLog = esp_timer_get_time() / 1000;
    while (true)
    {
        const uint64_t now = esp_timer_get_time() / 1000;
        if (now - lastLoopLog > 10000)
        {
            _LOGI("Main loop heartbeat");
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
        lv_tick_inc(5);
        lv_timer_handler();
#endif
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
