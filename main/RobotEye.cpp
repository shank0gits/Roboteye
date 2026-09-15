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
#include "VisionTask.h"
#include "ServoEngine.h"


//==================================================
// GPIO
//==================================================

#define SDA_PIN     ((gpio_num_t)46)
#define SCL_PIN     ((gpio_num_t)14)

#define I2C_PORT    I2C_NUM_0


//--------------------------------------------------
// Servo GPIO
//--------------------------------------------------

#define PAN_SERVO_PIN     ((gpio_num_t)20)
#define TILT_SERVO_PIN    ((gpio_num_t)21)


//--------------------------------------------------
// Brain Wake GPIO
// RobotEye GPIO19 -> Brain GPIO1
//--------------------------------------------------

#define WAKE_GPIO         ((gpio_num_t)19)


//==================================================
// Wake Settings
//==================================================

// GPIO19 HIGH duration when wake is triggered
#define WAKE_PULSE_MS     150

// Wake lock duration
// After 2 minutes another wake can happen
#define WAKE_RESET_MS     120000


static const char* TAG =
    "RobotEye";


//==================================================
// I2C
//==================================================

i2c_master_bus_handle_t busHandle =
    nullptr;


//==================================================
// Display
//==================================================

SH1106 display;


//==================================================
// Engines
//==================================================

EyeEngine eye(
    display
);


CameraEngine camera;


FaceTracker tracker;


VisionEngine vision(
    camera,
    tracker,
    eye
);


VisionTask visionTask(
    vision
);


//==================================================
// Servo Engine
//==================================================

ServoEngine servos(
    PAN_SERVO_PIN,
    TILT_SERVO_PIN
);


//==================================================
// Application Main
//==================================================

