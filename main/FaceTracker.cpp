#include "FaceTracker.h"

#include "esp_log.h"
#include "esp_heap_caps.h"

#include "dl_image_jpeg.hpp"

#include <algorithm>

static const char* TAG = "FaceTracker";


//----------------------------------------------------
// Eye Tracking Output Range
//----------------------------------------------------

static constexpr float EYE_TRACK_MAX_X = 20.0f;
static constexpr float EYE_TRACK_MAX_Y = 12.0f;


//----------------------------------------------------
// Constructor
//----------------------------------------------------

FaceTracker::FaceTracker()
{
    reset();
}


//----------------------------------------------------
// Destructor
//----------------------------------------------------

FaceTracker::~FaceTracker()
{
    if (detector != nullptr)
    {
        delete detector;
        detector = nullptr;
    }
}


//----------------------------------------------------
// Initialize
//----------------------------------------------------

bool FaceTracker::begin()
{
    ESP_LOGI(
        TAG,
        "Initializing Face Tracker..."
    );

    reset();


    //-----------------------------------------------
    // Prevent Duplicate Detector
    //-----------------------------------------------

    if (detector != nullptr)
    {
        ESP_LOGI(
            TAG,
            "Face Detector already initialized"
        );

        return true;
    }


    //-----------------------------------------------
    // Create Human Face Detector
    //-----------------------------------------------

    detector =
        new HumanFaceDetect(
            HumanFaceDetect::ESPDET_PICO_224_224_FACE
        );


    //-----------------------------------------------
    // Check Detector
    //-----------------------------------------------

    if (detector == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Failed to create HumanFaceDetect"
        );

        return false;
    }


    //-----------------------------------------------
    // Ready
    //-----------------------------------------------

    ESP_LOGI(
        TAG,
        "HumanFaceDetect initialized"
    );

    ESP_LOGI(
        TAG,
        "Face Tracker Ready"
    );

    return true;
}


//----------------------------------------------------
// Update
//----------------------------------------------------

bool FaceTracker::update(
    camera_fb_t* frame
)
{
    if (frame == nullptr)
    {
        reset();
        return false;
    }


    if (detector == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Face detector is not initialized"
        );

        reset();
        return false;
    }


    return processFrame(frame);
}


//----------------------------------------------------
// Process Camera Frame
//----------------------------------------------------

