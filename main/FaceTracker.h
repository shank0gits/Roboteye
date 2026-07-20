#ifndef FACE_TRACKER_H
#define FACE_TRACKER_H

#include <Arduino.h>
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

    //---------------------------------------
    // Status
    //---------------------------------------

    bool faceDetected() const;

    //---------------------------------------
    // Eye Coordinates
    //---------------------------------------

    float getEyeX() const;

    float getEyeY() const;

    //---------------------------------------
    // Reset
    //---------------------------------------

    void reset();

private:

    //---------------------------------------
    // Detection Status
    //---------------------------------------

    bool detected;

    //---------------------------------------
    // Eye Target
    //---------------------------------------

    float eyeX;
    float eyeY;

    //---------------------------------------
    // Internal
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