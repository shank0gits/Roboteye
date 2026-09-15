#include "ServoEngine.h"

#include "esp_log.h"

#include <algorithm>
#include <cmath>


static const char* TAG = "ServoEngine";


//====================================================
// Constructor
//====================================================

ServoEngine::ServoEngine(
    gpio_num_t panPin_,
    gpio_num_t tiltPin_
)
    : panPin(panPin_),
      tiltPin(tiltPin_)
{
}


//====================================================
// Begin
//====================================================

bool ServoEngine::begin()
{
    ESP_LOGI(
        TAG,
        "Initializing Servo Engine..."
    );


    //--------------------------------------------------
    // Configure LEDC Timer
    //--------------------------------------------------

    ledc_timer_config_t timerConfig = {};

    timerConfig.speed_mode =
        LEDC_LOW_SPEED_MODE;

    timerConfig.timer_num =
        SERVO_TIMER;

    timerConfig.duty_resolution =
        (ledc_timer_bit_t)SERVO_RESOLUTION_BITS;

    timerConfig.freq_hz =
        SERVO_FREQUENCY;

    timerConfig.clk_cfg =
        LEDC_AUTO_CLK;


    esp_err_t result =
        ledc_timer_config(
            &timerConfig
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to configure LEDC timer"
        );

        return false;
    }


    //--------------------------------------------------
    // Configure Pan Channel
    //--------------------------------------------------

    ledc_channel_config_t panConfig = {};

    panConfig.gpio_num =
        panPin;

    panConfig.speed_mode =
        LEDC_LOW_SPEED_MODE;

    panConfig.channel =
        PAN_CHANNEL;

    panConfig.intr_type =
        LEDC_INTR_DISABLE;

    panConfig.timer_sel =
        SERVO_TIMER;

    panConfig.duty =
        0;

    panConfig.hpoint =
        0;


    result =
        ledc_channel_config(
            &panConfig
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to configure Pan Servo"
        );

        return false;
    }


    //--------------------------------------------------
    // Configure Tilt Channel
    //--------------------------------------------------

    ledc_channel_config_t tiltConfig = {};

    tiltConfig.gpio_num =
        tiltPin;

    tiltConfig.speed_mode =
        LEDC_LOW_SPEED_MODE;

    tiltConfig.channel =
        TILT_CHANNEL;

    tiltConfig.intr_type =
        LEDC_INTR_DISABLE;

    tiltConfig.timer_sel =
        SERVO_TIMER;

    tiltConfig.duty =
        0;

    tiltConfig.hpoint =
        0;


    result =
        ledc_channel_config(
            &tiltConfig
        );


    if (result != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to configure Tilt Servo"
        );

        return false;
    }


    //--------------------------------------------------
    // Initialize Positions
    //--------------------------------------------------

    currentPan =
        PAN_CENTER;

    currentTilt =
        TILT_CENTER;

    targetPan =
        PAN_CENTER;

    targetTilt =
        TILT_CENTER;


    //--------------------------------------------------
    // Move To Center
    //--------------------------------------------------

    writePan(
        currentPan
    );

    writeTilt(
        currentTilt
    );


    ready = true;


    ESP_LOGI(
        TAG,
        "Servo Engine Ready"
    );

    ESP_LOGI(
        TAG,
        "Pan GPIO  : %d",
        panPin
    );

    ESP_LOGI(
        TAG,
        "Tilt GPIO : %d",
        tiltPin
    );


    return true;
}


//====================================================
// Update
//====================================================

void ServoEngine::update()
{
    if (!ready)
    {
        return;
    }


    //--------------------------------------------------
    // Smooth Pan
    //--------------------------------------------------

    currentPan =
        smoothStep(
            currentPan,
            targetPan
        );


    //--------------------------------------------------
    // Smooth Tilt
    //--------------------------------------------------

    currentTilt =
        smoothStep(
            currentTilt,
            targetTilt
        );


    //--------------------------------------------------
    // Write Servo Positions
    //--------------------------------------------------

    writePan(
        currentPan
    );

    writeTilt(
        currentTilt
    );
}


