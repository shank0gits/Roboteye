#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "SH1106.h"
#include "EyeEngine.h"

#define SDA_PIN     ((gpio_num_t)22)
#define SCL_PIN     ((gpio_num_t)23)

#define I2C_PORT    I2C_NUM_0

static const char *TAG = "RobotEye";

//--------------------------------------------------
// I2C
//--------------------------------------------------

i2c_master_bus_handle_t busHandle = nullptr;

//--------------------------------------------------
// Display
//--------------------------------------------------

SH1106 display;

EyeEngine eye(display);

//--------------------------------------------------
// Main
//--------------------------------------------------

extern "C" void app_main(void)
{
    //------------------------------------
    // Configure I2C Bus
    //------------------------------------

    i2c_master_bus_config_t bus_config = {};

    bus_config.i2c_port = I2C_PORT;
    bus_config.sda_io_num = SDA_PIN;
    bus_config.scl_io_num = SCL_PIN;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &busHandle
        )
    );

    //------------------------------------
    // Initialize Display
    //------------------------------------

    if (!display.begin(busHandle))
    {
        ESP_LOGE(TAG, "OLED initialization failed");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    //------------------------------------
    // Eye Engine
    //------------------------------------

    eye.begin();

    ESP_LOGI(TAG, "Eye Engine Started");

    //------------------------------------
    // Main Loop
    //------------------------------------

    while (true)
    {
        eye.update();

        eye.draw();

        vTaskDelay(pdMS_TO_TICKS(20));     // ~50 FPS
    }
}