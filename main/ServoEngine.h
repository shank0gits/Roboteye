#ifndef SERVO_ENGINE_H
#define SERVO_ENGINE_H

#include <stdint.h>

#include "driver/ledc.h"
#include "driver/gpio.h"

class ServoEngine
{
public:

    //--------------------------------------------------
    // Constructor
    //--------------------------------------------------

    ServoEngine(
        gpio_num_t panPin,
        gpio_num_t tiltPin
    );


    //--------------------------------------------------
    // Initialization
    //--------------------------------------------------

    bool begin();


    //--------------------------------------------------
    // Update Servo Movement
    //--------------------------------------------------

    void update();


    //--------------------------------------------------
    // Set Target From Face Tracking
    //--------------------------------------------------

    void setTarget(
        float x,
        float y
    );


    //--------------------------------------------------
    // Center Servos
    //--------------------------------------------------

    void center();


    //--------------------------------------------------
    // Check Initialization
    //--------------------------------------------------

    bool isReady() const;


private:

    //--------------------------------------------------
    // Servo Pins
    //--------------------------------------------------

    gpio_num_t panPin;
    gpio_num_t tiltPin;


    //--------------------------------------------------
    // Servo State
    //--------------------------------------------------

    bool ready = false;


    //--------------------------------------------------
    // Current Servo Angles
    //--------------------------------------------------

    float currentPan = 90.0f;
    float currentTilt = 90.0f;


    //--------------------------------------------------
    // Target Servo Angles
    //--------------------------------------------------

    float targetPan = 90.0f;
    float targetTilt = 90.0f;


    //--------------------------------------------------
    // Servo Limits
    //--------------------------------------------------

    static constexpr float PAN_MIN = 45.0f;
    static constexpr float PAN_MAX = 135.0f;

    static constexpr float TILT_MIN = 82.0f;
    static constexpr float TILT_MAX = 98.0f;


    //--------------------------------------------------
    // Servo Center
    //--------------------------------------------------

    static constexpr float PAN_CENTER = 90.0f;
    static constexpr float TILT_CENTER = 90.0f;


    //--------------------------------------------------
    // LEDC Configuration
    //--------------------------------------------------

    static constexpr uint32_t SERVO_FREQUENCY = 50;

    static constexpr uint32_t SERVO_RESOLUTION_BITS = 14;


    //--------------------------------------------------
    // LEDC Channels
    //--------------------------------------------------

    static constexpr ledc_channel_t PAN_CHANNEL =
        LEDC_CHANNEL_0;

    static constexpr ledc_channel_t TILT_CHANNEL =
        LEDC_CHANNEL_1;


    //--------------------------------------------------
    // LEDC Timer
    //--------------------------------------------------

    static constexpr ledc_timer_t SERVO_TIMER =
        LEDC_TIMER_0;


    //--------------------------------------------------
    // Internal Functions
    //--------------------------------------------------

    void writePan(
        float angle
    );

    void writeTilt(
        float angle
    );


    uint32_t angleToDuty(
        float angle
    ) const;


    float smoothStep(
        float current,
        float target
    ) const;
};

#endif