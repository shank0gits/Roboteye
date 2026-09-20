#include "EyeEngine.h"


//----------------------------------------------------
// Constructor
//----------------------------------------------------

EyeEngine::EyeEngine(
    SH1106 &display
)
    : graphics(display)
{
}


//----------------------------------------------------
// Begin
//----------------------------------------------------

void EyeEngine::begin()
{
    graphics.begin();

    randomSeed(
        micros()
    );

    lastBlink = millis();

    lastMove = millis();

    pupilX = 0.0f;

    pupilY = 0.0f;

    targetX = 0.0f;

    targetY = 0.0f;

    tracking = false;

    idleEnabled = true;

    blinking = false;

    blinkOpening = false;

    blinkStepAt = millis();

    blinkHoldUntil = 0;

    blinkHeight = 0;
}


//----------------------------------------------------
// Look
//----------------------------------------------------

void EyeEngine::look(
    float x,
    float y
)
{
    tracking = true;

    idleEnabled = false;

    setTarget(
        x,
        y
    );
}


//----------------------------------------------------
// Set Target
//----------------------------------------------------

void EyeEngine::setTarget(
    float x,
    float y
)
{
    //-----------------------------------------------
    // Limit Target Position
    //-----------------------------------------------

    targetX = constrain(
        x,
        -MAX_X,
        MAX_X
    );

    targetY = constrain(
        y,
        -MAX_Y,
        MAX_Y
    );
}


//----------------------------------------------------
// Center Eye
//----------------------------------------------------

void EyeEngine::center()
{
    tracking = false;

    idleEnabled = true;

    targetX = 0.0f;

    targetY = 0.0f;
}


//----------------------------------------------------
// Enable Tracking
//----------------------------------------------------

void EyeEngine::enableTracking(
    bool state
)
{
    tracking = state;

    idleEnabled = !state;
}


//----------------------------------------------------
// Tracking Status
//----------------------------------------------------

bool EyeEngine::trackingEnabled() const
{
    return tracking;
}


//----------------------------------------------------
// Enable Idle Movement
//----------------------------------------------------

void EyeEngine::enableIdle(
    bool state
)
{
    idleEnabled = state;

    if (!state)
    {
        targetX = pupilX;

        targetY = pupilY;
    }
}


//----------------------------------------------------
// Blink
//----------------------------------------------------

void EyeEngine::blink()
{
    if (!blinking)
    {
        blinking = true;
        blinkOpening = false;
        blinkHeight = 0;
        blinkStepAt = millis();
        blinkHoldUntil = 0;
    }
}


//----------------------------------------------------
// Smooth Eye Movement
//----------------------------------------------------

void EyeEngine::smoothMovement()
{
    //-----------------------------------------------
    // Tracking Response Speed
    //-----------------------------------------------

    const float speed = tracking
        ? 0.18f
        : 0.15f;


    //-----------------------------------------------
    // Smooth X Movement
    //-----------------------------------------------

    pupilX +=
        (targetX - pupilX)
        * speed;


    //-----------------------------------------------
    // Smooth Y Movement
    //-----------------------------------------------

    pupilY +=
        (targetY - pupilY)
        * speed;


    //-----------------------------------------------
    // Stop Tiny Floating-Point Movement
    //-----------------------------------------------

    if (abs(targetX - pupilX) < 0.02f)
    {
        pupilX = targetX;
    }

    if (abs(targetY - pupilY) < 0.02f)
    {
        pupilY = targetY;
    }
}


//----------------------------------------------------
// Random Idle Movement
//----------------------------------------------------

void EyeEngine::randomMovement()
{
    if (!idleEnabled)
    {
        return;
    }


    //-----------------------------------------------
    // Random Eye Movement Interval
    //-----------------------------------------------

    if (
        millis() - lastMove
        >
        2500
    )
    {
        targetX = random(
            -static_cast<int>(MAX_X),
             static_cast<int>(MAX_X) + 1
        );

        targetY = random(
            -static_cast<int>(MAX_Y),
             static_cast<int>(MAX_Y) + 1
        );

        lastMove = millis();
    }
}


//----------------------------------------------------
// Update
//----------------------------------------------------

void EyeEngine::update()
{
    //-----------------------------------------------
    // Idle Movement
    //-----------------------------------------------

    if (!tracking)
    {
        randomMovement();
    }


    //-----------------------------------------------
    // Smooth Movement
    //-----------------------------------------------

    smoothMovement();


    //-----------------------------------------------
    // Automatic Blink
    //-----------------------------------------------

    if (!blinking)
    {
        if (
            millis() - lastBlink
            >
            random(
                3000,
                6000
            )
        )
        {
            blinking = true;
        }
    }


    //-----------------------------------------------
    // Blink Animation
    //-----------------------------------------------

    if (blinking)
    {
        // Non-blocking blink: do not pause face tracking while the eyelid
        // closes and opens.
        const unsigned long now = millis();
        if (!blinkOpening && now - blinkStepAt >= 15)
        {
            blinkHeight = min(32, blinkHeight + 4);
            blinkStepAt = now;
            if (blinkHeight >= 32)
            {
                blinkOpening = true;
                blinkHoldUntil = now + 40;
            }
        }
        else if (blinkOpening && now >= blinkHoldUntil && now - blinkStepAt >= 15)
        {
            blinkHeight = max(0, blinkHeight - 4);
            blinkStepAt = now;
            if (blinkHeight == 0)
            {
                blinking = false;
                blinkOpening = false;
                lastBlink = now;
            }
        }
    }
}


//----------------------------------------------------
// Draw
//----------------------------------------------------

void EyeEngine::draw()
{
    graphics.drawEye(
        pupilX,
        pupilY,
        blinkHeight
    );
}


//----------------------------------------------------
// Current X
//----------------------------------------------------

float EyeEngine::getX() const
{
    return pupilX;
}


//----------------------------------------------------
// Current Y
//----------------------------------------------------

float EyeEngine::getY() const
{
    return pupilY;
}


//----------------------------------------------------
// Target X
//----------------------------------------------------

float EyeEngine::getTargetX() const
{
    return targetX;
}


//----------------------------------------------------
// Target Y
//----------------------------------------------------

float EyeEngine::getTargetY() const
{
    return targetY;
}
