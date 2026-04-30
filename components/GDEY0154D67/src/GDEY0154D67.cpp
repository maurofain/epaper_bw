#include "GDEY0154D67.h"
#include "pin_config.h"
#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <cstdlib>
#include <cstring>

static const char *TAG = "GDEY0154D67";
static spi_device_handle_t epdSpiHandle = nullptr;
static bool epdInitialized = false;
static GDEY0154D67_Orientation currentOrientation = GDEY0154D67_Orientation::ORIENTATION_0;
static constexpr uint16_t PANEL_WIDTH = 200;
static constexpr uint16_t PANEL_HEIGHT = 200;
static constexpr int EPD_BUSY_ACTIVE_LEVEL = 0; // SSD1681: LOW when busy, HIGH when ready (GxEPD2: wait while pin != HIGH)
static const uint32_t EPD_BUSY_TIMEOUT_MS = 15000;

static void configureOutputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << static_cast<uint32_t>(pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

static void configureInputPin(gpio_num_t pin)
{
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << static_cast<uint32_t>(pin);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
}

static void feed_watchdog()
{
    vTaskDelay(pdMS_TO_TICKS(1));
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
        const TickType_t start = xTaskGetTickCount();
        while (gpio_get_level(busyPin) == EPD_BUSY_ACTIVE_LEVEL)
        {
            if ((xTaskGetTickCount() - start) > pdMS_TO_TICKS(EPD_BUSY_TIMEOUT_MS))
            {
                ESP_LOGW(TAG, "EPD BUSY timeout");
                break;
            }
            feed_watchdog();
        }
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
        ESP_LOGW(TAG, "EPD reset pin not configured");
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
    if (epdSpiHandle != nullptr)
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
        ESP_LOGE(TAG, "SPI bus initialize failed: 0x%02X", err);
        return err;
    }

    spi_device_interface_config_t devConfig = {};
    devConfig.clock_speed_hz = 2000000;
    devConfig.mode = 0;
    devConfig.spics_io_num = PIN_EPD_CS_CFG;
    devConfig.queue_size = 1;
    devConfig.flags = SPI_DEVICE_HALFDUPLEX;

    err = spi_bus_add_device(SPI2_HOST, &devConfig, &epdSpiHandle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI device add failed: 0x%02X", err);
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

static void setEpdRamArea(uint8_t xStart, uint8_t xEnd, uint16_t yStart, uint16_t yEnd)
{
    sendEpdCommand(0x11); // data entry mode: x-increase, y-increase
    sendEpdData(0x03);
    sendEpdCommand(0x44);
    sendEpdData(xStart);
    sendEpdData(xEnd);

    sendEpdCommand(0x45);
    sendEpdData(static_cast<uint8_t>(yStart & 0xFF));
    sendEpdData(static_cast<uint8_t>((yStart >> 8) & 0xFF));
    sendEpdData(static_cast<uint8_t>(yEnd & 0xFF));
    sendEpdData(static_cast<uint8_t>((yEnd >> 8) & 0xFF));
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
    setEpdRamArea(0x00, 0x18, 0x0000, 0x00C7);
    setEpdRamPointer(0x00, 0x0000);
    sendEpdCommand(command);
    const size_t length = (static_cast<size_t>(PANEL_WIDTH) * PANEL_HEIGHT) / 8;
    // Send all data in ONE SPI transaction (CS held low throughout, as required by SSD1681)
    uint8_t *buf = static_cast<uint8_t *>(malloc(length));
    if (buf == nullptr)
    {
        ESP_LOGE(TAG, "writeEpdRam alloc failed");
        return;
    }
    memset(buf, value, length);
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 1);
    epdTransmit(buf, length);
    free(buf);
}

static void epdRefresh()
{
    sendEpdCommand(0x22);
    sendEpdData(0xF7); // full update sequence
    sendEpdCommand(0x20);
    waitUntilIdle();
}

static void epdRefreshPartial()
{
    sendEpdCommand(0x22);
    sendEpdData(0xFC); // partial update sequence
    sendEpdCommand(0x20);
    waitUntilIdle();
}

static void setPartialWindow(int x1, int y1, int x2, int y2)
{
    const uint8_t xStart = static_cast<uint8_t>(x1 / 8);
    const uint8_t xEnd = static_cast<uint8_t>((x2 + 8) / 8 - 1);

    setEpdRamArea(xStart, xEnd, static_cast<uint16_t>(y1), static_cast<uint16_t>(y2));
    setEpdRamPointer(xStart, static_cast<uint16_t>(y1));
}

static bool epdInitSequence()
{
    // SSD1681 init sequence (matches GxEPD2_154_GDEY0154D67)
    sendEpdCommand(0x12); // soft reset
    vTaskDelay(pdMS_TO_TICKS(10));

    sendEpdCommand(0x01); // Driver output control
    sendEpdData(0xC7);
    sendEpdData(0x00);
    sendEpdData(0x00);

    sendEpdCommand(0x3C); // BorderWaveform
    sendEpdData(0x05);

    sendEpdCommand(0x18); // Reading temperature sensor
    sendEpdData(0x80);

    // set full RAM area and reset pointer (also sends 0x11/0x03 data entry mode)
    setEpdRamArea(0x00, 0x18, 0x0000, 0x00C7);
    setEpdRamPointer(0x00, 0x0000);

    waitUntilIdle();
    return true;
}

static void rotate_coordinates(int x, int y, int &out_x, int &out_y)
{
    switch (currentOrientation)
    {
        case GDEY0154D67_Orientation::ORIENTATION_90:
            out_x = PANEL_WIDTH - 1 - y;
            out_y = x;
            break;
        case GDEY0154D67_Orientation::ORIENTATION_180:
            out_x = PANEL_WIDTH - 1 - x;
            out_y = PANEL_HEIGHT - 1 - y;
            break;
        case GDEY0154D67_Orientation::ORIENTATION_270:
            out_x = y;
            out_y = PANEL_HEIGHT - 1 - x;
            break;
        default:
            out_x = x;
            out_y = y;
            break;
    }
}

static void createPartialImageBuffer(const lv_area_t *area, lv_color_t *color_p, uint8_t *buffer, int widthBytes, int height, int xOffset, int yOffset)
{
    const int areaWidth = area->x2 - area->x1 + 1;
    const int areaHeight = area->y2 - area->y1 + 1;

    memset(buffer, 0xFF, static_cast<size_t>(widthBytes) * height);

    for (int row = 0; row < areaHeight; ++row)
    {
        for (int col = 0; col < areaWidth; ++col)
        {
            int logicalX = area->x1 + col;
            int logicalY = area->y1 + row;
            int physicalX;
            int physicalY;
            rotate_coordinates(logicalX, logicalY, physicalX, physicalY);

            if (physicalX < xOffset || physicalX >= xOffset + widthBytes * 8 || physicalY < yOffset || physicalY >= yOffset + height)
            {
                continue;
            }

            const bool white = lv_color_to1(color_p[row * areaWidth + col]) != 0;
            int column = physicalX - xOffset;
            int byteIndex = (physicalY - yOffset) * widthBytes + (column / 8);
            int bit = 7 - (column % 8);
            if (!white)
            {
                buffer[byteIndex] &= static_cast<uint8_t>(~(1 << bit));
            }
        }
    }
}

void GDEY0154D67_init()
{
    if (epdInitialized)
    {
        return;
    }

    configureOutputPin(static_cast<gpio_num_t>(PIN_EPD_CS_CFG));
    configureOutputPin(static_cast<gpio_num_t>(PIN_EPD_DC_CFG));
    configureOutputPin(static_cast<gpio_num_t>(PIN_EPD_RST_CFG));
    if (PIN_EPD_CS2_CFG >= 0)
    {
        configureOutputPin(static_cast<gpio_num_t>(PIN_EPD_CS2_CFG));
    }
    if (PIN_EPD_PWR_CFG >= 0)
    {
        configureOutputPin(static_cast<gpio_num_t>(PIN_EPD_PWR_CFG));
        gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_PWR_CFG), 0);
    }
    if (PIN_EPD_BUSY_CFG >= 0)
    {
        configureInputPin(static_cast<gpio_num_t>(PIN_EPD_BUSY_CFG));
    }

    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_CS_CFG), 1);
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 0);
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_RST_CFG), 1);
    if (PIN_EPD_CS2_CFG >= 0)
    {
        gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_CS2_CFG), 1);
    }

    esp_err_t err = initEpaperSpi();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "EPD SPI init failed");
        return;
    }

    epdPowerOn();
    epdReset();
    if (!epdInitSequence())
    {
        ESP_LOGE(TAG, "EPD init sequence failed");
        return;
    }

    GDEY0154D67_clear_screen();
    epdInitialized = true;
    ESP_LOGI(TAG, "EPD initialized");
}

