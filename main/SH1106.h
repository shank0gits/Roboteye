#ifndef SH1106_H
#define SH1106_H

#include <stdint.h>
#include <stdbool.h>

#include "driver/i2c_master.h"

class SH1106
{
public:

    //------------------------------------
    // Constructor
    //------------------------------------

    SH1106(
    uint8_t address = 0x3C
);

    //------------------------------------
    // Initialization
    //------------------------------------

    bool begin(
    i2c_master_bus_handle_t bus
);

    void clear();

    void display();

    //------------------------------------
    // Basic Drawing
    //------------------------------------

    void drawPixel(
        int x,
        int y,
        bool color = true
    );

    void drawFastHLine(
        int x,
        int y,
        int w,
        bool color = true
    );

    void drawFastVLine(
        int x,
        int y,
        int h,
        bool color = true
    );

    void drawLine(
        int x0,
        int y0,
        int x1,
        int y1,
        bool color = true
    );

    void drawRect(
        int x,
        int y,
        int w,
        int h,
        bool color = true
    );

    void fillRect(
        int x,
        int y,
        int w,
        int h,
        bool color = true
    );

    void drawCircle(
        int x0,
        int y0,
        int radius,
        bool color = true
    );

    void fillCircle(
        int x0,
        int y0,
        int radius,
        bool color = true
    );

    //------------------------------------
    // Display Size
    //------------------------------------

    static constexpr int WIDTH  = 128;
    static constexpr int HEIGHT = 64;

private:

    //------------------------------------
    // I2C
    //------------------------------------

    i2c_master_bus_handle_t busHandle = nullptr;

    i2c_master_dev_handle_t displayHandle;

    uint8_t i2cAddress;

    //------------------------------------
    // Frame Buffer
    //------------------------------------

    uint8_t buffer[WIDTH * HEIGHT / 8];

    //------------------------------------
    // Internal Functions
    //------------------------------------

    bool initDisplay();

    bool sendCommand(uint8_t cmd);

    bool sendCommands(
        const uint8_t *cmds,
        size_t length
    );

    bool sendData(
        const uint8_t *data,
        size_t length
    );
};

#endif