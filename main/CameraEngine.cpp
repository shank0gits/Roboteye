#include "CameraEngine.h"

#include <Arduino.h>
#include "esp_camera.h"

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

    width = 0;
    height = 0;

    pixelFormat = PIXFORMAT_JPEG;
}

//----------------------------------------------------
// Initialize Camera Engine
//----------------------------------------------------

bool CameraEngine::begin()
{
    Serial.println();
    Serial.println("--------------------------------");
    Serial.println("Starting Camera Engine...");
    Serial.println("--------------------------------");

    setupConfig();

    return initCamera();
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

    //------------------------------------
    // LEDC
    //------------------------------------

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;

    //------------------------------------
    // Camera Data Pins
    //------------------------------------

    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;

    //------------------------------------
    // Sync Pins
    //------------------------------------

    config.pin_xclk  = XCLK_GPIO_NUM;
    config.pin_pclk  = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href  = HREF_GPIO_NUM;

    //------------------------------------
    // SCCB Pins
    //------------------------------------

    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;

    //------------------------------------
    // Control Pins
    //------------------------------------

    config.pin_pwdn  = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;

    //------------------------------------
    // Camera Clock
    //------------------------------------

    config.xclk_freq_hz = 20000000;

    //------------------------------------
    // Pixel Format
    //------------------------------------

    pixelFormat = PIXFORMAT_JPEG;
    config.pixel_format = pixelFormat;

    //------------------------------------
    // Memory Configuration
    //------------------------------------

    if (psramFound())
    {
        Serial.println("PSRAM Detected");

        config.frame_size = FRAMESIZE_QVGA;

        width  = 320;
        height = 240;

        config.jpeg_quality = 12;

        config.fb_count = 2;

        config.fb_location = CAMERA_FB_IN_PSRAM;

        config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
        Serial.println("PSRAM Not Found");

        config.frame_size = FRAMESIZE_QQVGA;

        width  = 160;
        height = 120;

        config.jpeg_quality = 15;

        config.fb_count = 1;

        config.fb_location = CAMERA_FB_IN_DRAM;

        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }

    //------------------------------------
    // Debug Information
    //------------------------------------

    Serial.print("Frame Size : ");
    Serial.print(width);
    Serial.print(" x ");
    Serial.println(height);

    Serial.print("Frame Buffers : ");
    Serial.println(config.fb_count);

    Serial.print("JPEG Quality : ");
    Serial.println(config.jpeg_quality);
}
//----------------------------------------------------
// Initialize Camera
//----------------------------------------------------

bool CameraEngine::initCamera()
{
    Serial.println("Initializing Camera...");

    esp_err_t err = esp_camera_init(&config);

    if (err != ESP_OK)
    {
        Serial.print("Camera Initialization Failed! Error : 0x");
        Serial.println(err, HEX);

        initialized = false;

        return false;
    }

    Serial.println("Camera Initialized Successfully");

    //------------------------------------
    // Get Camera Sensor
    //------------------------------------

    sensor = esp_camera_sensor_get();

    if(sensor == nullptr)
    {
        Serial.println("Unable to Detect Camera Sensor");

        initialized = false;

        return false;
    }

    Serial.println("Camera Sensor Found");
        //------------------------------------
    // Detect Sensor
    //------------------------------------

    switch(sensor->id.PID)
    {
        case OV2640_PID:
            Serial.println("Sensor : OV2640");
            break;

        case OV3660_PID:
            Serial.println("Sensor : OV3660");
            break;

        case OV5640_PID:
            Serial.println("Sensor : OV5640");
            break;

        default:
            Serial.print("Unknown Sensor PID : ");
            Serial.println(sensor->id.PID, HEX);
            break;
    }
        //------------------------------------
    // Sensor Settings
    //------------------------------------

    sensor->set_vflip(sensor, 1);

    sensor->set_brightness(sensor, 1);

    sensor->set_contrast(sensor, 1);

    sensor->set_saturation(sensor, -2);

    sensor->set_whitebal(sensor, 1);

    sensor->set_gain_ctrl(sensor, 1);// may be have to remove this

    sensor->set_exposure_ctrl(sensor, 1);// may be have to remove this
        //------------------------------------
    // Final Status
    //------------------------------------

    initialized = true;

    Serial.println("--------------------------------");
    Serial.println("Camera Ready");
    Serial.println("--------------------------------");

    return true;
}
//----------------------------------------------------
// Capture Camera Frame
//----------------------------------------------------

camera_fb_t* CameraEngine::captureFrame()
{
    if(!initialized)
    {
        Serial.println("Camera not initialized.");

        return nullptr;
    }

    camera_fb_t* frame = esp_camera_fb_get();

    if(frame == nullptr)
    {
        Serial.println("Failed to capture frame.");
    }

    return frame;
}
//----------------------------------------------------
// Release Camera Frame
//----------------------------------------------------

void CameraEngine::releaseFrame(camera_fb_t* frame)
{
    if(frame != nullptr)
    {
        esp_camera_fb_return(frame);
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
