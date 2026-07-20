#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <Arduino.h>
#include "SH1106.h"

class Graphics
{
public:

    Graphics(SH1106 &display);

    void begin();

    void clear();

    void show();

    //------------------------------------
    // Main Draw Function
    //------------------------------------

    void drawEye(
        float pupilX,
        float pupilY,
        int blinkHeight
    );

private:

    //------------------------------------
    // OLED Driver
    //------------------------------------

    SH1106 *oled;

    //------------------------------------
    // Screen
    //------------------------------------

    static constexpr uint16_t WIDTH  = 128;
    static constexpr uint16_t HEIGHT = 64;

    //------------------------------------
    // Eye
    //------------------------------------

    static constexpr int CENTER_X = 64;
    static constexpr int CENTER_Y = 32;

    static constexpr int EYE_RADIUS = 70;

    static constexpr int IRIS_RADIUS = 40;

    static constexpr int PUPIL_RADIUS = 30;

    //------------------------------------
    // Drawing
    //------------------------------------

    void drawEyeball();

    void drawIris(
        int x,
        int y
    );

    void drawPupil(
        int x,
        int y
    );

    void drawHighlights(
        int x,
        int y
    );

    void drawBlink(
        int amount
    );

    //------------------------------------
    // Iris Details
    //------------------------------------

    void drawIrisTexture(
        int x,
        int y
    );

    void drawOuterRing(
        int x,
        int y
    );

    void drawInnerRing(
        int x,
        int y
    );

    void drawSpokes(
        int x,
        int y
    );

    //------------------------------------
    // Helper Drawing
    //------------------------------------

    void drawCircleRing(
        int x,
        int y,
        int radius
    );

    void drawGlow(
        int x,
        int y,
        int radius
    );
};

#endif