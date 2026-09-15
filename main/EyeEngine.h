#ifndef EYE_ENGINE_H
#define EYE_ENGINE_H

#include <Arduino.h>

#include "SH1106.h"
#include "Graphics.h"


class EyeEngine
{
public:

    //------------------------------------
    // Eye Movement Range
    //------------------------------------

    static constexpr float MAX_X = 20.0f;
    static constexpr float MAX_Y = 12.0f;


    //------------------------------------
    // Constructor
    //------------------------------------

    explicit EyeEngine(SH1106 &display);


    //------------------------------------
    // Engine
    //------------------------------------

    void begin();

    void update();

    void draw();


    //------------------------------------
    // Eye Control
    //------------------------------------

    void look(float x, float y);

    void setTarget(float x, float y);

    void center();


    //------------------------------------
    // Tracking
    //------------------------------------

    void enableTracking(bool state);

    bool trackingEnabled() const;


    //------------------------------------
    // Blink
    //------------------------------------

    void blink();


    //------------------------------------
    // Idle
    //------------------------------------

    void enableIdle(bool state);


    //------------------------------------
    // Current Position
    //------------------------------------

    float getX() const;

    float getY() const;


    //------------------------------------
    // Target Position
    //------------------------------------

    float getTargetX() const;

    float getTargetY() const;


private:

    //------------------------------------
    // Graphics Engine
    //------------------------------------

    Graphics graphics;


    //------------------------------------
    // Current Eye Position
    //------------------------------------

    float pupilX = 0.0f;

    float pupilY = 0.0f;


    //------------------------------------
    // Target Position
    //------------------------------------

    float targetX = 0.0f;

    float targetY = 0.0f;


    //------------------------------------
    // Tracking
    //------------------------------------

    bool tracking = false;


    //------------------------------------
    // Blink
    //------------------------------------

    int blinkHeight = 0;

    bool blinking = false;


    //------------------------------------
    // Idle
    //------------------------------------

    bool idleEnabled = true;

    unsigned long lastBlink = 0;

    unsigned long lastMove = 0;


    //------------------------------------
    // Internal Functions
    //------------------------------------

    void smoothMovement();

    void randomMovement();

};

#endif