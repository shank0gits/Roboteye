#ifndef CAMERA_ENGINE_H
#define CAMERA_ENGINE_H

#include <cstdint>
#include <mutex>
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
    // Update
    //---------------------------------------

    void update();


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

    //---------------------------------------
    // Camera Sensor
    //---------------------------------------

    sensor_t* sensor = nullptr;


    //---------------------------------------
    // Status
    //---------------------------------------

    bool initialized = false;


    //---------------------------------------
    // Camera Configuration
    //---------------------------------------

    camera_config_t config = {};

    uint32_t frameCounter = 0;

    // Vision processing and the HTTP capture endpoint share the single
    // camera frame buffer. Serialize both users to prevent FB-OVF.
    std::timed_mutex frame_mutex;


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
