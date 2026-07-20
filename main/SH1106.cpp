#include "SH1106.h"

#include <string.h>

#include "esp_log.h"

static const char *TAG = "SH1106";

//----------------------------------------------------
// Constructor
//----------------------------------------------------

SH1106::SH1106(uint8_t address)
{
    busHandle = nullptr;

    i2cAddress = address;

    displayHandle = nullptr;

    memset(buffer, 0, sizeof(buffer));
}

//----------------------------------------------------
// Initialize Display
//----------------------------------------------------

bool SH1106::begin(
    i2c_master_bus_handle_t bus)
{
    //------------------------------------
    // Save bus handle
    //------------------------------------

    busHandle = bus;

    if(busHandle == nullptr)
    {
        ESP_LOGE(TAG, "Invalid I2C bus");

        return false;
    }

    //------------------------------------
    // Configure device
    //------------------------------------

    i2c_device_config_t devConfig = {};

    devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;

    devConfig.device_address = i2cAddress;

    devConfig.scl_speed_hz = 400000;

    esp_err_t err =
        i2c_master_bus_add_device(
            busHandle,
            &devConfig,
            &displayHandle);

    if(err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SH1106 device");

        return false;
    }

    //------------------------------------
    // Initialize OLED
    //------------------------------------

    if(!initDisplay())
    {
        ESP_LOGE(TAG, "Display initialization failed");

        return false;
    }

    clear();

    display();

    ESP_LOGI(TAG, "SH1106 Ready");

    return true;
}

//----------------------------------------------------
// Initialize Controller
//----------------------------------------------------

bool SH1106::initDisplay()
{
    static const uint8_t initCmds[] =
    {
        0xAE,         // Display OFF

        0xD5, 0x80,   // Clock Divide

        0xA8, 0x3F,   // Multiplex Ratio

        0xD3, 0x00,   // Display Offset

        0x40,         // Start Line

        0xAD, 0x8B,   // DC/DC Enable

        0xA1,         // Segment Remap

        0xC8,         // Scan Direction

        0xDA, 0x12,   // COM Pins

        0x81, 0xCF,   // Contrast

        0xD9, 0xF1,   // Precharge

        0xDB, 0x40,   // VCOM Detect

        0xA4,         // Resume RAM

        0xA6,         // Normal Display

        0xAF          // Display ON
    };

    return sendCommands(
        initCmds,
        sizeof(initCmds));
}

//----------------------------------------------------
// Send Single Command
//----------------------------------------------------

bool SH1106::sendCommand(uint8_t cmd)
{
    uint8_t buffer[2];

    buffer[0] = 0x00;

    buffer[1] = cmd;

    esp_err_t err =
        i2c_master_transmit(
            displayHandle,
            buffer,
            sizeof(buffer),
            -1);

    return err == ESP_OK;
}

//----------------------------------------------------
// Send Multiple Commands
//----------------------------------------------------

bool SH1106::sendCommands(
    const uint8_t *cmds,
    size_t length)
{
    for(size_t i=0;i<length;i++)
    {
        if(!sendCommand(cmds[i]))
        {
            return false;
        }
    }

    return true;
}

//----------------------------------------------------
// Send Display Data
//----------------------------------------------------

bool SH1106::sendData(
    const uint8_t *data,
    size_t length)
{
    static uint8_t txBuffer[129];

    if(length > 128)
    {
        return false;
    }

    txBuffer[0] = 0x40;

    memcpy(
        &txBuffer[1],
        data,
        length);

    esp_err_t err =
        i2c_master_transmit(
            displayHandle,
            txBuffer,
            length + 1,
            -1);

    return err == ESP_OK;
}
//----------------------------------------------------
// Clear Frame Buffer
//----------------------------------------------------

void SH1106::clear()
{
    memset(buffer, 0, sizeof(buffer));
}

//----------------------------------------------------
// Draw Pixel
//----------------------------------------------------

void SH1106::drawPixel(
    int x,
    int y,
    bool color)
{
    if(x < 0 || x >= WIDTH)
        return;

    if(y < 0 || y >= HEIGHT)
        return;

    uint16_t index = x + (y / 8) * WIDTH;

    uint8_t mask = 1 << (y & 7);

    if(color)
    {
        buffer[index] |= mask;
    }
    else
    {
        buffer[index] &= ~mask;
    }
}

//----------------------------------------------------
// Update Display
//----------------------------------------------------

void SH1106::display()
{
    for(uint8_t page = 0; page < 8; page++)
    {
        sendCommand(0xB0 + page);

        sendCommand(0x02);      // Lower column

        sendCommand(0x10);      // Higher column

        sendData(
            &buffer[page * WIDTH],
            WIDTH
        );
    }
}
//----------------------------------------------------
// Draw Horizontal Line
//----------------------------------------------------

void SH1106::drawFastHLine(
    int x,
    int y,
    int w,
    bool color)
{
    if(y < 0 || y >= HEIGHT)
        return;

    for(int i = 0; i < w; i++)
    {
        drawPixel(x + i, y, color);
    }
}

//----------------------------------------------------
// Draw Vertical Line
//----------------------------------------------------

void SH1106::drawFastVLine(
    int x,
    int y,
    int h,
    bool color)
{
    if(x < 0 || x >= WIDTH)
        return;

    for(int i = 0; i < h; i++)
    {
        drawPixel(x, y + i, color);
    }
}

//----------------------------------------------------
// Draw Line (Bresenham)
//----------------------------------------------------

void SH1106::drawLine(
    int x0,
    int y0,
    int x1,
    int y1,
    bool color)
{
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;

    int dy = -abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    while(true)
    {
        drawPixel(x0, y0, color);

        if(x0 == x1 && y0 == y1)
            break;

        int e2 = 2 * err;

        if(e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if(e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}
//----------------------------------------------------
// Draw Rectangle
//----------------------------------------------------

void SH1106::drawRect(
    int x,
    int y,
    int w,
    int h,
    bool color)
{
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y + h - 1, w, color);

    drawFastVLine(x, y, h, color);
    drawFastVLine(x + w - 1, y, h, color);
}

//----------------------------------------------------
// Fill Rectangle
//----------------------------------------------------

void SH1106::fillRect(
    int x,
    int y,
    int w,
    int h,
    bool color)
{
    for(int i = 0; i < h; i++)
    {
        drawFastHLine(
            x,
            y + i,
            w,
            color
        );
    }
}

//----------------------------------------------------
// Draw Circle
//----------------------------------------------------

void SH1106::drawCircle(
    int x0,
    int y0,
    int radius,
    bool color)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while(x >= y)
    {
        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 - x, y0 + y, color);

        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 - y, y0 - x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 + x, y0 - y, color);

        if(err <= 0)
        {
            y++;
            err += 2 * y + 1;
        }

        if(err > 0)
        {
            x--;
            err -= 2 * x + 1;
        }
    }
}

//----------------------------------------------------
// Fill Circle
//----------------------------------------------------

void SH1106::fillCircle(
    int x0,
    int y0,
    int radius,
    bool color)
{
    for(int y = -radius; y <= radius; y++)
    {
        for(int x = -radius; x <= radius; x++)
        {
            if((x * x + y * y) <= radius * radius)
            {
                drawPixel(
                    x0 + x,
                    y0 + y,
                    color
                );
            }
        }
    }
}