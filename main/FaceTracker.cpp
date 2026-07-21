#include "FaceTracker.h"

#include "esp_log.h"
#include "esp_camera.h"

#include <algorithm>

static const char* TAG = "FaceTracker";

//----------------------------------------------------
// Constructor
//----------------------------------------------------

FaceTracker::FaceTracker()
{
    reset();
}


//----------------------------------------------------
// Initialize
//----------------------------------------------------

bool FaceTracker::begin()
{
    ESP_LOGI(TAG, "Initializing Face Tracker...");

    reset();

    ESP_LOGI(TAG, "Face Tracker Ready");

    return true;
}


//----------------------------------------------------
// Update
//----------------------------------------------------

bool FaceTracker::update(camera_fb_t* frame)
{
    if (frame == nullptr)
    {
        reset();

        return false;
    }

    return processFrame(frame);
}


//----------------------------------------------------
// Process Camera Frame
//----------------------------------------------------

bool FaceTracker::processFrame(camera_fb_t* frame)
{
    if (frame == nullptr)
    {
        reset();

        return false;
    }

    //------------------------------------------------
    // Current frame information
    //------------------------------------------------

    int frameWidth  = frame->width;
    int frameHeight = frame->height;


    //------------------------------------------------
    // TODO:
    //
    // Real ESP32-S3 face detection will be added here.
    //
    // Expected result:
    //
    // faceX
    // faceY
    // faceWidth
    // faceHeight
    //
    // Then:
    //
    // faceFound = true;
    //
    // mapFaceToEye(
    //     getFaceCenterX(),
    //     getFaceCenterY(),
    //     frameWidth,
    //     frameHeight
    // );
    //------------------------------------------------


    //------------------------------------------------
    // Temporary:
    // No face detection yet
    //------------------------------------------------

    (void)frameWidth;
    (void)frameHeight;

    reset();

    return false;
}


//----------------------------------------------------
// Convert Face Position to Eye Position
//----------------------------------------------------

void FaceTracker::mapFaceToEye(
    int faceCenterX,
    int faceCenterY,
    int frameWidth,
    int frameHeight)
{
    if (frameWidth <= 0 || frameHeight <= 0)
    {
        eyeX = 0.0f;
        eyeY = 0.0f;

        return;
    }


    //------------------------------------------------
    // Normalize X
    //
    // 0              = Left
    // frameWidth/2   = Center
    // frameWidth     = Right
    //------------------------------------------------

    float nx =
        ((float)faceCenterX / (float)frameWidth)
        * 2.0f
        - 1.0f;


    //------------------------------------------------
    // Normalize Y
    //
    // 0              = Up
    // frameHeight/2  = Center
    // frameHeight     = Down
    //------------------------------------------------

    float ny =
        ((float)faceCenterY / (float)frameHeight)
        * 2.0f
        - 1.0f;


    //------------------------------------------------
    // Convert to EyeEngine coordinate range
    //------------------------------------------------

    eyeX = std::clamp(
        nx * 10.0f,
        -10.0f,
        10.0f
    );

    eyeY = std::clamp(
        ny * 6.0f,
        -6.0f,
        6.0f
    );


    //------------------------------------------------
    // Debug
    //------------------------------------------------

    ESP_LOGD(
        TAG,
        "Face Center: (%d,%d)  Eye Target: (%.2f, %.2f)",
        faceCenterX,
        faceCenterY,
        eyeX,
        eyeY
    );
}


//----------------------------------------------------
// Reset
//----------------------------------------------------

void FaceTracker::reset()
{
    faceFound = false;

    faceX = 0;
    faceY = 0;

    faceWidth = 0;
    faceHeight = 0;

    eyeX = 0.0f;
    eyeY = 0.0f;
}


//----------------------------------------------------
// Face Detection Status
//----------------------------------------------------

bool FaceTracker::faceDetected() const
{
    return faceFound;
}


//----------------------------------------------------
// Face X
//----------------------------------------------------

int FaceTracker::getFaceX() const
{
    return faceX;
}


//----------------------------------------------------
// Face Y
//----------------------------------------------------

int FaceTracker::getFaceY() const
{
    return faceY;
}


//----------------------------------------------------
// Face Width
//----------------------------------------------------

int FaceTracker::getFaceWidth() const
{
    return faceWidth;
}


//----------------------------------------------------
// Face Height
//----------------------------------------------------

int FaceTracker::getFaceHeight() const
{
    return faceHeight;
}


//----------------------------------------------------
// Face Center X
//----------------------------------------------------

int FaceTracker::getFaceCenterX() const
{
    return faceX + (faceWidth / 2);
}


//----------------------------------------------------
// Face Center Y
//----------------------------------------------------

int FaceTracker::getFaceCenterY() const
{
    return faceY + (faceHeight / 2);
}


//----------------------------------------------------
// Eye Target X
//----------------------------------------------------

float FaceTracker::getEyeX() const
{
    return eyeX;
}


//----------------------------------------------------
// Eye Target Y
//----------------------------------------------------

float FaceTracker::getEyeY() const
{
    return eyeY;
}