void GDEY0154D67_set_orientation(GDEY0154D67_Orientation orientation)
{
    currentOrientation = orientation;
}

bool GDEY0154D67_is_initialized()
{
    return epdInitialized;
}

void GDEY0154D67_clear_screen()
{
    writeEpdRam(0x26, 0xFF); // previous buffer = white
    writeEpdRam(0x24, 0xFF); // current buffer = white
    epdRefresh();
}

void GDEY0154D67_black_screen()
{
    writeEpdRam(0x26, 0x00); // previous buffer = black
    writeEpdRam(0x24, 0x00); // current buffer = black
    epdRefresh();
}

void GDEY0154D67_refresh()
{
    epdRefresh();
}

#if ENABLE_DISPLAY_LVGL
void GDEY0154D67_draw_partial(const lv_area_t *area, lv_color_t *color_p)
{
    if (!epdInitialized || area == nullptr || color_p == nullptr)
    {
        return;
    }

    int x1 = area->x1;
    int x2 = area->x2;
    int y1 = area->y1;
    int y2 = area->y2;

    if (x1 < 0)
        x1 = 0;
    if (y1 < 0)
        y1 = 0;
    if (x2 >= static_cast<int>(PANEL_WIDTH))
        x2 = static_cast<int>(PANEL_WIDTH) - 1;
    if (y2 >= static_cast<int>(PANEL_HEIGHT))
        y2 = static_cast<int>(PANEL_HEIGHT) - 1;
    if (x2 < x1 || y2 < y1)
    {
        return;
    }

    int cornersX[4];
    int cornersY[4];
    rotate_coordinates(x1, y1, cornersX[0], cornersY[0]);
    rotate_coordinates(x2, y1, cornersX[1], cornersY[1]);
    rotate_coordinates(x1, y2, cornersX[2], cornersY[2]);
    rotate_coordinates(x2, y2, cornersX[3], cornersY[3]);

    int xMin = cornersX[0];
    int xMax = cornersX[0];
    int yMin = cornersY[0];
    int yMax = cornersY[0];
    for (int index = 1; index < 4; ++index)
    {
        if (cornersX[index] < xMin) xMin = cornersX[index];
        if (cornersX[index] > xMax) xMax = cornersX[index];
        if (cornersY[index] < yMin) yMin = cornersY[index];
        if (cornersY[index] > yMax) yMax = cornersY[index];
    }

    const uint8_t xStart = static_cast<uint8_t>(xMin / 8);
    const uint8_t xEnd = static_cast<uint8_t>((xMax + 8) / 8 - 1);
    const int widthBytes = xEnd - xStart + 1;
    const int height = yMax - yMin + 1;
    uint8_t *buffer = static_cast<uint8_t *>(malloc(static_cast<size_t>(widthBytes) * static_cast<size_t>(height)));
    if (buffer == nullptr)
    {
        ESP_LOGW(TAG, "Partial image buffer allocation failed");
        return;
    }

    createPartialImageBuffer(area, color_p, buffer, widthBytes, height, xMin, yMin);

    // write new image to current buffer (0x24)
    setEpdRamArea(xStart, xEnd, static_cast<uint16_t>(yMin), static_cast<uint16_t>(yMax));
    setEpdRamPointer(xStart, static_cast<uint16_t>(yMin));
    sendEpdCommand(0x24);
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 1);
    epdTransmit(buffer, static_cast<size_t>(widthBytes) * static_cast<size_t>(height));

    const bool full_screen_update = (xStart == 0 && xEnd == static_cast<uint8_t>((PANEL_WIDTH / 8) - 1) && yMin == 0 && yMax == static_cast<int>(PANEL_HEIGHT - 1));
    if (full_screen_update)
    {
        epdRefresh();
    }
    else
    {
        epdRefreshPartial();
    }

    // Update previous buffer (0x26) to match current — keeps internal state for future updates
    setEpdRamArea(xStart, xEnd, static_cast<uint16_t>(yMin), static_cast<uint16_t>(yMax));
    setEpdRamPointer(xStart, static_cast<uint16_t>(yMin));
    sendEpdCommand(0x26);
    gpio_set_level(static_cast<gpio_num_t>(PIN_EPD_DC_CFG), 1);
    epdTransmit(buffer, static_cast<size_t>(widthBytes) * static_cast<size_t>(height));

    free(buffer);
}
#endif
