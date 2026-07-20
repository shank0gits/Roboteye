#include "Graphics.h"
#include <math.h>

Graphics::Graphics(SH1106 &display)
{
    oled = &display;
}

void Graphics::begin()
{
    clear();
    show();
}

void Graphics::clear()
{
    oled->clear();
}

void Graphics::show()
{
    oled->display();
}

//====================================================
// Main Draw
//====================================================

void Graphics::drawEye(float pupilX, float pupilY, int blinkHeight)
{
    clear();

    drawEyeball();

    //------------------------------------
    // Limit iris movement
    //------------------------------------

    float distance = sqrtf(pupilX * pupilX + pupilY * pupilY);

    const float maxDistance =
        (float)(IRIS_RADIUS - PUPIL_RADIUS - 2);

    if(distance > maxDistance)
    {
        pupilX *= maxDistance / distance;
        pupilY *= maxDistance / distance;
    }

    int irisX = CENTER_X + (int)roundf(pupilX);
    int irisY = CENTER_Y + (int)roundf(pupilY);

    drawIris(
        irisX,
        irisY
    );

    drawPupil(
        irisX,
        irisY
    );

    drawHighlights(
        irisX,
        irisY
    );

    drawBlink(
        blinkHeight
    );

    show();
}

//====================================================
// Eyeball
//====================================================

void Graphics::drawEyeball()
{
    oled->fillCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS,
        true
    );

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS,
        true
    );
}

//====================================================
// Iris
//====================================================

void Graphics::drawIris(int x, int y)
{
    oled->fillCircle(
        x,
        y,
        IRIS_RADIUS,
        false
    );

    drawOuterRing(
        x,
        y
    );

    drawInnerRing(
        x,
        y
    );

    drawGlow(
        x,
        y,
        IRIS_RADIUS
    );

    drawSpokes(
        x,
        y
    );

    drawIrisTexture(
        x,
        y
    );
}
//====================================================
// Pupil
//====================================================

void Graphics::drawPupil(int x, int y)
{
    oled->fillCircle(
        x,
        y,
        PUPIL_RADIUS,
        false
    );

    oled->drawCircle(
        x,
        y,
        PUPIL_RADIUS,
        true
    );
}

//====================================================
// Highlights
//====================================================

void Graphics::drawHighlights(int x, int y)
{
    // Main reflection
    oled->fillCircle(
        x - 5,
        y - 5,
        3,
        true
    );

    // Secondary reflection
    oled->fillCircle(
        x + 5,
        y + 5,
        1,
        true
    );
}

//====================================================
// Blink
//====================================================

void Graphics::drawBlink(int amount)
{
    if(amount <= 0)
        return;

    if(amount > HEIGHT / 2)
        amount = HEIGHT / 2;

    // Upper eyelid
    oled->fillRect(
        0,
        0,
        WIDTH,
        amount,
        false
    );

    // Lower eyelid
    oled->fillRect(
        0,
        HEIGHT - amount,
        WIDTH,
        amount,
        false
    );
}

//====================================================
// Outer Iris Ring
//====================================================

void Graphics::drawOuterRing(int x, int y)
{
    drawCircleRing(
        x,
        y,
        IRIS_RADIUS
    );

    drawCircleRing(
        x,
        y,
        IRIS_RADIUS - 1
    );
}

//====================================================
// Inner Iris Rings
//====================================================

void Graphics::drawInnerRing(int x, int y)
{
    drawCircleRing(
        x,
        y,
        PUPIL_RADIUS + 2
    );

    drawCircleRing(
        x,
        y,
        PUPIL_RADIUS + 4
    );

    drawCircleRing(
        x,
        y,
        PUPIL_RADIUS + 6
    );
}
//====================================================
// Iris Spokes
//====================================================

void Graphics::drawSpokes(int x, int y)
{
    for(int angle = 0; angle < 360; angle += 30)
    {
        float rad = angle * (3.14159265f / 180.0f);

        int x1 = x + (int)(cosf(rad) * (PUPIL_RADIUS + 2));
        int y1 = y + (int)(sinf(rad) * (PUPIL_RADIUS + 2));

        int x2 = x + (int)(cosf(rad) * (IRIS_RADIUS - 2));
        int y2 = y + (int)(sinf(rad) * (IRIS_RADIUS - 2));

        oled->drawLine(
            x1,
            y1,
            x2,
            y2,
            true
        );
    }
}

//====================================================
// Iris Texture
//====================================================

void Graphics::drawIrisTexture(int x, int y)
{
    for(int r = PUPIL_RADIUS + 3;
        r < IRIS_RADIUS;
        r += 2)
    {
        drawCircleRing(
            x,
            y,
            r
        );
    }
}

//====================================================
// Circle Ring
//====================================================

void Graphics::drawCircleRing(
    int x,
    int y,
    int radius)
{
    oled->drawCircle(
        x,
        y,
        radius,
        true
    );
}

//====================================================
// Glow
//====================================================

void Graphics::drawGlow(
    int x,
    int y,
    int radius)
{
    oled->drawCircle(
        x,
        y,
        radius,
        true
    );

    oled->drawCircle(
        x,
        y,
        radius + 1,
        true
    );

    oled->drawCircle(
        x,
        y,
        radius + 2,
        true
    );
}