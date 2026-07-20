#ifndef EYE_ENGINE_H
#define EYE_ENGINE_H

#include <Arduino.h>

#include "SH1106.h"
#include "Graphics.h"

class EyeEngine
{
public:

    EyeEngine(SH1106 &display);

    void begin();

    void update();

    void draw();

    void look(float x, float y);

    void center();

    void blink();

    void enableIdle(bool state);

private:

    Graphics graphics;

    //------------------------------------
    // Eye Position
    //------------------------------------

    float pupilX;
    float pupilY;

    float targetX;
    float targetY;

    //------------------------------------
    // Blink
    //------------------------------------

    int blinkHeight;

    bool blinking;

    //------------------------------------
    // Idle Movement
    //------------------------------------

    bool idleEnabled;

    unsigned long lastBlink;

    unsigned long lastMove;

    //------------------------------------
    // Internal
    //------------------------------------

    void smoothMovement();

    void randomMovement();
};

#endif