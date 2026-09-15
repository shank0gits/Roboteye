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
    blinking = true;
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
        ? 0.22f
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
        //-------------------------------------------
        // Close Eye
        //-------------------------------------------

        blinkHeight += 4;

        if (blinkHeight >= 32)
        {
            blinkHeight = 32;
        }


        //-------------------------------------------
        // Fully Closed
        //-------------------------------------------

        if (blinkHeight == 32)
        {
            delay(40);


            //---------------------------------------
            // Open Eye
            //---------------------------------------

            while (blinkHeight > 0)
            {
                blinkHeight -= 4;


                graphics.drawEye(
                    pupilX,
                    pupilY,
                    blinkHeight
                );


                delay(15);
            }


            //---------------------------------------
            // Blink Complete
            //---------------------------------------

            blinkHeight = 0;

            blinking = false;

            lastBlink = millis();
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