bool FaceTracker::processFrame(
    camera_fb_t* frame
)
{
    //-----------------------------------------------
    // Validate Frame
    //-----------------------------------------------

    if (frame == nullptr)
    {
        reset();
        return false;
    }


    //-----------------------------------------------
    // Validate JPEG
    //-----------------------------------------------

    if (frame->format != PIXFORMAT_JPEG)
    {
        ESP_LOGW(
            TAG,
            "Unsupported frame format: %d",
            frame->format
        );

        reset();

        return false;
    }


    //-----------------------------------------------
    // Create JPEG Image
    //-----------------------------------------------

    dl::image::jpeg_img_t jpegImage;

    jpegImage.data =
        frame->buf;

    jpegImage.data_len =
        frame->len;


    //-----------------------------------------------
    // Decode JPEG
    //-----------------------------------------------

    dl::image::img_t image =
        dl::image::sw_decode_jpeg(
            jpegImage,
            dl::image::DL_IMAGE_PIX_TYPE_RGB888
        );


    //-----------------------------------------------
    // Decode Failed
    //-----------------------------------------------

    if (image.data == nullptr)
    {
        ESP_LOGW(
            TAG,
            "JPEG decode failed"
        );

        reset();

        return false;
    }


    //-----------------------------------------------
    // Save Dimensions
    //-----------------------------------------------

    const int decodedWidth =
        image.width;

    const int decodedHeight =
        image.height;


    //-----------------------------------------------
    // Run Face Detection
    //-----------------------------------------------

    std::list<dl::detect::result_t>& results =
        detector->run(image);


    //-----------------------------------------------
    // No Face
    //-----------------------------------------------

    if (results.empty())
    {
        heap_caps_free(
            image.data
        );

        image.data = nullptr;

        reset();

        return false;
    }


    //-----------------------------------------------
    // Get First Face
    //-----------------------------------------------

    const auto& face =
        results.front();


    //-----------------------------------------------
    // Save Bounding Box
    //-----------------------------------------------

    faceX =
        static_cast<int>(
            face.box[0]
        );

    faceY =
        static_cast<int>(
            face.box[1]
        );

    faceWidth =
        static_cast<int>(
            face.box[2]
            -
            face.box[0]
        );

    faceHeight =
        static_cast<int>(
            face.box[3]
            -
            face.box[1]
        );


    //-----------------------------------------------
    // Free Decoded Image
    //-----------------------------------------------

    heap_caps_free(
        image.data
    );

    image.data = nullptr;


    //-----------------------------------------------
    // Validate Face
    //-----------------------------------------------

    if (
        faceWidth <= 0 ||
        faceHeight <= 0
    )
    {
        reset();

        return false;
    }


    //-----------------------------------------------
    // Face Found
    //-----------------------------------------------

    faceFound = true;


    //-----------------------------------------------
    // Face Center
    //-----------------------------------------------

    const int centerX =
        getFaceCenterX();

    const int centerY =
        getFaceCenterY();


    //-----------------------------------------------
    // Convert Face Position
    // To Eye Position
    //-----------------------------------------------

    mapFaceToEye(
        centerX,
        centerY,
        decodedWidth,
        decodedHeight
    );


    //-----------------------------------------------
    // Debug
    //-----------------------------------------------

    ESP_LOGI(
        TAG,
        "FACE FOUND "
        "Box=(%d,%d,%d,%d) "
        "Center=(%d,%d) "
        "Eye=(%.2f,%.2f)",
        faceX,
        faceY,
        faceWidth,
        faceHeight,
        centerX,
        centerY,
        eyeX,
        eyeY
    );


    return true;
}


//----------------------------------------------------
// Map Face Position To Eye Position
//----------------------------------------------------

void FaceTracker::mapFaceToEye(
    int faceCenterX,
    int faceCenterY,
    int frameWidth,
    int frameHeight
)
{
    //-----------------------------------------------
    // Validate Dimensions
    //-----------------------------------------------

    if (
        frameWidth <= 0 ||
        frameHeight <= 0
    )
    {
        eyeX = 0.0f;
        eyeY = 0.0f;

        return;
    }


    //-----------------------------------------------
    // Clamp Face Center
    //-----------------------------------------------

    faceCenterX =
        std::clamp(
            faceCenterX,
            0,
            frameWidth - 1
        );

    faceCenterY =
        std::clamp(
            faceCenterY,
            0,
            frameHeight - 1
        );


    //-----------------------------------------------
    // Normalize X
    // 0.0 -> 1.0
    //-----------------------------------------------

    const float normalizedX =
        static_cast<float>(
            faceCenterX
        )
        /
        static_cast<float>(
            frameWidth - 1
        );


    //-----------------------------------------------
    // Normalize Y
    // 0.0 -> 1.0
    //-----------------------------------------------

    const float normalizedY =
        static_cast<float>(
            faceCenterY
        )
        /
        static_cast<float>(
            frameHeight - 1
        );


    //-----------------------------------------------
    // Convert To -1.0 -> +1.0
    //-----------------------------------------------

    const float nx =
        (normalizedX * 2.0f)
        -
        1.0f;

    const float ny =
        (normalizedY * 2.0f)
        -
        1.0f;


    //-----------------------------------------------
    // Map To Eye Movement
    //-----------------------------------------------

    eyeX =
        std::clamp(
            nx * EYE_TRACK_MAX_X,
            -EYE_TRACK_MAX_X,
            EYE_TRACK_MAX_X
        );

    eyeY =
        std::clamp(
            ny * EYE_TRACK_MAX_Y,
            -EYE_TRACK_MAX_Y,
            EYE_TRACK_MAX_Y
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
    return faceX +
           (faceWidth / 2);
}


//----------------------------------------------------
// Face Center Y
//----------------------------------------------------

int FaceTracker::getFaceCenterY() const
{
    return faceY +
           (faceHeight / 2);
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