extern "C"
void app_main(
    void
)
{
    //--------------------------------------------------
    // Start I2C
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting I2C Bus..."
    );


    //--------------------------------------------------
    // Brain ESP Wake GPIO
    // GPIO19 -> Brain GPIO1
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Initializing Wake GPIO..."
    );


    gpio_config_t wake_config = {};


    wake_config.pin_bit_mask =
        (1ULL << WAKE_GPIO);


    wake_config.mode =
        GPIO_MODE_OUTPUT;


    wake_config.pull_up_en =
        GPIO_PULLUP_DISABLE;


    wake_config.pull_down_en =
        GPIO_PULLDOWN_DISABLE;


    wake_config.intr_type =
        GPIO_INTR_DISABLE;


    ESP_ERROR_CHECK(
        gpio_config(
            &wake_config
        )
    );


    //--------------------------------------------------
    // Keep GPIO19 LOW at boot
    //--------------------------------------------------

    gpio_set_level(
        WAKE_GPIO,
        0
    );


    ESP_LOGI(
        TAG,
        "Wake GPIO Ready - GPIO19"
    );


    //--------------------------------------------------
    // I2C Configuration
    //--------------------------------------------------

    i2c_master_bus_config_t bus_config = {};


    bus_config.i2c_port =
        I2C_PORT;


    bus_config.sda_io_num =
        SDA_PIN;


    bus_config.scl_io_num =
        SCL_PIN;


    bus_config.clk_source =
        I2C_CLK_SRC_DEFAULT;


    bus_config.glitch_ignore_cnt =
        7;


    bus_config.flags.enable_internal_pullup =
        true;


    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &busHandle
        )
    );


    ESP_LOGI(
        TAG,
        "I2C Bus Ready"
    );


    //--------------------------------------------------
    // Start OLED
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting SH1106 OLED..."
    );


    if (
        !display.begin(
            busHandle
        )
    )
    {
        ESP_LOGE(
            TAG,
            "OLED initialization failed!"
        );


        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    1000
                )
            );
        }
    }


    ESP_LOGI(
        TAG,
        "SH1106 OLED Ready"
    );


    //--------------------------------------------------
    // Start Eye Engine
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Eye Engine..."
    );


    eye.begin();


    ESP_LOGI(
        TAG,
        "Eye Engine Started"
    );


    //--------------------------------------------------
    // Start Servo Engine
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Servo Engine..."
    );


    if (
        !servos.begin()
    )
    {
        ESP_LOGE(
            TAG,
            "Servo Engine initialization failed!"
        );


        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    1000
                )
            );
        }
    }


    ESP_LOGI(
        TAG,
        "Servo Engine Started"
    );


    //--------------------------------------------------
    // Start Camera
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Camera..."
    );


    if (
        !camera.begin()
    )
    {
        ESP_LOGE(
            TAG,
            "Camera initialization failed!"
        );


        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    1000
                )
            );
        }
    }


    ESP_LOGI(
        TAG,
        "Camera Ready"
    );


    //--------------------------------------------------
    // Start Vision Engine
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Vision Engine..."
    );


    if (
        !vision.begin()
    )
    {
        ESP_LOGE(
            TAG,
            "Vision Engine initialization failed!"
        );


        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    1000
                )
            );
        }
    }


    ESP_LOGI(
        TAG,
        "Vision Engine Started"
    );


    //--------------------------------------------------
    // Start Vision Task
    //--------------------------------------------------

    ESP_LOGI(
        TAG,
        "Starting Vision Task..."
    );


    if (
        !visionTask.begin()
    )
    {
        ESP_LOGE(
            TAG,
            "Vision Task initialization failed!"
        );


        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(
                    1000
                )
            );
        }
    }


    ESP_LOGI(
        TAG,
        "Vision Task Started"
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
        "Pan Servo GPIO  : %d",
        PAN_SERVO_PIN
    );


    ESP_LOGI(
        TAG,
        "Tilt Servo GPIO : %d",
        TILT_SERVO_PIN
    );


    ESP_LOGI(
        TAG,
        "Wake GPIO       : %d",
        WAKE_GPIO
    );


    ESP_LOGI(
        TAG,
        "Wake Pulse      : %d ms",
        WAKE_PULSE_MS
    );


    ESP_LOGI(
        TAG,
        "Wake Reset      : %d ms",
        WAKE_RESET_MS
    );


    ESP_LOGI(
        TAG,
        "================================"
    );


    //==================================================
    // Wake State
    //==================================================

    // True after a wake has been triggered.
    // Prevents repeated triggers while locked.
    static bool wakeTriggered =
        false;


    // True while GPIO19 is in its 150ms HIGH pulse.
    static bool wakePulseActive =
        false;


    // Time at which GPIO19 went HIGH.
    static TickType_t wakePulseStart =
        0;


    // Time at which the 2-minute wake lock started.
    static TickType_t wakeResetStart =
        0;


    //==================================================
    // Main Loop
    //==================================================

    while (true)
    {
        //------------------------------------------------
        // Current Face State
        //------------------------------------------------

        const bool currentFaceState =
            tracker.faceDetected();


        //================================================
        // FACE DETECTED
        //================================================
        //
        // Only triggers if wakeTriggered == false.
        //
        // Face gone is NOT used for resetting.
        //
        // The reset happens only after 2 minutes.
        //================================================

        if (
            currentFaceState &&
            !wakeTriggered
        )
        {
            //------------------------------------------------
            // GPIO19 HIGH
            //------------------------------------------------

            gpio_set_level(
                WAKE_GPIO,
                1
            );


            //------------------------------------------------
            // Start 150ms pulse timer
            //------------------------------------------------

            wakePulseStart =
                xTaskGetTickCount();


            wakePulseActive =
                true;


            //------------------------------------------------
            // Lock wake detection
            //------------------------------------------------

            wakeTriggered =
                true;


            //------------------------------------------------
            // Start 2-minute reset timer
            //------------------------------------------------

            wakeResetStart =
                xTaskGetTickCount();


            ESP_LOGI(
                TAG,
                "FACE DETECTED -> GPIO19 HIGH"
            );
        }


        //================================================
        // END 150ms WAKE PULSE
        //================================================

        if (
            wakePulseActive &&
            (
                xTaskGetTickCount() -
                wakePulseStart
            ) >=
            pdMS_TO_TICKS(
                WAKE_PULSE_MS
            )
        )
        {
            //------------------------------------------------
            // GPIO19 LOW
            //------------------------------------------------

            gpio_set_level(
                WAKE_GPIO,
                0
            );


            wakePulseActive =
                false;


            ESP_LOGI(
                TAG,
                "WAKE PULSE COMPLETE -> GPIO19 LOW"
            );
        }


        //================================================
        // 2 MINUTE WAKE RESET
        //================================================
        //
        // Face state is completely ignored here.
        //
        // After 2 minutes:
        // wakeTriggered becomes false.
        //
        // If a face is still detected, the next loop
        // can immediately trigger another wake pulse.
        //================================================

        if (
            wakeTriggered &&
            (
                xTaskGetTickCount() -
                wakeResetStart
            ) >=
            pdMS_TO_TICKS(
                WAKE_RESET_MS
            )
        )
        {
            //------------------------------------------------
            // Unlock wake detection
            //------------------------------------------------

            wakeTriggered =
                false;


            //------------------------------------------------
            // Make sure GPIO19 is LOW
            //------------------------------------------------

            gpio_set_level(
                WAKE_GPIO,
                0
            );


            ESP_LOGI(
                TAG,
                "2 MINUTES COMPLETE -> WAKE RESET"
            );
        }


        //================================================
        // EXISTING FACE TRACKING
        //================================================

        if (
            tracker.faceDetected()
        )
        {
            //--------------------------------------------
            // Get Face Tracking Target
            //--------------------------------------------

            const float faceX =
                tracker.getEyeX();


            const float faceY =
                tracker.getEyeY();


            //--------------------------------------------
            // Move OLED Eye
            //--------------------------------------------

            eye.setTarget(
                faceX,
                faceY
            );


            //--------------------------------------------
            // Move Pan + Tilt Servos
            //--------------------------------------------

            servos.setTarget(
                faceX,
                faceY
            );
        }


        //================================================
        // FACE LOST
        //================================================

        else
        {
            //--------------------------------------------
            // Center OLED Eye
            //--------------------------------------------

            eye.center();


            //--------------------------------------------
            // Center Servos Smoothly
            //--------------------------------------------

            servos.center();
        }


        //================================================
        // Update OLED Eye
        //================================================

        eye.update();


        //================================================
        // Draw OLED Eye
        //================================================

        eye.draw();


        //================================================
        // Update Servos
        //================================================

        servos.update();


        //================================================
        // 50 FPS Main Loop
        //================================================

        vTaskDelay(
            pdMS_TO_TICKS(
                20
            )
        );
    }
}