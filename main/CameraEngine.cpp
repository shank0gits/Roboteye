#include "CameraEngine.h"
#include <chrono>

#include "esp_log.h"
#include "esp_camera.h"

#include <string.h>

static const char *TAG = "CameraEngine";

//
// Camera Pin Mapping
//

#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1

#define XCLK_GPIO_NUM   15

#define SIOD_GPIO_NUM   4
#define SIOC_GPIO_NUM   5

#define Y2_GPIO_NUM     11
#define Y3_GPIO_NUM     9
#define Y4_GPIO_NUM     8
#define Y5_GPIO_NUM     10
#define Y6_GPIO_NUM     12
#define Y7_GPIO_NUM     18
#define Y8_GPIO_NUM     17
#define Y9_GPIO_NUM     16

#define VSYNC_GPIO_NUM  6
#define HREF_GPIO_NUM   7
#define PCLK_GPIO_NUM   13


//----------------------------------------------------
// Constructor
//----------------------------------------------------

CameraEngine::CameraEngine()
{
    initialized = false;
    sensor = nullptr;

    width = 0;
    height = 0;

    pixelFormat = PIXFORMAT_JPEG;

    frameCounter = 0;
}


//----------------------------------------------------
// Initialize Camera Engine
//----------------------------------------------------

bool CameraEngine::begin()
{
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "Starting Camera Engine...");
    ESP_LOGI(TAG, "================================");

    setupConfig();

    bool success = initCamera();

    if (success)
    {
        ESP_LOGI(TAG, "Camera Engine Started");
    }
    else
    {
        ESP_LOGE(TAG, "Camera Engine Failed");
    }

    return success;
}


//----------------------------------------------------
// Camera Status
//----------------------------------------------------

bool CameraEngine::isReady() const
{
    return initialized;
}


//----------------------------------------------------
// Setup Camera Configuration
//----------------------------------------------------

void CameraEngine::setupConfig()
{
    memset(&config, 0, sizeof(camera_config_t));

    //-----------------------------------------------
    // LEDC
    //-----------------------------------------------

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;


    //-----------------------------------------------
    // Camera Data Pins
    //-----------------------------------------------

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;


    //-----------------------------------------------
    // Sync Pins
    //-----------------------------------------------

    config.pin_xclk  = XCLK_GPIO_NUM;
    config.pin_pclk  = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href  = HREF_GPIO_NUM;


    //-----------------------------------------------
    // SCCB Pins
    //-----------------------------------------------

    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;


    //-----------------------------------------------
    // Control Pins
    //-----------------------------------------------

    config.pin_pwdn  = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;


    //-----------------------------------------------
    // Camera Clock
    //-----------------------------------------------

    config.xclk_freq_hz = 20000000;


    //-----------------------------------------------
    // Pixel Format
    //-----------------------------------------------

    pixelFormat = PIXFORMAT_JPEG;

    config.pixel_format = pixelFormat;


    //-----------------------------------------------
    // Frame Size
    //-----------------------------------------------

    config.frame_size = FRAMESIZE_QQVGA;

    width  = 160;
    height = 120;


    //-----------------------------------------------
    // JPEG Quality
    //-----------------------------------------------
    //
    // Lower number = higher JPEG quality
    // Higher number = more compression
    //
    // 15 is a good balance for face detection.
    //

    config.jpeg_quality = 15;


    //-----------------------------------------------
    // Frame Buffers
    //-----------------------------------------------
    //
    // Face detection decodes JPEG in software and is slower than the
    // camera's capture cadence. One buffer prevents the DMA/frame queue from
    // overrunning while the vision task owns the current frame.
    //

    config.fb_count = 1;


    //-----------------------------------------------
    // Frame Buffer Location
    //-----------------------------------------------
    //
    // ESP32-S3 has 8MB PSRAM.
    // Keep camera frame buffers in PSRAM.
    //

    config.fb_location = CAMERA_FB_IN_PSRAM;


    //-----------------------------------------------
    // Frame Grab Mode
    //-----------------------------------------------
    //
    // Wait for the current frame to be returned before capturing another one.
    // This avoids cam_hal FB-OVF and false face-lost events.
    //

    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;


    //-----------------------------------------------
    // Debug
    //-----------------------------------------------

    ESP_LOGI(TAG, "Camera Configuration:");
    ESP_LOGI(TAG, "Pixel Format  : JPEG");
    ESP_LOGI(TAG, "Frame Size    : %d x %d", width, height);
    ESP_LOGI(TAG, "Frame Buffers : %d", config.fb_count);
    ESP_LOGI(TAG, "JPEG Quality  : %d", config.jpeg_quality);
    ESP_LOGI(TAG, "Frame Buffer  : PSRAM");
    ESP_LOGI(TAG, "Grab Mode     : WHEN_EMPTY");
}


