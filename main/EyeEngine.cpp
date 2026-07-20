#include "EyeEngine.h"

EyeEngine::EyeEngine(SH1106 &display)
    : graphics(display)
{
    pupilX = 0;
    pupilY = 0;

    targetX = 0;
    targetY = 0;

    blinkHeight = 0;

    blinking = false;
    idleEnabled = true;

    lastBlink = 0;
    lastMove = 0;
}

void EyeEngine::begin()
{
    graphics.begin();

    randomSeed(millis());

    lastBlink = millis();
    lastMove = millis();
}

void EyeEngine::look(float x, float y)
{
    targetX = x;
    targetY = y;
}

void EyeEngine::center()
{
    targetX = 0;
    targetY = 0;
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

void EyeEngine::update()
{
    randomMovement();

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