#ifndef FACE_TRACKER_H
#define FACE_TRACKER_H

#include <cstdint>

#include "esp_camera.h"

#include "human_face_detect.hpp"

#include "dl_image_define.hpp"

class FaceTracker
{
public:

    //---------------------------------------
    // Constructor / Destructor
    //---------------------------------------

    FaceTracker();

    ~FaceTracker();

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
    // Face Detector
    //---------------------------------------

    HumanFaceDetect* detector = nullptr;

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