#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "SH1106.h"
#include "EyeEngine.h"
#include "CameraEngine.h"
#include "FaceTracker.h"
#include "VisionEngine.h"


//--------------------------------------------------
// GPIO
//--------------------------------------------------

#define SDA_PIN     ((gpio_num_t)46)
#define SCL_PIN     ((gpio_num_t)14)

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


//--------------------------------------------------
// Engines
//--------------------------------------------------

EyeEngine eye(display);

CameraEngine camera;

FaceTracker tracker;

VisionEngine vision(camera, tracker);


//--------------------------------------------------
// Main
//--------------------------------------------------

extern "C" void app_main(void)
{
    //--------------------------------------------------
    // Configure I2C Bus
    //--------------------------------------------------

    ESP_LOGI(TAG, "Starting I2C Bus...");

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

    ESP_LOGI(TAG, "I2C Bus Ready");


    //--------------------------------------------------
    // Initialize OLED
    //--------------------------------------------------

    ESP_LOGI(TAG, "Starting SH1106 OLED...");

    if (!display.begin(busHandle))
    {
        ESP_LOGE(
            TAG,
            "OLED initialization failed!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    ESP_LOGI(
        TAG,
        "SH1106 OLED Ready"
    );


    //--------------------------------------------------
    // Initialize Eye Engine
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Eye Engine..."
    );

    eye.begin();

    // Enable face tracking control
    eye.enableTracking(true);

    ESP_LOGI(
        TAG,
        "Eye Engine Started"
    );


    //--------------------------------------------------
    // Initialize Camera
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Camera..."
    );

    if (!camera.begin())
    {
        ESP_LOGE(
            TAG,
            "Camera initialization failed!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    ESP_LOGI(
        TAG,
        "Camera Ready"
    );


    //--------------------------------------------------
    // Initialize Vision Engine
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Vision Engine..."
    );

    if (!vision.begin())
    {
        ESP_LOGE(
            TAG,
            "Vision Engine initialization failed!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    ESP_LOGI(
        TAG,
        "Vision Engine Started"
    );


    //--------------------------------------------------
    // System Ready
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "Robot Eye System Ready"
    );

    ESP_LOGI(
        TAG,
        "Face Tracking Active"
    );

    ESP_LOGI(
        TAG,
        "================================"
    );


    //--------------------------------------------------
    // Main Loop
    //--------------------------------------------------

    while (true)
    {
        //--------------------------------------------------
        // Capture Camera Frame
        // Process Face Detection
        //--------------------------------------------------

        vision.update();


        //--------------------------------------------------
        // Face Tracking
        //--------------------------------------------------

        if (tracker.faceDetected())
        {
            //--------------------------------------------------
            // Face Found
            //--------------------------------------------------

            float eyeTargetX =
                tracker.getEyeX();

            float eyeTargetY =
                tracker.getEyeY();


            //--------------------------------------------------
            // Move Eye Toward Face
            //--------------------------------------------------

            eye.setTarget(
                eyeTargetX,
                eyeTargetY
            );
        }
        else
        {
            //--------------------------------------------------
            // No Face
            // Return Eye to Center
            //--------------------------------------------------

            eye.center();
        }


        //--------------------------------------------------
        // Update Eye Animation
        //--------------------------------------------------

        eye.update();


        //--------------------------------------------------
        // Draw Eye on 0.96 inch SH1106 OLED
        //--------------------------------------------------

        eye.draw();


        //--------------------------------------------------
        // Loop Delay
        //--------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(20)
        );
    }
}