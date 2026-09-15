#include "VisionEngine.h"

#include "esp_log.h"

static const char* TAG = "VisionEngine";


//----------------------------------------------------
// Constructor
//----------------------------------------------------

VisionEngine::VisionEngine(
    CameraEngine& cameraRef,
    FaceTracker& trackerRef,
    EyeEngine& eyeRef
)
    : camera(cameraRef),
      tracker(trackerRef),
      eye(eyeRef)
{
}


//----------------------------------------------------
// Begin
//----------------------------------------------------

bool VisionEngine::begin()
{
    ESP_LOGI(
        TAG,
        "Starting Vision Engine..."
    );


    //-----------------------------------------------
    // Check Camera
    //-----------------------------------------------

    if (!camera.isReady())
    {
        ESP_LOGE(
            TAG,
            "Camera is not ready"
        );

        initialized = false;

        return false;
    }


    //-----------------------------------------------
    // Initialize Face Tracker
    //-----------------------------------------------

    if (!tracker.begin())
    {
        ESP_LOGE(
            TAG,
            "FaceTracker initialization failed"
        );

        initialized = false;

        return false;
    }


    //-----------------------------------------------
    // Reset Statistics
    //-----------------------------------------------

    frameCounter = 0;

    processedFrames = 0;

    lostFrames = 0;


    //-----------------------------------------------
    // Disable Eye Tracking Initially
    //-----------------------------------------------

    eye.enableTracking(false);


    //-----------------------------------------------
    // Ready
    //-----------------------------------------------

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
    //-----------------------------------------------
    // Check Initialization
    //-----------------------------------------------

    if (!initialized)
    {
        return;
    }


    //-----------------------------------------------
    // Capture Camera Frame
    //-----------------------------------------------

    camera_fb_t* frame =
        camera.captureFrame();


    //-----------------------------------------------
    // Check Frame
    //-----------------------------------------------

    if (frame == nullptr)
    {
        lostFrames++;

        //-------------------------------------------
        // No Frame = No Tracking
        //-------------------------------------------

        eye.enableTracking(false);

        return;
    }


    //-----------------------------------------------
    // Count Captured Frame
    //-----------------------------------------------

    frameCounter++;


    //-----------------------------------------------
    // Run Face Detection
    //-----------------------------------------------

    const bool detected =
        tracker.update(frame);


    //-----------------------------------------------
    // IMPORTANT:
    // Release Camera Frame Immediately
    //-----------------------------------------------
    //
    // FaceTracker has already decoded and processed
    // the frame by this point.
    //
    // Releasing immediately prevents the camera
    // frame buffer from remaining occupied.
    //

    camera.releaseFrame(frame);

    frame = nullptr;


    //-----------------------------------------------
    // Face Detected
    //-----------------------------------------------

    if (detected)
    {
        //-------------------------------------------
        // Count Successful Detection
        //-------------------------------------------

        processedFrames++;


        //-------------------------------------------
        // Enable Eye Tracking
        //-------------------------------------------

        eye.enableTracking(true);


        //-------------------------------------------
        // Update Eye Target
        //-------------------------------------------

        eye.setTarget(
            tracker.getEyeX(),
            tracker.getEyeY()
        );
    }


    //-----------------------------------------------
    // Face Not Detected
    //-----------------------------------------------

    else
    {
        //-------------------------------------------
        // Count Lost Frame
        //-------------------------------------------

        lostFrames++;


        //-------------------------------------------
        // Disable Eye Tracking
        //-------------------------------------------

        eye.enableTracking(false);
    }


    //-----------------------------------------------
    // Periodic Statistics
    //-----------------------------------------------

    if (
        (frameCounter % 60) == 0
    )
    {
        ESP_LOGI(
            TAG,
            "Frames: %lu | "
            "Processed: %lu | "
            "Lost: %lu | "
            "Face: %s",

            (unsigned long)
            frameCounter,

            (unsigned long)
            processedFrames,

            (unsigned long)
            lostFrames,

            detected
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