#include "VisionEngine.h"

#include "esp_log.h"

static const char* TAG = "VisionEngine";


//----------------------------------------------------
// Constructor
//----------------------------------------------------

VisionEngine::VisionEngine(
    CameraEngine& cameraRef,
    FaceTracker& trackerRef
)
    : camera(cameraRef),
      tracker(trackerRef)
{
}


//----------------------------------------------------
// Begin
//----------------------------------------------------

bool VisionEngine::begin()
{
    ESP_LOGI(TAG, "Starting Vision Engine...");

    //------------------------------------
    // Check Camera
    //------------------------------------

    if (!camera.isReady())
    {
        ESP_LOGE(
            TAG,
            "Camera is not ready"
        );

        initialized = false;

        return false;
    }

    //------------------------------------
    // Initialize Face Tracker
    //------------------------------------

    if (!tracker.begin())
    {
        ESP_LOGE(
            TAG,
            "FaceTracker initialization failed"
        );

        initialized = false;

        return false;
    }

    //------------------------------------
    // Reset Statistics
    //------------------------------------

    frameCounter = 0;

    processedFrames = 0;

    lostFrames = 0;

    //------------------------------------
    // Ready
    //------------------------------------

    initialized = true;

    ESP_LOGI(
        TAG,
        "Vision Engine Ready"
    );

    return true;
}


//----------------------------------------------------
// Update
//----------------------------------------------------

void VisionEngine::update()
{
    if (!initialized)
    {
        return;
    }

    //------------------------------------
    // Capture Camera Frame
    //------------------------------------

    camera_fb_t* frame =
        camera.captureFrame();

    if (frame == nullptr)
    {
        lostFrames++;

        return;
    }

    //------------------------------------
    // Count Frame
    //------------------------------------

    frameCounter++;

    //------------------------------------
    // Face Detection
    //------------------------------------

    bool detected =
        tracker.update(frame);

    if (detected)
    {
        processedFrames++;
    }
    else
    {
        lostFrames++;
    }

    //------------------------------------
    // Release Camera Frame
    //------------------------------------

    camera.releaseFrame(frame);

    //------------------------------------
    // Debug Statistics
    //------------------------------------

    if ((frameCounter % 60) == 0)
    {
        ESP_LOGI(
            TAG,
            "Frames: %lu | "
            "Processed: %lu | "
            "Lost: %lu | "
            "Face: %s",

            (unsigned long)frameCounter,

            (unsigned long)processedFrames,

            (unsigned long)lostFrames,

            tracker.faceDetected()
                ? "YES"
                : "NO"
        );
    }
}


//----------------------------------------------------
// Status
//----------------------------------------------------

bool VisionEngine::isReady() const
{
    return initialized;
}


//----------------------------------------------------
// Face Detection Status
//----------------------------------------------------

bool VisionEngine::faceDetected() const
{
    if (!initialized)
    {
        return false;
    }

    return tracker.faceDetected();
}


//----------------------------------------------------
// Statistics
//----------------------------------------------------

uint32_t VisionEngine::getFrameCount() const
{
    return frameCounter;
}


uint32_t VisionEngine::getProcessedFrames() const
{
    return processedFrames;
}


uint32_t VisionEngine::getLostFrames() const
{
    return lostFrames;
}


//----------------------------------------------------
// Components
//----------------------------------------------------

CameraEngine& VisionEngine::getCamera()
{
    return camera;
}


FaceTracker& VisionEngine::getTracker()
{
    return tracker;
}