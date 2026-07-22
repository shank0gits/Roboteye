#include "FaceTracker.h"

#include "esp_log.h"

#include "dl_image_jpeg.hpp"

#include <algorithm>

#include "esp_heap_caps.h"

static const char* TAG = "FaceTracker";


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
    // Create Human Face Detector
    //-----------------------------------------------

    detector = new HumanFaceDetect(
        HumanFaceDetect::ESPDET_PICO_224_224_FACE
    );

    if (detector == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Failed to create HumanFaceDetect"
        );

        return false;
    }

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

bool FaceTracker::update(camera_fb_t* frame)
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
    if (frame == nullptr)
    {
        reset();

        return false;
    }

    //-----------------------------------------------
    // We currently expect JPEG
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
    // Create ESP-DL JPEG Image
    //-----------------------------------------------

    dl::image::jpeg_img_t jpegImage;

    jpegImage.data = frame->buf;

    jpegImage.data_len = frame->len;

//-----------------------------------------------
// Decode JPEG
//-----------------------------------------------

ESP_LOGI(
    TAG,
    "Before JPEG decode: Free=%u, Largest=%u, PSRAM Free=%u, PSRAM Largest=%u",
    heap_caps_get_free_size(MALLOC_CAP_8BIT),
    heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
    heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
    heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)
);

dl::image::img_t image =
    dl::image::sw_decode_jpeg(
        jpegImage,
        dl::image::DL_IMAGE_PIX_TYPE_RGB888
    );


//-----------------------------------------------
// Check Decode Result
//-----------------------------------------------

if (image.data == nullptr)
{
    ESP_LOGE(
        TAG,
        "JPEG decode failed: Free=%u, Largest=%u, PSRAM Free=%u, PSRAM Largest=%u",
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM)
    );

    reset();

    return false;
}

ESP_LOGI(
    TAG,
    "JPEG decoded successfully: %dx%d",
    image.width,
    image.height
);

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
    if (image.data != nullptr)
    {
    heap_caps_free(image.data);
        image.data = nullptr;
    }

    reset();

    return false;
}


//-----------------------------------------------
// Get First Face
//-----------------------------------------------

const auto& face = results.front();


//-----------------------------------------------
// Save Face Coordinates
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
        face.box[2] - face.box[0]
    );

faceHeight =
    static_cast<int>(
        face.box[3] - face.box[1]
    );

 //-----------------------------------------------
 // Free Decoded JPEG Image
 //-----------------------------------------------

if (image.data != nullptr)
{
    heap_caps_free(image.data);
    image.data = nullptr;
}



//-----------------------------------------------
// Save Image Dimensions
//-----------------------------------------------

int decodedWidth = image.width;
int decodedHeight = image.height;


    //-----------------------------------------------
    // Validate Bounding Box
    //-----------------------------------------------

    if (faceWidth <= 0 ||
        faceHeight <= 0)
    {
        reset();

        return false;
    }


    //-----------------------------------------------
    // Face Found
    //-----------------------------------------------

    faceFound = true;


    //-----------------------------------------------
    // Calculate Face Center
    //-----------------------------------------------

    int centerX =
        getFaceCenterX();

    int centerY =
        getFaceCenterY();


    //-----------------------------------------------
    // Map Face Position
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
// Convert Face Position To Eye Position
//----------------------------------------------------

void FaceTracker::mapFaceToEye(
    int faceCenterX,
    int faceCenterY,
    int frameWidth,
    int frameHeight
)
{
    if (frameWidth <= 0 ||
        frameHeight <= 0)
    {
        eyeX = 0.0f;

        eyeY = 0.0f;

        return;
    }


    //-----------------------------------------------
    // Normalize X
    //-----------------------------------------------

    float nx =
        (
            static_cast<float>(faceCenterX)
            /
            static_cast<float>(frameWidth)
        )
        *
        2.0f
        -
        1.0f;


    //-----------------------------------------------
    // Normalize Y
    //-----------------------------------------------

    float ny =
        (
            static_cast<float>(faceCenterY)
            /
            static_cast<float>(frameHeight)
        )
        *
        2.0f
        -
        1.0f;


    //-----------------------------------------------
    // Convert To EyeEngine Coordinates
    //-----------------------------------------------

    eyeX =
        std::clamp(
            nx * 10.0f,
            -10.0f,
            10.0f
        );


    eyeY =
        std::clamp(
            ny * 6.0f,
            -6.0f,
            6.0f
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