//----------------------------------------------------
// Initialize Camera
//----------------------------------------------------

bool CameraEngine::initCamera()
{
    ESP_LOGI(TAG, "Initializing Camera...");


    //-----------------------------------------------
    // Initialize ESP Camera Driver
    //-----------------------------------------------

    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Camera Initialization Failed! Error: 0x%x",
            err
        );

        initialized = false;

        return false;
    }


    ESP_LOGI(
        TAG,
        "Camera Initialized Successfully"
    );


    //-----------------------------------------------
    // Get Camera Sensor
    //-----------------------------------------------

    sensor = esp_camera_sensor_get();

    if (sensor == nullptr)
    {
        ESP_LOGE(
            TAG,
            "Unable to detect camera sensor"
        );

        initialized = false;

        return false;
    }


    //-----------------------------------------------
    // Detect Sensor
    //-----------------------------------------------

    ESP_LOGI(
        TAG,
        "Camera Sensor PID: 0x%04X",
        sensor->id.PID
    );


    switch (sensor->id.PID)
    {
        case OV2640_PID:

            ESP_LOGI(
                TAG,
                "Sensor : OV2640"
            );

            break;


        case OV3660_PID:

            ESP_LOGI(
                TAG,
                "Sensor : OV3660"
            );

            break;


        case OV5640_PID:

            ESP_LOGI(
                TAG,
                "Sensor : OV5640"
            );

            break;


        default:

            ESP_LOGW(
                TAG,
                "Unknown Sensor PID : 0x%04X",
                sensor->id.PID
            );

            break;
    }


    //-----------------------------------------------
    // Sensor Settings
    //-----------------------------------------------

    sensor->set_pixformat(
        sensor,
        PIXFORMAT_JPEG
    );

    sensor->set_framesize(
        sensor,
        FRAMESIZE_QQVGA
    );

    sensor->set_vflip(
        sensor,
        1
    );

    sensor->set_brightness(
        sensor,
        1
    );

    sensor->set_contrast(
        sensor,
        1
    );

    sensor->set_saturation(
        sensor,
        -2
    );

    sensor->set_whitebal(
        sensor,
        1
    );


    //-----------------------------------------------
    // Final Status
    //-----------------------------------------------

    initialized = true;

    ESP_LOGI(
        TAG,
        "--------------------------------"
    );

    ESP_LOGI(
        TAG,
        "Camera Ready"
    );

    ESP_LOGI(
        TAG,
        "--------------------------------"
    );

    return true;
}


//----------------------------------------------------
// Capture Camera Frame
//----------------------------------------------------

camera_fb_t* CameraEngine::captureFrame()
{
    if (!initialized)
    {
        return nullptr;
    }

    // Do not block an HTTP request indefinitely behind face processing.
    if (!frame_mutex.try_lock_for(std::chrono::milliseconds(250)))
    {
        return nullptr;
    }


    //-----------------------------------------------
    // Capture Latest Frame
    //-----------------------------------------------

    camera_fb_t* frame =
        esp_camera_fb_get();


    //-----------------------------------------------
    // Check Frame
    //-----------------------------------------------

    if (frame == nullptr)
    {
        ESP_LOGW(
            TAG,
            "Camera frame capture failed"
        );

        frame_mutex.unlock();
        return nullptr;
    }


    //-----------------------------------------------
    // Frame Counter
    //-----------------------------------------------

    frameCounter++;


    return frame;
}


//----------------------------------------------------
// Release Camera Frame
//----------------------------------------------------

void CameraEngine::releaseFrame(
    camera_fb_t* frame
)
{
    if (frame != nullptr)
    {
        esp_camera_fb_return(frame);
        frame_mutex.unlock();
    }
}


//----------------------------------------------------
// Get Frame Width
//----------------------------------------------------

int CameraEngine::getWidth() const
{
    return width;
}


//----------------------------------------------------
// Get Frame Height
//----------------------------------------------------

int CameraEngine::getHeight() const
{
    return height;
}


//----------------------------------------------------
// Get Pixel Format
//----------------------------------------------------

pixformat_t CameraEngine::getPixelFormat() const
{
    return pixelFormat;
}


//----------------------------------------------------
// Get Camera Sensor
//----------------------------------------------------

sensor_t* CameraEngine::getSensor()
{
    return sensor;
}


//----------------------------------------------------
// Update
//----------------------------------------------------

void CameraEngine::update()
{
    if (!initialized)
    {
        return;
    }


    //-----------------------------------------------
    // Capture Frame
    //-----------------------------------------------

    camera_fb_t* frame =
        captureFrame();


    if (frame == nullptr)
    {
        return;
    }


    //-----------------------------------------------
    // Release Frame
    //-----------------------------------------------

    releaseFrame(frame);
}
