#include "Graphics.h"

#include <math.h>


//====================================================
// Constructor
//====================================================

Graphics::Graphics(
    SH1106 &display
)
    : oled(&display)
{
}


//====================================================
// Display Initialization
//====================================================

void Graphics::begin()
{
    clear();
    show();
}


//====================================================
// Clear Display
//====================================================

void Graphics::clear()
{
    oled->clear();
}


//====================================================
// Update Display
//====================================================

void Graphics::show()
{
    oled->display();
}


//====================================================
// Main Eye Drawing
//====================================================

void Graphics::drawEye(
    float pupilX,
    float pupilY,
    int blinkHeight
)
{
    //-----------------------------------------------
    // BLACK BACKGROUND
    //-----------------------------------------------

    clear();


    //-----------------------------------------------
    // Outer Eye Housing
    //-----------------------------------------------

    drawEyeball();


    //-----------------------------------------------
    // Maximum Eye Movement
    //-----------------------------------------------

    constexpr float maxDistance =
        static_cast<float>(
            IRIS_RADIUS
            -
            PUPIL_RADIUS
            -
            2
        );

    constexpr float maxDistanceSquared =
        maxDistance * maxDistance;


    //-----------------------------------------------
    // Limit Movement
    //-----------------------------------------------

    const float distanceSquared =
        pupilX * pupilX +
        pupilY * pupilY;


    if (
        distanceSquared >
        maxDistanceSquared
    )
    {
        const float scale =
            maxDistance /
            sqrtf(distanceSquared);

        pupilX *= scale;
        pupilY *= scale;
    }


    //-----------------------------------------------
    // Eye Position
    //-----------------------------------------------

    const int irisX =
        CENTER_X +
        static_cast<int>(
            roundf(pupilX)
        );

    const int irisY =
        CENTER_Y +
        static_cast<int>(
            roundf(pupilY)
        );


    //-----------------------------------------------
    // Draw Iris
    //-----------------------------------------------

    drawIris(
        irisX,
        irisY
    );


    //-----------------------------------------------
    // Draw Pupil
    //-----------------------------------------------

    drawPupil(
        irisX,
        irisY
    );


    //-----------------------------------------------
    // Highlights
    //-----------------------------------------------

    drawHighlights(
        irisX,
        irisY
    );


    //-----------------------------------------------
    // Blink
    //-----------------------------------------------

    drawBlink(
        blinkHeight
    );


    //-----------------------------------------------
    // Update OLED
    //-----------------------------------------------

    show();
}


//====================================================
// Main Eye Housing
//====================================================

void Graphics::drawEyeball()
{
    //-----------------------------------------------
    // BLACK SCREEN
    //-----------------------------------------------

    oled->clear();


    //-----------------------------------------------
    // THICK OUTER CIRCULAR RING
    //-----------------------------------------------
    //
    // 5 pixel thick
    //
    //-----------------------------------------------

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS,
        true
    );

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS - 1,
        true
    );

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS - 2,
        true
    );

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS - 3,
        true
    );

    oled->drawCircle(
        CENTER_X,
        CENTER_Y,
        EYE_RADIUS - 4,
        true
    );


    //-----------------------------------------------
    // Cut Inner Area Back To BLACK
    //-----------------------------------------------

    const int innerRadius =
        EYE_RADIUS - 6;


    oled->fillCircle(
        CENTER_X,
        CENTER_Y,
        innerRadius,
        false
    );
}


//====================================================
// Iris
//====================================================

void Graphics::drawIris(
    int x,
    int y
)
{
    //-----------------------------------------------
    // THICK OUTER IRIS RING
    //-----------------------------------------------

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS,
        true
    );

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS - 1,
        true
    );

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS - 2,
        true
    );


    //-----------------------------------------------
    // BLACK IRIS INTERIOR
    //-----------------------------------------------

    oled->fillCircle(
        x,
        y,
        IRIS_RADIUS - 3,
        false
    );


    //-----------------------------------------------
    // INNER ROBOTIC RING
    //-----------------------------------------------

    const int ringRadius =
        PUPIL_RADIUS + 6;


    oled->drawCircle(
        x,
        y,
        ringRadius,
        true
    );

    oled->drawCircle(
        x,
        y,
        ringRadius - 1,
        true
    );


    //-----------------------------------------------
    // SECOND INNER RING
    //-----------------------------------------------

    const int secondRing =
        PUPIL_RADIUS + 10;


    oled->drawCircle(
        x,
        y,
        secondRing,
        true
    );


    //-----------------------------------------------
    // ROBOTIC RADIAL DETAILS
    //-----------------------------------------------

    drawSpokes(
        x,
        y
    );


    //-----------------------------------------------
    // Minimal Texture
    //-----------------------------------------------

    drawIrisTexture(
        x,
        y
    );
}


//====================================================
// Pupil
//====================================================

