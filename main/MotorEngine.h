#ifndef MOTOR_ENGINE_H
#define MOTOR_ENGINE_H

#include <cstdint>
#include "driver/gpio.h"
#include "driver/ledc.h"

// Two-input H-bridge controller for continuous geared motors.
// Position is estimated from run time until an encoder/limit sensor is added.
class MotorEngine {
public:
    MotorEngine(gpio_num_t a1a, gpio_num_t a1b);
    bool begin();
    void update();
    void setTarget(float x, float y);
    void center();
    bool isReady() const;

private:
    gpio_num_t a1a_, a1b_;
    bool ready_ = false;
    float pan_ = 0.0f;
    float pan_target_ = 0.0f;
    int64_t last_update_us_ = 0;
    int last_direction_ = 99;
    int requested_direction_ = 0;
    int active_direction_ = 0;
    int64_t pulse_until_us_ = 0;
    int64_t next_pulse_us_ = 0;

    // HG7881/L9110 input PWM: lower frequency gives cleaner bench testing.
    static constexpr uint32_t PWM_FREQUENCY = 700;
    static constexpr uint32_t PWM_BITS = 10;
    static constexpr uint32_t SPEED_PERCENT = 60;
    static constexpr float PAN_MIN = -100.0f;
    static constexpr float PAN_MAX = 100.0f;
    // Calibrate these for the actual gearbox, load and supply voltage.
    static constexpr float PAN_DEGREES_PER_SECOND = 35.0f;
    static constexpr float DEADBAND = 3.0f;
    static constexpr int64_t PULSE_DURATION_US = 60000;
    static constexpr int64_t PULSE_PAUSE_US = 60000;
    static constexpr ledc_timer_t TIMER = LEDC_TIMER_1;
    static constexpr ledc_channel_t A1A_CHANNEL = LEDC_CHANNEL_2;
    static constexpr ledc_channel_t A1B_CHANNEL = LEDC_CHANNEL_3;

    uint32_t duty() const;
    void stop(ledc_channel_t first, ledc_channel_t second);
    void drive(ledc_channel_t first, ledc_channel_t second, int direction);
};

#endif
