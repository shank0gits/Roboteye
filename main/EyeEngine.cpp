#include "EyeEngine.h"
// constructor
EyeEngine::EyeEngine(SH1106 &display)
    : graphics(display)
{
}

void EyeEngine::begin()
{
    graphics.begin();

    randomSeed(millis());

    lastBlink = millis();
    lastMove = millis();
}
// look()
void EyeEngine::look(float x, float y)
{
    tracking = true;
    idleEnabled = false;

    targetX = constrain(x, -10.0f, 10.0f);
    targetY = constrain(y, -6.0f, 6.0f);
}
void EyeEngine::setTarget(float x, float y)
{
    targetX = constrain(x, -10.0f, 10.0f);
    targetY = constrain(y, -6.0f, 6.0f);
}
//center
void EyeEngine::center()
{
    tracking = false;
    idleEnabled = true;

    targetX = 0.0f;
    targetY = 0.0f;
}
void EyeEngine::enableTracking(bool state)
{
    tracking = state;
    idleEnabled = !state;
}

bool EyeEngine::trackingEnabled() const
{
    return tracking;
}

void EyeEngine::enableIdle(bool state)
{
    idleEnabled = state;
}

void EyeEngine::blink()
{
    blinking = true;
}

void EyeEngine::smoothMovement()
{
    const float speed = 0.15f;

    pupilX += (targetX - pupilX) * speed;
    pupilY += (targetY - pupilY) * speed;
}

void EyeEngine::randomMovement()
{
    if (!idleEnabled)
        return;

    if (millis() - lastMove > 2500)
    {
        targetX = random(-10, 11);
        targetY = random(-6, 7);

        lastMove = millis();
    }
}
//update
void EyeEngine::update()
{
    if(!tracking)
    {
        randomMovement();
    }

    smoothMovement();

    if (!blinking)
    {
        if (millis() - lastBlink > random(3000, 6000))
        {
            blinking = true;
        }
    }

    if (blinking)
    {
        blinkHeight += 4;

        if (blinkHeight >= 32)
            blinkHeight = 32;

        if (blinkHeight == 32)
        {
            delay(40);

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

            blinking = false;

            lastBlink = millis();
        }
    }
}

void EyeEngine::draw()
{
    graphics.drawEye(
        pupilX,
        pupilY,
        blinkHeight
    );
}
float EyeEngine::getX() const
{
    return pupilX;
}

float EyeEngine::getY() const
{
    return pupilY;
}

float EyeEngine::getTargetX() const
{
    return targetX;
}

float EyeEngine::getTargetY() const
{
    return targetY;
}