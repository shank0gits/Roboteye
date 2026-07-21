#ifndef FACE_TRACKER_H
#define FACE_TRACKER_H

#include <cstdint>
#include "esp_camera.h"

class FaceTracker
{
public:

    //---------------------------------------
    // Constructor
    //---------------------------------------

    FaceTracker();

    //---------------------------------------
    // Control
    //---------------------------------------

    bool begin();

    bool update(camera_fb_t* frame);

    void reset();

    //---------------------------------------
    // Detection Status
    //---------------------------------------

    bool faceDetected() const;

    //---------------------------------------
    // Face Information
    //---------------------------------------

    int getFaceX() const;

    int getFaceY() const;

    int getFaceWidth() const;

    int getFaceHeight() const;

    int getFaceCenterX() const;

    int getFaceCenterY() const;

    //---------------------------------------
    // Eye Target
    //---------------------------------------

    float getEyeX() const;

    float getEyeY() const;

private:

    //---------------------------------------
    // Detection Status
    //---------------------------------------

    bool faceFound = false;

    //---------------------------------------
    // Face Bounding Box
    //---------------------------------------

    int faceX = 0;
    int faceY = 0;

    int faceWidth = 0;
    int faceHeight = 0;

    //---------------------------------------
    // Eye Target
    //
    // Normalized coordinates
    // X : -1.0 (Left)  -> +1.0 (Right)
    // Y : -1.0 (Up)    -> +1.0 (Down)
    //---------------------------------------

    float eyeX = 0.0f;
    float eyeY = 0.0f;

    //---------------------------------------
    // Internal Functions
    //---------------------------------------

    bool processFrame(camera_fb_t* frame);

    void mapFaceToEye(
        int faceCenterX,
        int faceCenterY,
        int frameWidth,
        int frameHeight
    );
};

#endif