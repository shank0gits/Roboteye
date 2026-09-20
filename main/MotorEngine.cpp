#include "MotorEngine.h"

#include <algorithm>
#include "esp_log.h"
#include "esp_timer.h"

static const char* TAG = "MotorEngine";

MotorEngine::MotorEngine(gpio_num_t a1a, gpio_num_t a1b)
    : a1a_(a1a), a1b_(a1b) {}

uint32_t MotorEngine::duty() const { return (1u << PWM_BITS) * SPEED_PERCENT / 100u; }

bool MotorEngine::begin() {
    ledc_timer_config_t timer = {};
    timer.speed_mode = LEDC_LOW_SPEED_MODE;
    timer.timer_num = TIMER;
    timer.duty_resolution = static_cast<ledc_timer_bit_t>(PWM_BITS);
    timer.freq_hz = PWM_FREQUENCY;
    timer.clk_cfg = LEDC_AUTO_CLK;
    if (ledc_timer_config(&timer) != ESP_OK) return false;

    const gpio_num_t pins[] = {a1a_, a1b_};
    const ledc_channel_t channels[] = {A1A_CHANNEL, A1B_CHANNEL};
    for (int i = 0; i < 2; ++i) {
        ledc_channel_config_t channel = {};
        channel.gpio_num = pins[i];
        channel.speed_mode = LEDC_LOW_SPEED_MODE;
        channel.channel = channels[i];
        channel.timer_sel = TIMER;
        if (ledc_channel_config(&channel) != ESP_OK) return false;
    }
    ready_ = true;
    last_update_us_ = esp_timer_get_time();
    last_direction_ = 99;
    requested_direction_ = 0;
    active_direction_ = 0;
    pulse_until_us_ = 0;
    next_pulse_us_ = 0;
    stop(A1A_CHANNEL, A1B_CHANNEL);
    ESP_LOGI(TAG, "Pan H-bridge ready: A-1A GPIO%d, A-1B GPIO%d, speed=%d%%",
             a1a_, a1b_, SPEED_PERCENT);
    ESP_LOGW(TAG, "Position is time-estimated; encoder/limit sensors are needed for exact angles");
    return true;
}

void MotorEngine::stop(ledc_channel_t first, ledc_channel_t second) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, first, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, first);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, second, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, second);
}

void MotorEngine::drive(ledc_channel_t first, ledc_channel_t second, int direction) {
    if (direction > 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, first, duty());
        ledc_set_duty(LEDC_LOW_SPEED_MODE, second, 0);
    } else if (direction < 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, first, 0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, second, duty());
    } else {
        stop(first, second);
        return;
    }
    ledc_update_duty(LEDC_LOW_SPEED_MODE, first);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, second);
}

void MotorEngine::setTarget(float x, float y) {
    if (!ready_) return;
    (void)y; // Vertical tracking remains OLED-only until a second motor is added.
    x = std::clamp(x, -20.0f, 20.0f);
    pan_target_ = x;
    requested_direction_ = x > DEADBAND ? 1 : x < -DEADBAND ? -1 : 0;
    if (requested_direction_ != active_direction_ && active_direction_ != 0) {
        active_direction_ = 0;
        pulse_until_us_ = 0;
        next_pulse_us_ = 0;
    }
}

void MotorEngine::center() {
    if (ready_) {
        pan_target_ = 0.0f;
        requested_direction_ = 0;
        active_direction_ = 0;
        pulse_until_us_ = 0;
        next_pulse_us_ = 0;
    }
}

void MotorEngine::update() {
    if (!ready_) return;
    const int64_t now = esp_timer_get_time();
    last_update_us_ = now;
    if (requested_direction_ == 0)
    {
        if (last_direction_ != 0) {
            ESP_LOGI(TAG, "Pan command=STOP target=%.1f", pan_target_);
            last_direction_ = 0;
        }
        active_direction_ = 0;
        drive(A1A_CHANNEL, A1B_CHANNEL, 0);
        return;
    }

    if (active_direction_ != 0 && now < pulse_until_us_)
    {
        drive(A1A_CHANNEL, A1B_CHANNEL, active_direction_);
        return;
    }

    if (active_direction_ != 0)
    {
        active_direction_ = 0;
        drive(A1A_CHANNEL, A1B_CHANNEL, 0);
    }

    if (now >= next_pulse_us_)
    {
        active_direction_ = requested_direction_;
        pulse_until_us_ = now + PULSE_DURATION_US;
        next_pulse_us_ = pulse_until_us_ + PULSE_PAUSE_US;
        const char* direction = active_direction_ > 0 ? "RIGHT" : "LEFT";
        ESP_LOGI(TAG, "Pan pulse=%s target=%.1f duration=%lldms",
                 direction, pan_target_, static_cast<long long>(PULSE_DURATION_US / 1000));
        drive(A1A_CHANNEL, A1B_CHANNEL, active_direction_);
    }
}

bool MotorEngine::isReady() const { return ready_; }
