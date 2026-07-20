#include "FaceTracker.h"

FaceTracker::FaceTracker()
{
    detected = false;

    eyeX = 0.0f;
    eyeY = 0.0f;
}

//---------------------------------------
// Initialize
//---------------------------------------

bool FaceTracker::begin()
{
    detected = false;

    eyeX = 0.0f;
    eyeY = 0.0f;

    Serial.println("FaceTracker Ready");

    return true;
}
//---------------------------------------
// Update
//---------------------------------------

bool FaceTracker::update(camera_fb_t* frame)
{
    if(frame == nullptr)
    {
        detected = false;

        return false;
    }

    return processFrame(frame);
}
//---------------------------------------
// Process Frame
//---------------------------------------

bool FaceTracker::processFrame(camera_fb_t* frame)
{
    // Face detection will be added later.
    (void)frame;

    detected = false;

    eyeX = 0.0f;
    eyeY = 0.0f;

    return false;
}
//---------------------------------------
// Map Face Position
//---------------------------------------

void FaceTracker::mapFaceToEye(
    int faceCenterX,
    int faceCenterY,
    int frameWidth,
    int frameHeight)
{
    // Prevent divide by zero
    if(frameWidth <= 0 || frameHeight <= 0)
    {
        eyeX = 0.0f;
        eyeY = 0.0f;
        return;
    }

    // Normalize to -1.0 ... +1.0
    float normalizedX =
        ((float)faceCenterX / (float)frameWidth) * 2.0f - 1.0f;

    float normalizedY =
        ((float)faceCenterY / (float)frameHeight) * 2.0f - 1.0f;

    // Convert to EyeEngine movement range
    eyeX = normalizedX * 10.0f;
    eyeY = normalizedY * 6.0f;

    // Clamp values
    eyeX = constrain(eyeX, -10.0f, 10.0f);
    eyeY = constrain(eyeY, -6.0f, 6.0f);
}

--------------------------------------
// Status
//---------------------------------------

bool FaceTracker::faceDetected() const
{
    return detected;
}
//---------------------------------------
// Eye Coordinates
//---------------------------------------

float FaceTracker::getEyeX() const
{
    return eyeX;
}

float FaceTracker::getEyeY() const
{
    return eyeY;
}
//---------------------------------------
// Reset
//---------------------------------------

void FaceTracker::reset()
{
    detected = false;

    eyeX = 0.0f;
    eyeY = 0.0f;
}