//====================================================
// Set Target
//====================================================

void ServoEngine::setTarget(
    float x,
    float y
)
{
    if (!ready)
    {
        return;
    }


    //--------------------------------------------------
    // Clamp Face Coordinates
    //--------------------------------------------------

    x =
        std::clamp(
            x,
            -10.0f,
            10.0f
        );

    y =
        std::clamp(
            y,
            -6.0f,
            6.0f
        );


    //--------------------------------------------------
    // Convert X To Pan Angle
    //--------------------------------------------------

    targetPan =
        PAN_CENTER
        +
        (
            x
            /
            10.0f
        )
        *
        (
            PAN_MAX
            -
            PAN_CENTER
        );


    //--------------------------------------------------
    // Convert Y To Minimal Tilt
    //--------------------------------------------------

    targetTilt =
        TILT_CENTER
        +
        (
            y
            /
            6.0f
        )
        *
        (
            TILT_MAX
            -
            TILT_CENTER
        );


    //--------------------------------------------------
    // Safety Limits
    //--------------------------------------------------

    targetPan =
        std::clamp(
            targetPan,
            PAN_MIN,
            PAN_MAX
        );

    targetTilt =
        std::clamp(
            targetTilt,
            TILT_MIN,
            TILT_MAX
        );
}


//====================================================
// Center
//====================================================

void ServoEngine::center()
{
    if (!ready)
    {
        return;
    }


    targetPan =
        PAN_CENTER;

    targetTilt =
        TILT_CENTER;
}


//====================================================
// Is Ready
//====================================================

bool ServoEngine::isReady() const
{
    return ready;
}


//====================================================
// Write Pan
//====================================================

void ServoEngine::writePan(
    float angle
)
{
    const uint32_t duty =
        angleToDuty(
            angle
        );


    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        PAN_CHANNEL,
        duty
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        PAN_CHANNEL
    );
}


//====================================================
// Write Tilt
//====================================================

void ServoEngine::writeTilt(
    float angle
)
{
    const uint32_t duty =
        angleToDuty(
            angle
        );


    ledc_set_duty(
        LEDC_LOW_SPEED_MODE,
        TILT_CHANNEL,
        duty
    );

    ledc_update_duty(
        LEDC_LOW_SPEED_MODE,
        TILT_CHANNEL
    );
}


//====================================================
// Convert Angle To PWM Duty
//====================================================

uint32_t ServoEngine::angleToDuty(
    float angle
) const
{
    //--------------------------------------------------
    // Standard Servo Pulse
    //
    // 0°   = 500 us
    // 180° = 2500 us
    //
    // Frequency = 50 Hz
    // Period    = 20 ms
    //--------------------------------------------------

    angle =
        std::clamp(
            angle,
            0.0f,
            180.0f
        );


    const float pulseUs =
        500.0f
        +
        (
            angle
            /
            180.0f
        )
        *
        2000.0f;


    const float periodUs =
        20000.0f;


    const float maxDuty =
        (1 << SERVO_RESOLUTION_BITS) - 1;


    const float duty =
        (
            pulseUs
            /
            periodUs
        )
        *
        maxDuty;


    return static_cast<uint32_t>(
        duty
    );
}


//====================================================
// Smooth Movement
//====================================================

float ServoEngine::smoothStep(
    float current,
    float target
) const
{
    //--------------------------------------------------
    // Smoothness
    //
    // Higher = Faster
    // Lower  = Smoother
    //--------------------------------------------------

    constexpr float smoothing =
        0.12f;


    const float difference =
        target
        -
        current;


    //--------------------------------------------------
    // Stop Tiny Movements
    //--------------------------------------------------

    if (
        fabsf(
            difference
        )
        <
        0.05f
    )
    {
        return target;
    }


    return
        current
        +
        (
            difference
            *
            smoothing
        );
}