void Graphics::drawPupil(
    int x,
    int y
)
{
    //-----------------------------------------------
    // THICK PUPIL OUTLINE
    //-----------------------------------------------

    oled->drawCircle(
        x,
        y,
        PUPIL_RADIUS,
        true
    );

    oled->drawCircle(
        x,
        y,
        PUPIL_RADIUS - 1,
        true
    );

    oled->drawCircle(
        x,
        y,
        PUPIL_RADIUS - 2,
        true
    );


    //-----------------------------------------------
    // BLACK PUPIL CENTER
    //-----------------------------------------------

    oled->fillCircle(
        x,
        y,
        PUPIL_RADIUS - 3,
        false
    );


    //-----------------------------------------------
    // Small Central Core
    //-----------------------------------------------

    oled->drawCircle(
        x,
        y,
        4,
        true
    );
}


//====================================================
// Highlights
//====================================================

void Graphics::drawHighlights(
    int x,
    int y
)
{
    //-----------------------------------------------
    // Small Digital Reflection
    //-----------------------------------------------

    oled->fillCircle(
        x - 5,
        y - 5,
        2,
        true
    );


    //-----------------------------------------------
    // Tiny Reflection
    //-----------------------------------------------

    oled->fillCircle(
        x + 5,
        y + 4,
        1,
        true
    );
}


//====================================================
// Blink
//====================================================

void Graphics::drawBlink(
    int amount
)
{
    //-----------------------------------------------
    // Normal Eye
    //-----------------------------------------------

    if (amount <= 0)
    {
        return;
    }


    //-----------------------------------------------
    // Limit Blink
    //-----------------------------------------------

    const int maxBlink =
        HEIGHT / 2;


    if (amount > maxBlink)
    {
        amount = maxBlink;
    }


    //-----------------------------------------------
    // Upper Black Mask
    //-----------------------------------------------

    oled->fillRect(
        0,
        0,
        WIDTH,
        amount,
        false
    );


    //-----------------------------------------------
    // Lower Black Mask
    //-----------------------------------------------

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

void Graphics::drawOuterRing(
    int x,
    int y
)
{
    //-----------------------------------------------
    // VERY THICK RING
    //-----------------------------------------------

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS,
        true
    );

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS - 1,
        true
    );

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS - 2,
        true
    );

    oled->drawCircle(
        x,
        y,
        IRIS_RADIUS - 3,
        true
    );
}


//====================================================
// Inner Iris Rings
//====================================================

void Graphics::drawInnerRing(
    int x,
    int y
)
{
    const int innerRadius =
        PUPIL_RADIUS + 4;


    oled->drawCircle(
        x,
        y,
        innerRadius,
        true
    );

    oled->drawCircle(
        x,
        y,
        innerRadius - 1,
        true
    );


    const int outerInnerRadius =
        PUPIL_RADIUS + 9;


    oled->drawCircle(
        x,
        y,
        outerInnerRadius,
        true
    );
}


//====================================================
// Iris Spokes
//====================================================

void Graphics::drawSpokes(
    int x,
    int y
)
{
    constexpr float RAD_PER_DEG =
        0.017453292519943295f;


    const int innerRadius =
        PUPIL_RADIUS + 5;


    const int outerRadius =
        IRIS_RADIUS - 4;


    //-----------------------------------------------
    // 8 LARGE ROBOTIC SPOKES
    //-----------------------------------------------

    for (
        int angle = 0;
        angle < 360;
        angle += 45
    )
    {
        const float rad =
            angle * RAD_PER_DEG;


        const float c =
            cosf(rad);


        const float s =
            sinf(rad);


        const int x1 =
            x +
            static_cast<int>(
                c * innerRadius
            );


        const int y1 =
            y +
            static_cast<int>(
                s * innerRadius
            );


        const int x2 =
            x +
            static_cast<int>(
                c * outerRadius
            );


        const int y2 =
            y +
            static_cast<int>(
                s * outerRadius
            );


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

void Graphics::drawIrisTexture(
    int x,
    int y
)
{
    //-----------------------------------------------
    // Mechanical segmented rings
    //-----------------------------------------------

    const int r1 =
        PUPIL_RADIUS + 12;


    if (r1 < IRIS_RADIUS)
    {
        oled->drawCircle(
            x,
            y,
            r1,
            true
        );
    }


    //-----------------------------------------------
    // Four small mechanical marks
    //-----------------------------------------------

    const int markRadius =
        IRIS_RADIUS - 5;


    oled->drawLine(
        x - 3,
        y - markRadius,
        x + 3,
        y - markRadius,
        true
    );


    oled->drawLine(
        x - 3,
        y + markRadius,
        x + 3,
        y + markRadius,
        true
    );


    oled->drawLine(
        x - markRadius,
        y - 3,
        x - markRadius,
        y + 3,
        true
    );


    oled->drawLine(
        x + markRadius,
        y - 3,
        x + markRadius,
        y + 3,
        true
    );
}


//====================================================
// Circle Ring
//====================================================

void Graphics::drawCircleRing(
    int x,
    int y,
    int radius
)
{
    if (radius <= 0)
    {
        return;
    }


    oled->drawCircle(
        x,
        y,
        radius,
        true
    );
}


//====================================================
// Iris Glow
//====================================================

void Graphics::drawGlow(
    int x,
    int y,
    int radius
)
{
    //-----------------------------------------------
    // THICK GLOW RING
    //-----------------------------------------------

    oled->drawCircle(
        x,
        y,
        radius,
        true
    );

    oled->drawCircle(
        x,
        y,
        radius - 1,
        true
    );

    oled->drawCircle(
        x,
        y,
        radius - 2,
        true
    );
}