#ifndef CAMERA_ENGINE_H
#define CAMERA_ENGINE_H

#include <Arduino.h>
#include "esp_camera.h"

class CameraEngine
{
public:

    //---------------------------------------
    // Constructor
    //---------------------------------------

    CameraEngine();

    //---------------------------------------
    // Camera Control
    //---------------------------------------

    bool begin();

    bool isReady() const;

    //---------------------------------------
    // Frame Capture
    //---------------------------------------

    camera_fb_t* captureFrame();

    void releaseFrame(camera_fb_t* frame);

    //---------------------------------------
    // Camera Information
    //---------------------------------------

    int getWidth() const;

    int getHeight() const;

    pixformat_t getPixelFormat() const;

    sensor_t* getSensor();

private:
    sensor_t* sensor;
    //---------------------------------------
    // Status
    //---------------------------------------

    bool initialized = false;

    //---------------------------------------
    // Camera Configuration
    //---------------------------------------

    camera_config_t config;

    //---------------------------------------
    // Cached Information
    //---------------------------------------

    int width = 0;
    int height = 0;
    pixformat_t pixelFormat = PIXFORMAT_JPEG;

    //---------------------------------------
    // Internal Functions
    //---------------------------------------

    bool initCamera();

    void setupConfig();
};

#endif