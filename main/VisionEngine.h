#ifndef VISION_ENGINE_H
#define VISION_ENGINE_H

#include <cstdint>

#include "CameraEngine.h"
#include "FaceTracker.h"
#include "EyeEngine.h"

class VisionEngine
{
public:

    //---------------------------------------
    // Constructor
    //---------------------------------------

    VisionEngine(
        CameraEngine& camera,
        FaceTracker& tracker,
        EyeEngine& eye
    );

    //---------------------------------------
    // Initialization
    //---------------------------------------

    bool begin();

    //---------------------------------------
    // Update
    //---------------------------------------

    void update();

    //---------------------------------------
    // Status
    //---------------------------------------

    bool isReady() const;

    bool faceDetected() const;

    //---------------------------------------
    // Statistics
    //---------------------------------------

    uint32_t getFrameCount() const;

    uint32_t getProcessedFrames() const;

    uint32_t getLostFrames() const;

    //---------------------------------------
    // Components
    //---------------------------------------

    CameraEngine& getCamera();

    FaceTracker& getTracker();

private:

    //---------------------------------------
    // References
    //---------------------------------------

    CameraEngine& camera;

    FaceTracker& tracker;

    EyeEngine& eye;

    //---------------------------------------
    // Status
    //---------------------------------------

    bool initialized = false;

    //---------------------------------------
    // Statistics
    //---------------------------------------

    uint32_t frameCounter = 0;

    uint32_t processedFrames = 0;

    uint32_t lostFrames = 0;
};

#endif